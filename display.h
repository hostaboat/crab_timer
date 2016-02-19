// HT16K33 7 Segment Display **************************************************
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <cstdint>

namespace Display
{
    void init(void);
    void wake(void);
    void standby(void);
    bool isAwake(void);
    void on(void);
    void off(void);
    bool isOn(void);
    void brightness(uint8_t level);
    void count(uint16_t num);
    void time(uint8_t second, uint8_t minute);
    void blank(void);
    void dashes(void);
    void done(void);
    void lowBattery(void);
};

#endif
