#include <IntervalTimer.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <elapsedMillis.h>
#include <cstdint>
#include "pins.h"
#include "interface.h"
#include "display.h"
#include "player.h"

#define DISPLAY_INACTIVE  15000  // 15 seconds before setting it to standby
#define RESET_TIME         2000  // 2 seconds of continuous pushing of switch to reset

#define LED_BRIGHTNESS   64
#define NUM_LEDS          8
CRGB s_leds[NUM_LEDS];

#define TIMER_HUE_MAX   160  // Start with blue hue and decrease to red hue
#define TIMER_HUE_MIN     0  // Red hue

#define EEPROM_COUNT_HIGH  0
#define EEPROM_COUNT_LOW   1
#define EEPROM_MINUTES     3

// *****************************************************************************
// Global variables used in interrupt handlers
static volatile int8_t s_encoder_turn = 0;
static volatile switch_state_t s_switch_state = SWITCH_STATE__NONE;
static volatile uint32_t s_switch_depressed;

static volatile uint16_t s_seconds = 0;
static volatile uint16_t s_hue = 160;

// *****************************************************************************


// *****************************************************************************
// Interrupt handlers
static void encoder_rotate(void)
{
    static int last = 0;

    if (GPIO::read(PIN_ROT_ENC_SW) == HIGH)
        return;

    if (GPIO::read(PIN_ROT_ENC_A) != GPIO::read(PIN_ROT_ENC_B))
    {
        // This is so a turn in the opposite direction isn't too sudden
        s_encoder_turn = (last == -1) ? 0 : 1;
        last = 1;
    }
    else
    {
        s_encoder_turn = (last == 1) ? 0 : -1;
        last = -1;
    }
}

static void switch_pressed(void)
{
    static int depressed = 0;

    depressed ^= 1;

    if (depressed)
    {
        s_switch_depressed = millis();
        s_switch_state = SWITCH_STATE__DEPRESSED;
    }
    else  // Released
    {
        if ((millis() - s_switch_depressed) < RESET_TIME)
            s_switch_state = SWITCH_STATE__SHORT_PRESS;
        else
            s_switch_state = SWITCH_STATE__LONG_PRESS;
    }
}

static void update_display(void)
{
    if (s_seconds == 0)
        return;

    s_seconds--;

    uint16_t minute = 0;
    uint16_t second = s_seconds;

    while (second >= 60)
    {
        minute++;
        second -= 60;
    }

    Display::time(second, minute);
}

static void update_leds(void)
{
    if (s_hue == TIMER_HUE_MIN)
        return;

    s_hue--;

    for (uint8_t i = 0; i < NUM_LEDS; i++)
        s_leds[i] = CHSV(s_hue, 255, 255);
    FastLED.show();
    //FastLED.showColor(CHSV(s_hue, 255, 255));
}

// *****************************************************************************


// *****************************************************************************
// Member functions
UserInterface::UserInterface(void)
    : _encoder_turn(0), _switch_state(SWITCH_STATE__NONE),
      _display_sleep(0), _ui_state(UI_STATE__PASS), _last_ui_state(UI_STATE__PASS),
      _total_count(0), _count(0), _default_minutes(0), _minutes(0),
      _timer_state(TIMER_STATE__INIT), _timer_pause(1), _low_battery_ack(false)
{
}

void UserInterface::init(void)
{
    FastLED.addLeds<NEOPIXEL, PIN_NEOPIXEL>(s_leds, NUM_LEDS);
    FastLED.setBrightness(LED_BRIGHTNESS);

    Display::init();
    Player::init();

    attachInterrupt(PIN_ROT_ENC_A, encoder_rotate, CHANGE);
    attachInterrupt(PIN_ROT_ENC_SW, switch_pressed, CHANGE);

    _total_count = ((uint16_t)EEPROM.read(EEPROM_COUNT_HIGH) << 8)
        | (uint16_t)EEPROM.read(EEPROM_COUNT_LOW);
    _default_minutes = _minutes = EEPROM.read(EEPROM_MINUTES);
}

void UserInterface::stateInit(void)
{
    FastLED.clear(true);
    timerStop();
    Display::blank();

    if (!Display::isAwake())
        Display::wake();

    switch (_ui_state)
    {
        case UI_STATE__COUNT:
            _count = 0;
            Display::count(_total_count);
            break;

        case UI_STATE__SET:
            _minutes = _default_minutes;
            Display::time(0, _default_minutes);
            break;

        case UI_STATE__TIMER:
            _timer_state = TIMER_STATE__INIT;
            _timer_pause = 1;
            Display::time(0, _minutes);
            break;

        case UI_STATE__LOW_BAT:
            timerStop();
            Player::disable();
            break;

        case UI_STATE__PASS:
            break;
    }
}

