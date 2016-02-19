#ifndef _SD_H_
#define _SD_H_

#include <cstdint>

#define READ_BLK_LEN  512

namespace SD
{
    bool init(void);
    bool read(uint32_t sector, uint8_t (&buf)[READ_BLK_LEN]);
};

#endif
