#include "fat32.h"
#include "sd.h"
#include "nvm.h"
#include <cstdint>
#include <cstring>
//#include <WProgram.h>   // For Serial debugging

#define FAT32_SECTOR_SIZE   READ_BLK_LEN
#define DIR_MAX_LEVEL  7

#define BOOT_RECORD_SIGNATURE    0xAA55

#define MBR_EXE_CODE_LEN    446
#define NUM_PARTITIONS        4

#define SHORT_NAME_LEN   11
#define LONG_NAME_LEN_1  10
#define LONG_NAME_LEN_2  12
#define LONG_NAME_LEN_3   4

#define ATTR_READ_ONLY  0x01
#define ATTR_HIDDEN     0x02
#define ATTR_SYSTEM     0x04
#define ATTR_VOLUME_ID  0x08
#define ATTR_DIRECTORY  0x10
#define ATTR_ARCHIVE    0x20
#define ATTR_LONG_NAME  (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)

#define LAST_LONG_ENTRY  0x40
#define DIR_ENTRY_FREE   0xE5
#define DIR_ENTRY_LAST   0x00

#define PTYPE_FAT12            0x01
#define PTYPE_FAT16_LT_32MB    0x04
#define PTYPE_EXT_DOS          0x05
#define PTYPE_FAT16_GT_32MB    0x06
#define PTYPE_FAT32            0x0B
#define PTYPE_FAT32_LBA13h     0x0C
#define PTYPE_FAT16_LBA13h     0x0E
#define PTYPE_EXT_DOS_LBA13h   0x0F

#define CLUSTER_MAX_FAT12    4085
#define CLUSTER_MAX_FAT16   65525

#define FAT32_CLUSTER_EOC   0x0FFFFFF8   // End of Chain
#define FAT32_CLUSTER_BAD   0x0FFFFFF7
#define FAT32_CLUSTER_MASK  0x0FFFFFFF

// 16 bytes
typedef struct
{
    uint8_t current_state;
    uint8_t partition_head_start;
    uint16_t partition_cylinder_start;
    uint8_t partition_type;
    uint8_t partition_head_end;
    uint16_t partition_cylinder_end;
    uint32_t sector_offset;
    uint32_t num_sectors;

} partition_t;

typedef struct
{
    uint8_t jump_code[3];
    uint8_t oem_name[8];
    uint16_t bytes_per_sector;          // Valid values though assuming 512
                                        //   512, 1024, 2048, 4096
    uint8_t sectors_per_cluster;        // Must be power of 2
                                        //   1, 2, 4, 8, 16, 32, 64, 128
    uint16_t reserved_sectors;          // MUST not be 0
    uint8_t num_FATs;                   // Should always be 2
    uint16_t num_root_dir_entries;      // Only valid for FAT12 and FAT16.
                                        //   FAT32 MUST be 0
    uint16_t num_sectors16;             // Valid for FAT12 and FAT16.
                                        //   For FAT32 MUST be 0
    uint8_t media;
    uint16_t num_FAT_sectors16;         // For FAT32 must be 0
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t num_hidden_sectors;
    uint32_t num_sectors32;             // Total count of sectors on the volume
                                        //   FAT32 only

    union
    {
        // FAT12 and FAT16
        struct
        {
            uint8_t drive_num;
            uint8_t reserved1;
            uint8_t boot_sig;
            uint32_t volume_id;
            uint8_t volume_label[11];
            uint8_t file_sys_type[8];
        };

        // FAT32
        struct
        {
            uint32_t num_FAT_sectors32;      // Number of sectors per FAT
            uint16_t ext_flags;
            uint16_t fs_version;
            uint32_t root_cluster;           // Set to the cluster number
                                             //   of the first cluster of the
                                             //   root directory
            uint16_t fs_info_sector;         // Sector number of the FSINFO
                                             //   structure in the reserved area
            uint16_t backup_boot_sector;     // Sector number of backup copy of
                                             //   boot record in reserved area
            uint8_t reserved[12];            // Not used and not to be confused
                                             //   with the reserved area
            uint8_t drive_num_32;
            uint8_t reserved2;
            uint8_t boot_sig_32;             // Extended boot signature 0x29 indicates
                                             //   presence of next 3 fields
            uint32_t volume_id_32;
            uint8_t volume_label_32[11];
            uint8_t file_sys_type_32[8];
        };
    };

} __attribute__((packed)) boot_sector_t;

