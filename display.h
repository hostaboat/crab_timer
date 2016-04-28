// HT16K33 7 Segment Display **************************************************
#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <cstdint>

#define DISPLAY_MIN_BRIGHTNESS  0x00
#define DISPLAY_MID_BRIGHTNESS  0x07
#define DISPLAY_MAX_BRIGHTNESS  0x0F

namespace Display
{
    bool init(uint8_t brightness);
    void wake(void);
    void standby(void);
    bool isAwake(void);
    void on(void);
    void off(void);
    bool isOn(void);
    void up(void);
    void down(void);
    void brightness(uint8_t level);
    void count(uint16_t num);
    void time(uint8_t second, uint8_t minute);
    void blank(void);
    void dashes(void);
    void done(void);
    void lowBattery(void);
};

#endif
