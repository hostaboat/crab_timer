#ifndef _I2C_H_
#define _I2C_H_

#include <cstdint>

// XXX This only implements one master and one slave with the
// MCU as master.
namespace I2C
{
    bool init(void);
    void enable(void);
    void disable(void);
    bool enabled(void);
    void start(void);
    void stop(void);
    bool running(void);
    bool isBusy(void);
    bool begin(uint8_t slave_addr);
    bool write(uint8_t data);
    bool end(void);
};

#endif