// 512 bytes
typedef struct
{
    uint32_t lead_sig;            // 0x41615252 - validates that this
                                  //   is an FSInfo sector
    uint8_t reserved1[480];
    uint32_t struct_sig;          // 0x61417272 - another signature
    uint32_t free_count;          // Last known free cluster on the volume.
                                  //   0xFFFFFFFF == Unknown.  Not reliable.
    uint32_t next_free;           // Cluster number driver should start
                                  //   looking for free clusters - A hint.
    uint8_t reserved2[12];
    uint32_t trail_sig;           // 0xAA55 - like the boot record signature

} fs_info_t;

typedef struct
{
    union
    {
        // 32 bytes
        struct
        {
            char name[SHORT_NAME_LEN];     // Short 8.3 name
            uint8_t short_attrs;           // File attributes
            uint8_t reserved;              // Reserved for Windows NT
            uint8_t ctime_tenth;           // File creation time stamp
                                           //   a tenth of a second
            uint16_t ctime;                // File creation time
            uint16_t cdate;                // File creation date
            uint16_t adate;                // Last access date
            uint16_t first_cluster_high;   // High word of this entry's first
                                           //   cluster number (0 for FAT12 and FAT16)
            uint16_t wtime;                // Last write time
            uint16_t wdate;                // Last write date
            uint16_t first_cluster_low;    // Low word of this entry's first cluster number
            uint32_t file_size;            // This file's size in bytes

        };

        // 32 bytes
        struct
        {
            uint8_t order_num;                 // The order of this entry in the
                                               //   sequence of long dir entries
                                               //   associated with short dir entry.
                                               // If masked with 0x40 (LAST_LONG_ENTRY),
                                               //   it's the last in the set.
            char long_name1[LONG_NAME_LEN_1];  // Characters 1-5 of the long name (UTF-16LE)
            uint8_t long_attrs;                // Must be ATTR_LONG_NAME
            uint8_t type;                      // If zero, directory entry is a
                                               //   sub-component of a long name
            uint8_t cksum;                     // Checksum of name in the short
                                               //   dir entry at end of long dir set
            char long_name2[LONG_NAME_LEN_2];  // Characters 6-11 of the long name
            uint16_t first_cluster;            // MUST be ZERO.  Has no meaning in this context
            char long_name3[LONG_NAME_LEN_3];  // Characters 12-13 of the long name

        };

        // For access to common attributes field
        struct
        {
            uint8_t dummy[SHORT_NAME_LEN];
            uint8_t attrs;
        };
    };

} dir_t;

typedef union
{
    uint8_t   u8[FAT32_SECTOR_SIZE];
    uint16_t u16[FAT32_SECTOR_SIZE / 2];
    uint32_t u32[FAT32_SECTOR_SIZE / 4];
    dir_t    dir[FAT32_SECTOR_SIZE / 32];

} sector_t;

typedef struct
{
    uint32_t cluster;
    uint32_t size;

} file_t;

namespace FAT32
{
    bool isFAT32(const boot_sector_t* bs);
    uint32_t sectorOfCluster(uint32_t cluster);
    uint32_t cluster(uint16_t high, uint16_t low);
    bool nextCluster(uint32_t cluster);
    bool readFAT(uint32_t fat_sector);
    bool getFiles(uint32_t cluster, uint8_t level, const char* extensions[]);
    bool newFile(uint16_t index);

    uint32_t fat_sector_start = 0;
    uint32_t data_sector_start = 0;
    uint8_t sectors_per_cluster = 0;
    uint8_t cluster_to_sector = 0;
    uint32_t root_cluster = 0;

    uint32_t fat_sector = 0;
    uint8_t fat_buffer[FAT32_SECTOR_SIZE];

    uint16_t read_size = 0;

    uint32_t file_cluster = 0;
    uint32_t file_sector = 0;
    uint32_t file_cluster_offset = 0;
    uint32_t file_sector_offset = 0;
    uint32_t file_remaining = 0;
    uint8_t file_buffer[FAT32_SECTOR_SIZE];

    uint8_t data_dir_sectors[DIR_MAX_LEVEL][FAT32_SECTOR_SIZE];
    uint8_t fat_dir_sectors[DIR_MAX_LEVEL][FAT32_SECTOR_SIZE];

    uint16_t file_index = 0;
    uint16_t num_files = 0;
    file_t files[1024];
};

