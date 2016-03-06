#ifndef _FAT32_H_
#define _FAT32_H_

#include <cstdint>

namespace FAT32
{
    bool init(uint16_t read_size, const char* extensions[]);
    bool prev(uint16_t num_tracks);
    bool next(uint16_t num_tracks);
    bool rewind(void);
    int read(uint8_t** p);
    uint32_t remaining(void);
    bool eof(void);
};

#endif