ui_state_t UserInterface::getState(void)
{
    if (_ui_state == UI_STATE__LOW_BAT)
        return _ui_state;

    if (GPIO::read(PIN_LOW_BATT) == LOW)
        _ui_state = UI_STATE__LOW_BAT;
    else if (GPIO::read(PIN_ROT_SWI_A) == LOW)
        _ui_state = UI_STATE__COUNT;
    else if (GPIO::read(PIN_ROT_SWI_B) == LOW)
        _ui_state = UI_STATE__SET;
    else if (GPIO::read(PIN_ROT_SWI_C) == LOW)
        _ui_state = UI_STATE__TIMER;
    else
        _ui_state = UI_STATE__PASS;

    return _ui_state;
}

bool UserInterface::displaySleep(void)
{
    switch (_ui_state)
    {
        case UI_STATE__COUNT:
        case UI_STATE__SET:
            break;

        case UI_STATE__TIMER:
            if (!_timer_pause)
                return false;
            break;

        case UI_STATE__LOW_BAT:
            if (!_low_battery_ack)
                return false;
            break;

        case UI_STATE__PASS:
            break;
    }

    if ((_encoder_turn != 0) || (_switch_state != SWITCH_STATE__NONE))
    {
        _display_sleep = millis();

        // Treat encoder turn or switch press as a wakeup
        // and don't process in usual way
        if (!Display::isAwake())
        {
            FastLED.show(LED_BRIGHTNESS);
            Display::wake();
            return true;
        }
    }
    else if (!Display::isAwake())
    {
        return true;
    }
    else if ((millis() - _display_sleep) > DISPLAY_INACTIVE)
    {
        FastLED.show(0);
        Display::standby();
        return true;
    }

    return false;
}

ui_state_t UserInterface::getInput(void)
{
    uint32_t switch_depressed = 0;

    noInterrupts();

    if (getState() != _last_ui_state)
    {
        stateInit();
        _last_ui_state = _ui_state;
        _display_sleep = millis();
    }
    else if (s_switch_state != SWITCH_STATE__NONE)
    {
        _switch_state = s_switch_state;
        if (s_switch_state != SWITCH_STATE__DEPRESSED)
            s_switch_state = SWITCH_STATE__NONE;
        switch_depressed = millis() - s_switch_depressed;
        s_encoder_turn = 0;
    }
    else if (s_encoder_turn != 0)
    {
        _encoder_turn = s_encoder_turn;
        s_encoder_turn = 0;
        s_switch_state = SWITCH_STATE__NONE;
    }

    interrupts();

    if (_switch_state == SWITCH_STATE__DEPRESSED)
    {
        if (switch_depressed >= RESET_TIME)
        {
            static int display_blink = 0;
            static uint32_t blink_last = 0;

            timerStop();

            if ((switch_depressed - blink_last) > 300)
            {
                blink_last = switch_depressed;

                display_blink ^= 1;

                if (display_blink)
                    Display::dashes();
                else
                    Display::blank();
            }
        }

        return UI_STATE__PASS;
    }

    // Put display and leds to sleep when inactive
    if (displaySleep())
        return UI_STATE__PASS;

    return _ui_state;
}

void UserInterface::update(void)
{
    switch (getInput())
    {
        case UI_STATE__COUNT:
            count();
            break;

        case UI_STATE__SET:
            set();
            break;

        case UI_STATE__TIMER:
            time();
            break;

        case UI_STATE__LOW_BAT:
            lowBattery();
            break;

        case UI_STATE__PASS:
            break;
    }

    Player::play();

    _encoder_turn = 0;
    _switch_state = SWITCH_STATE__NONE;
}

void UserInterface::count(void)
{
    if (_encoder_turn != 0)
    {
        if (_encoder_turn < 0)
            _count = (_count == 0) ? 999 : (_count - 1);
        else
            _count = (_count == 999) ? 0 : (_count + 1);

        Display::count(_count);
    }
    else
    {
        switch (_switch_state)
        {
            case SWITCH_STATE__NONE:
            case SWITCH_STATE__DEPRESSED:
                break;

            case SWITCH_STATE__SHORT_PRESS:
                _total_count += _count;
                _count = 0;
                EEPROM.write(EEPROM_COUNT_HIGH, (uint8_t)(_total_count >> 8));
                EEPROM.write(EEPROM_COUNT_LOW, (uint8_t)(_total_count & 0x00FF));
                Display::count(_total_count);
                break;

            case SWITCH_STATE__LONG_PRESS:
                _count = _total_count = 0;
                EEPROM.write(EEPROM_COUNT_HIGH, 0);
                EEPROM.write(EEPROM_COUNT_LOW, 0);
                Display::count(_total_count);
                break;
        }
    }
}

