#ifndef _LLWU_H_
#define _LLWU_H_

#include <cstdint>

// Wake-Up Pin Edge
typedef enum
{
    WUPE_NONE = 0,
    WUPE_RISING,
    WUPE_FALLING,
    WUPE_CHANGE

} wupe_t;

// Wake-Up Source
typedef enum
{
    WUS_ERROR = -1,
    WUS_NONE = 0,
    WUS_PIN,
    WUS_TIMER

} wus_t;

namespace LLWU
{
    void enable(void);
    void disable(void);
    bool wakeupPinEnable(uint8_t pin, wupe_t wakeup_pin_edge);
    bool wakeupPinDisable(uint8_t pin);
    bool wakeupLPTMREnable(uint16_t msecs);
    bool wakeupLPTMRDisable(void);
    wus_t sleep(void);
    wus_t wakeupSource(void);
    int8_t wakeupPin(void);
};

#endif
