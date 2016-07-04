#ifndef _FLEX_RAM_H_
#define _FLEX_RAM_H_

#include <cstdint>

#define NVM_COUNT_INDEX   0
#define NVM_TIME_INDEX    1
#define NVM_FILE_INDEX    2

namespace NVM
{
    bool init(void);
    bool read8(uint16_t index, uint8_t& value);
    bool read16(uint16_t index, uint16_t& value);
    bool read32(uint16_t index, uint32_t& value);
    bool write(uint16_t index, uint32_t value);
};

#endif