bool FAT32::init(uint16_t read_size, const char* extensions[])
{
    if ((read_size > FAT32_SECTOR_SIZE)
            || ((FAT32_SECTOR_SIZE % read_size) != 0))
    {
        return false;
    }

    FAT32::read_size = read_size;

    if (!SD::init())
        return false;

    static uint8_t buf[FAT32_SECTOR_SIZE];

    if (!SD::read(0, buf))
        return false;

    sector_t* mbr = (sector_t*)buf;
    if (mbr->u16[255] != BOOT_RECORD_SIGNATURE)
        return false;

    uint32_t partition_sector_start = 0xFFFFFFFF;
    uint16_t offset = MBR_EXE_CODE_LEN;

    for (uint8_t i = 0; i < NUM_PARTITIONS; i++)
    {
        partition_t* p = (partition_t*)(mbr->u8 + offset);

        // XXX Only support FAT32 for now
        switch (p->partition_type)
        {
            case PTYPE_FAT32:
            case PTYPE_FAT32_LBA13h:
                partition_sector_start = p->sector_offset;
                break;

            case PTYPE_FAT16_LT_32MB:
            case PTYPE_FAT16_GT_32MB:
            case PTYPE_FAT16_LBA13h:
            case PTYPE_EXT_DOS:
            case PTYPE_EXT_DOS_LBA13h:
            case PTYPE_FAT12:
            default:
                break;
        }

        if (partition_sector_start != 0xFFFFFFFF)
            break;

        offset += sizeof(*p);
    }

    if (partition_sector_start == 0xFFFFFFFF)
        return false;

    if (!SD::read(partition_sector_start, buf))
        return false;

    sector_t* ps = (sector_t*)buf;
    
    // XXX Assuming 512 byte sectors
    if (ps->u16[255] != BOOT_RECORD_SIGNATURE)
        return false;

    boot_sector_t* bs = (boot_sector_t*)ps;

    // XXX Only supporting FAT32
    if (!FAT32::isFAT32(bs))
        return false;

    // XXX Supported values are supposedly 512, 1024, 2048 and 4096
    // however 512 is supposedly almost always used
    if (bs->bytes_per_sector != FAT32_SECTOR_SIZE)
        return false;

#if 0
    // XXX Supposedly could have more or less but should be 2
    // Only need one FAT to do lookups.  Others are copies.
    if (bs->num_FATs != 2)
        return false;
#endif

    // Field MUST NOT be 0
    if (bs->reserved_sectors == 0)
        return false;

    FAT32::fat_sector_start = partition_sector_start + bs->reserved_sectors;
    FAT32::data_sector_start = FAT32::fat_sector_start + (bs->num_FAT_sectors32 * bs->num_FATs);
    FAT32::sectors_per_cluster = bs->sectors_per_cluster;
    FAT32::cluster_to_sector = __builtin_ctz(FAT32::sectors_per_cluster);
    FAT32::root_cluster = bs->root_cluster;

    if (!FAT32::getFiles(FAT32::root_cluster, 0, extensions))
        return false;

    // getFiles() will end up setting file_index to the number of files found.
    FAT32::num_files = FAT32::file_index;

    (void)NVM::init();

    // Get index of file to start playing from EEPROM
    if (!NVM::read16(NVM_FILE_INDEX, FAT32::file_index)
            || (FAT32::file_index >= FAT32::num_files))
    {
        FAT32::file_index = 0;
    }

    if (!FAT32::newFile(FAT32::file_index))
        return false;

    return true;
}

bool FAT32::isFAT32(const boot_sector_t* bs)
{
    if ((bs->num_root_dir_entries != 0)
            || (bs->num_sectors16 != 0)
            || (bs->num_FAT_sectors16 != 0)
            || (bs->num_FAT_sectors32 == 0))
    {
        return false;
    }

    uint32_t cluster_count =
        (bs->num_sectors32 -
         (bs->reserved_sectors + bs->num_FATs * bs->num_FAT_sectors32)) / bs->sectors_per_cluster;

    if (cluster_count < CLUSTER_MAX_FAT12)
    {
        // FAT12
        return false;
    }
    else if (cluster_count < CLUSTER_MAX_FAT16)
    {
        // FAT16
        return false;
    }
    else
    {
        // FAT32
        return true;
    }
}

uint32_t FAT32::sectorOfCluster(uint32_t cluster)
{
    return (((cluster - 2) << FAT32::cluster_to_sector) + FAT32::data_sector_start);
}

uint32_t FAT32::cluster(uint16_t high, uint16_t low)
{
    return (((uint32_t)high << 16) & 0xFFFF0000) | ((uint32_t)low & 0x0000FFFF);
}