void UserInterface::set(void)
{
    if (_encoder_turn != 0)
    {
        if (_encoder_turn < 0)
            _minutes = (_minutes == 0) ? 99 : (_minutes - 1);
        else
            _minutes = (_minutes == 99) ? 0 : (_minutes + 1);

        Display::time(0, _minutes);
    }
    else
    {
        switch (_switch_state)
        {
            case SWITCH_STATE__NONE:
            case SWITCH_STATE__DEPRESSED:
                break;

            case SWITCH_STATE__SHORT_PRESS:
                _default_minutes = _minutes;
                EEPROM.write(EEPROM_MINUTES, _default_minutes);
                break;

            case SWITCH_STATE__LONG_PRESS:
                _default_minutes = EEPROM.read(EEPROM_MINUTES);
                _minutes = _default_minutes;
                Display::time(0, _default_minutes);
                break;
        }
    }
}

void UserInterface::time(void)
{
    bool switch_pressed = false;

    switch (_switch_state)
    {
        case SWITCH_STATE__NONE:
            break;

        case SWITCH_STATE__DEPRESSED:
            return;

        case SWITCH_STATE__SHORT_PRESS:
            _timer_pause ^= 1;
            switch_pressed = true;
            _display_sleep = millis();
            break;

        case SWITCH_STATE__LONG_PRESS:
            FastLED.clear(true);
            timerStop();
            _timer_state = TIMER_STATE__INIT;
            _timer_pause = 1;
            Display::time(0, _minutes);
            _display_sleep = millis();
            break;
    }

    switch (_timer_state)
    {
        case TIMER_STATE__INIT:
            if ((_minutes == 0) || _timer_pause)
                return;

            timerSet();
            timerStart();

            _timer_state = TIMER_STATE__TIMER;
            break;

        case TIMER_STATE__TIMER:
            if (s_seconds == 0)
            {
                timerStop();
                _timer_state = TIMER_STATE__ALERT;
            }
            else if (switch_pressed)
            {
                if (_timer_pause)
                    timerStop();
                else
                    timerStart();
            }
            break;

        case TIMER_STATE__ALERT:
            if (switch_pressed)
                _timer_state = TIMER_STATE__DONE;
            else
                timerAlert();
            break;

        case TIMER_STATE__DONE:
            if (switch_pressed)
            {
                _timer_pause = 1;
                Display::time(0, _minutes);
                _timer_state = TIMER_STATE__INIT;
            }  
            break;
    }
}

void UserInterface::timerSet(void)
{
    s_seconds = _minutes * 60;
    s_hue = TIMER_HUE_MAX;
    for (uint8_t i = 0; i < NUM_LEDS; i++)
        s_leds[i] = CHSV(s_hue, 255, 255);
    FastLED.show();
}

void UserInterface::timerStart(void)
{
    _display_timer.begin(update_display, 1000000);
    _led_timer.begin(update_leds, ((float)(_minutes * 60) / TIMER_HUE_MAX) * 1000000);
}

void UserInterface::timerStop(void)
{
    _display_timer.end();
    _led_timer.end();
}

void UserInterface::timerAlert(void)
{
    GPIO::set(PIN_BEEPER);
    //Display::time(0, 0);
    Display::done();
    for (uint8_t i = 0; i < NUM_LEDS; i++)
        s_leds[i] = CRGB(255, 0, 0);
    FastLED.show();
    //FastLED.showColor(CRGB(255, 0, 0));

    delay(500);

    GPIO::clear(PIN_BEEPER);
    Display::blank();
    FastLED.clear(true);

    delay(80);
}

void UserInterface::lowBattery(void)
{
    switch (_switch_state)
    {
        case SWITCH_STATE__NONE:
        case SWITCH_STATE__DEPRESSED:
            break;

        case SWITCH_STATE__SHORT_PRESS:
        case SWITCH_STATE__LONG_PRESS:
            _low_battery_ack = true;
            break;
    }

    if (!_low_battery_ack)
        lowBatteryAlert();
}

void UserInterface::lowBatteryAlert(void)
{
    static bool init = true;
    static uint16_t minute = 0;
    static uint16_t second = s_seconds;
    static int timer_show = 0;
    static int timer_state = 0;

    if (init)
    {
        if (second != 0)
        {
            timer_show = 1;

            while (second >= 60)
            {
                minute++;
                second -= 60;
            }
        }

        init = false;
    }

    GPIO::set(PIN_BEEPER);

    timer_state ^= timer_show;

    if (timer_state)
        Display::time(second, minute);
    else
        Display::lowBattery();

    delay(500);

    GPIO::clear(PIN_BEEPER);
    Display::blank();

    delay(80);
}
