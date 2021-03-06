#ifndef _USER_INTERFACE_H_
#define _USER_INTERFACE_H_

#include <IntervalTimer.h>
#include <cstdint>

typedef enum _ui_state
{
    UI_STATE__COUNT,
    UI_STATE__SET,
    UI_STATE__TIMER,
    UI_STATE__PASS

} ui_state_t;

typedef enum _switch_state
{
    SWITCH_STATE__NONE = 0,
    SWITCH_STATE__DEPRESSED,
    SWITCH_STATE__SHORT_PRESS,
    SWITCH_STATE__LONG_PRESS

} switch_state_t;

typedef enum _timer_state
{
    TIMER_STATE__INIT,
    TIMER_STATE__TIMER,
    TIMER_STATE__ALERT,
    TIMER_STATE__DONE

} timer_state_t;

class UserInterface
{
    public:
        UserInterface(void);
        void init(void);
        void update(void);

    private:
        // Functions
        void stateInit(void);
        void encoderInit(void);
        ui_state_t getState(void);
        ui_state_t getInput(void);
        bool sleep(void);
        void count(void);
        void set(void);
        void time(void);
        void timerSet(void);
        void timerStart(void);
        void timerStop(void);
        void timerAlert(void);

        // Actions
        int8_t _encoder_turn;
        switch_state_t _switch_state;

        // Timings
        unsigned long _sleep;
        unsigned long _sleep_time;

        // States
        ui_state_t _ui_state;
        ui_state_t _last_ui_state;

        // Count state
        uint16_t _total_count;
        uint16_t _count;

        // Set time state
        uint16_t _default_minutes;
        uint16_t _minutes;

        // Timer state
        timer_state_t _timer_state;
        uint8_t _timer_pause;

        // Display and LEDs brightness
        uint8_t _brightness;

        IntervalTimer _display_timer;
        IntervalTimer _led_timer;
};

#endif