// Each entry in FAT table is 32 bits or 4 bytes so byte offset would be
// cluster number * 4 or left shift of 2.
// Each FAT sector is assumed to be 512 bytes so the sector for the above
// calculated byte offset would be the byte offset / 512 or a right shift of 9.
// So sector == ((cluster << 2) >> 9) == (cluster >> 7)
bool FAT32::nextCluster(uint32_t cluster)
{
    uint32_t fat_sector = (cluster >> 7) + FAT32::fat_sector_start;
    if (!FAT32::readFAT(fat_sector))
        return false;

    sector_t* s = (sector_t*)FAT32::fat_buffer;
    uint32_t next_cluster =
        s->u32[cluster & ((FAT32_SECTOR_SIZE / 4) - 1)] & FAT32_CLUSTER_MASK;

    // XXX Need to totally abort playing and get a new card
    if (next_cluster == FAT32_CLUSTER_BAD)
        return false;

    if (next_cluster >= FAT32_CLUSTER_EOC)
    {
        // There are no more clusters and the file is done.
        // Only time this case will likely be relevant is if
        // EOF occurs exactly at the end of a cluster.
        FAT32::file_cluster = FAT32_CLUSTER_EOC;
        FAT32::file_sector = 0;
    }
    else
    {
        FAT32::file_cluster = next_cluster;
        FAT32::file_sector = FAT32::sectorOfCluster(next_cluster);
    }

    FAT32::file_cluster_offset = 0;
    FAT32::file_sector_offset = 0;

    return true;
}

bool FAT32::readFAT(uint32_t sector)
{
    if (sector == FAT32::fat_sector)
        return true;

    if (!SD::read(sector, FAT32::fat_buffer))
        return false;

    FAT32::fat_sector = sector;

    return true;
}

// Start with root cluster at level 0
bool FAT32::getFiles(uint32_t cluster, uint8_t level, const char* extensions[])
{
    if (level >= DIR_MAX_LEVEL)
        return true;

    if (FAT32::file_index == (sizeof(FAT32::files) / sizeof(file_t)))
        return true;

    uint32_t data_sector = FAT32::sectorOfCluster(cluster);
    uint32_t fat_sector = 0;

    if (!SD::read(data_sector, FAT32::data_dir_sectors[level]))
        return false;

    dir_t* entry = (dir_t*)FAT32::data_dir_sectors[level];
    uint16_t sector_offset = 0;
    uint8_t cluster_offset = 0;

    while (entry->name[0] != DIR_ENTRY_LAST)
    {
        uint8_t attrs = entry->attrs;

        // Read attrs field
        if (!((attrs & ATTR_LONG_NAME) == ATTR_LONG_NAME)
                && (entry->name[0] != 0x2E)  // Skip dot files
                && (entry->name[0] != DIR_ENTRY_FREE)
                && (entry->name[0] != DIR_ENTRY_LAST))
        {
            if ((attrs & ATTR_DIRECTORY) && !(attrs & (ATTR_SYSTEM | ATTR_HIDDEN)))
            {
                uint32_t next_dir_cluster =
                    FAT32::cluster(entry->first_cluster_high, entry->first_cluster_low);

                if (!getFiles(next_dir_cluster, level + 1, extensions))
                    return false;
            }
            else if (!(entry->attrs & (ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)))
            {
                char* ext = entry->name + 8;

                for (int i = 0; extensions[i] != NULL; i++)
                {
                    if (strncmp(ext, extensions[i], 3) == 0)
                    {
                        FAT32::files[FAT32::file_index].cluster =
                            FAT32::cluster(entry->first_cluster_high, entry->first_cluster_low);

                        FAT32::files[FAT32::file_index].size = entry->file_size;

                        FAT32::file_index++;

                        if (FAT32::file_index == (sizeof(FAT32::files) / sizeof(file_t)))
                            return true;
                    }
                }
            }
        }

        sector_offset += sizeof(dir_t);

        if (sector_offset == FAT32_SECTOR_SIZE)
        {
            cluster_offset++;
            sector_offset = 0;

            if (cluster_offset == FAT32::sectors_per_cluster)
            {
                uint32_t new_fat_sector = (cluster >> 7) + FAT32::fat_sector_start;

                if (new_fat_sector != fat_sector)
                {
                    if (!SD::read(new_fat_sector, FAT32::fat_dir_sectors[level]))
                        return false;

                    fat_sector = new_fat_sector;
                }

                sector_t* s = (sector_t*)FAT32::fat_dir_sectors[level];
                uint32_t next_cluster =
                    s->u32[cluster & ((FAT32_SECTOR_SIZE / 4) - 1)] & FAT32_CLUSTER_MASK;

                if (next_cluster == FAT32_CLUSTER_BAD)
                    return false;

                if (next_cluster >= FAT32_CLUSTER_EOC)
                {
                    // There are no more clusters and the file is done.
                    // Only time this case will likely be relevant is if
                    // EOF occurs exactly at the end of a cluster.
                    return true;
                }
                else
                {
                    cluster = next_cluster;
                    data_sector = FAT32::sectorOfCluster(next_cluster);
                }

                cluster_offset = 0;
            }
            else
            {
                data_sector++;
            }

            if (!SD::read(data_sector, FAT32::data_dir_sectors[level]))
                return false;

            entry = (dir_t*)FAT32::data_dir_sectors[level];
        }
        else
        {
            entry++;
        }
    }

    return true;
}

