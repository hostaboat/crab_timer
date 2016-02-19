#ifndef _FAT32_H_
#define _FAT32_H_

#include <cstdint>

namespace FAT32
{
    bool init(uint16_t read_size, const char* extensions[]);
    bool newFile(int8_t increment);
    int read(uint8_t** p);
    bool eof(void);
};

#endif