bool FAT32::prev(uint16_t num_tracks)
{
    if (num_tracks == 0)
        return FAT32::rewind();

    uint32_t file_size = FAT32::files[FAT32::file_index].size;

    // If at least 1/4 of the song played save index to EEPROM
    // and just rewind it, i.e. decrement number of tracks.
    if ((file_size - FAT32::file_remaining) > (file_size >> 2))
    {
        (void)NVM::write(NVM_FILE_INDEX, FAT32::file_index);
        num_tracks--;
    }

    if (num_tracks != 0)
    {
        int32_t inc = ((int32_t)FAT32::file_index - num_tracks) % FAT32::num_files;
        if (inc < 0)
            FAT32::file_index = (uint16_t)(FAT32::num_files + inc);
        else
            FAT32::file_index = (uint16_t)inc;
    }

    return FAT32::newFile(FAT32::file_index);
}

bool FAT32::next(uint16_t num_tracks)
{
    uint32_t file_size = FAT32::files[FAT32::file_index].size;

    // If at least 1/4 of the song played save index to EEPROM
    if ((file_size - FAT32::file_remaining) > (file_size >> 2))
        (void)NVM::write(NVM_FILE_INDEX, FAT32::file_index);

    if (num_tracks != 0)
    {
        int32_t inc = ((int32_t)FAT32::file_index + num_tracks) % FAT32::num_files;
        FAT32::file_index = (uint16_t)inc;
    }

    return FAT32::newFile(FAT32::file_index);
}

bool FAT32::rewind(void)
{
    return FAT32::newFile(FAT32::file_index);
}

bool FAT32::newFile(uint16_t index)
{
    if ((FAT32::num_files == 0) || (index >= FAT32::num_files))
        return false;

    FAT32::file_cluster = FAT32::files[index].cluster;
    FAT32::file_remaining = FAT32::files[index].size;
    FAT32::file_sector_offset = 0;
    FAT32::file_cluster_offset = 0;

    FAT32::file_sector = FAT32::sectorOfCluster(FAT32::file_cluster);

    if (!SD::read(FAT32::file_sector, FAT32::file_buffer))
        return false;

    return true;
}

int FAT32::read(uint8_t** p)
{
    if (FAT32::eof())
        return 0;

    if (FAT32::file_sector_offset == FAT32_SECTOR_SIZE)
    {
        FAT32::file_cluster_offset++;

        if (FAT32::file_cluster_offset == FAT32::sectors_per_cluster)
        {
            if (!FAT32::nextCluster(FAT32::file_cluster))
                return -1;

            if (FAT32::file_cluster == FAT32_CLUSTER_EOC)
            {
                FAT32::file_remaining = 0;
                return 0;
            }
        }
        else
        {
            FAT32::file_sector++;
            FAT32::file_sector_offset = 0;
        }

        if (!SD::read(FAT32::file_sector, FAT32::file_buffer))
            return -1;
    }

    *p = FAT32::file_buffer + FAT32::file_sector_offset;

    uint8_t bytes = FAT32::file_remaining < FAT32::read_size ? FAT32::file_remaining : FAT32::read_size;

    FAT32::file_remaining -= bytes;
    FAT32::file_sector_offset += bytes;
    
    return bytes;
}

uint32_t FAT32::remaining(void)
{
    return FAT32::file_remaining;
}

bool FAT32::eof(void)
{
    return (FAT32::file_remaining == 0);
}

