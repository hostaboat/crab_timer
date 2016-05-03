#include <IntervalTimer.h>
#include <FastLED.h>
#include <elapsedMillis.h>
#include <cstdint>
#include "pins.h"
#include "interface.h"
#include "display.h"
#include "player.h"
#include "llwu.h"
#include "nvm.h"
#include "intr.h"

#define RESET_TIME  2000  // 2 seconds of continuous pushing of switch to reset

#define SLEEP_TIME_DEFAULT  15000  // 15 seconds before sleep

#define LLWU_LPTMR_PERIOD   60000  // 60 seconds since 1kHz LPO timer is used
#define LLWU_LPTMR_PERIODS     15  // Wakeup from LLS 15 times before
                                   //  disabling player - 15 minutes.

#define STOP_TIME_PLAYER   (LLWU_LPTMR_PERIOD * LLWU_LPTMR_PERIODS)

#define LED_BRIGHTNESS   128
#define NUM_LEDS          16
static CRGB s_leds[NUM_LEDS];

#define TIMER_HUE_MAX   160  // Start with blue hue and decrease to red hue
#define TIMER_HUE_MIN     0  // Red hue

// *****************************************************************************
// Global variables used in interrupt handlers
static volatile int8_t s_encoder_turn = 0;
static volatile int8_t s_encoder_state = 0;
// Upper and lower are used to normalize initial state of encoder
static volatile int8_t s_encoder_upper = 0;
static volatile int8_t s_encoder_lower = 0;
static volatile switch_state_t s_switch_state = SWITCH_STATE__NONE;
static volatile uint32_t s_switch_depressed;

static volatile uint16_t s_seconds = 0;
static volatile uint8_t s_hue = 160;
static volatile uint8_t s_brightness = LED_BRIGHTNESS;

// *****************************************************************************


// *****************************************************************************
// Interrupt handlers
static void encoder_rotate(void);
static void switch_pressed(void);
static void timer_display(void);
static void timer_leds(void);
static void encoder_brightness(void);

static void encoder_rotate(void)
{
    if (GPIO::read(PIN_ROT_ENC_SW) == HIGH)
        return;

    static const uint8_t num_transitions = 4;
    static int8_t inc = 0;
    uint8_t a = GPIO::read(PIN_ROT_ENC_A);
    uint8_t b = GPIO::read(PIN_ROT_ENC_B);

    switch (s_encoder_state)
    {
        case 0:
        case 2:
            inc = (a == HIGH) ? (inc + 1) : (inc - 1);
            break;

        case 1:
        case 3:
            inc = (a == LOW) ? (inc + 1) : (inc - 1);
            break;

        default:
            break;
    }

    s_encoder_state = (a << 1) | b;

    // Since initial encoder state might not be zero these are used to
    // normalize encoder so that zero is the nominal state.  Once
    // reached encoder is normalized and they are not needed.
    if (s_encoder_upper || s_encoder_lower)
    {
        if ((inc != s_encoder_upper) && (inc != s_encoder_lower))
            return;

        if (inc == s_encoder_upper)
            inc = num_transitions;
        else
            inc = -num_transitions;

        s_encoder_upper = s_encoder_lower = 0;
    }

    if (inc == num_transitions)
    {
        s_encoder_turn = 1;
        inc = 0;
    }
    else if (inc == -num_transitions)
    {
        s_encoder_turn = -1;
        inc = 0;
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

static void timer_display(void)
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

    if (!Display::isOn())
        return;

    Display::time(second, minute);
}

static void timer_leds(void)
{
    if (s_hue == TIMER_HUE_MIN)
        return;

    s_hue--;

    for (uint8_t i = 0; i < NUM_LEDS; i++)
        s_leds[i] = CHSV(s_hue, 255, 255);

    if (s_brightness == 0)
        return;

    FastLED.show();
}

static void encoder_brightness(void)
{
    if (Display::isBusy())
        return;

    if (GPIO::read(PIN_ROT_ENC_BR_A) != GPIO::read(PIN_ROT_ENC_BR_B))
    {
        if (s_brightness == 255)
            return;

        s_brightness++;

        if (((s_brightness % 16) == 0) || (s_brightness == 255))
            Display::up();

        FastLED.setBrightness(s_brightness);
        FastLED.show();
    }
    else
    {
        if (s_brightness == 0)
            return;

        s_brightness--;

        if ((s_brightness % 16) == 0)
            Display::down();

        FastLED.setBrightness(s_brightness);
        FastLED.show();
    }
}

// *****************************************************************************


// *****************************************************************************
// Member functions
UserInterface::UserInterface(void)
    : _encoder_turn(0), _switch_state(SWITCH_STATE__NONE),
      _sleep(0), _sleep_time(SLEEP_TIME_DEFAULT),
      _ui_state(UI_STATE__PASS), _last_ui_state(UI_STATE__PASS),
      _total_count(0), _count(0), _default_minutes(0), _minutes(0),
      _timer_state(TIMER_STATE__INIT), _timer_pause(1),
      _brightness(s_brightness)
{
}

void UserInterface::init(void)
{
    FastLED.addLeds<NEOPIXEL, PIN_NEOPIXEL>(s_leds, NUM_LEDS);
    FastLED.setBrightness(s_brightness);

    INTR::init();
    (void)NVM::init();
    (void)Display::init(DISPLAY_MID_BRIGHTNESS);
    (void)Player::init();

    encoderInit();

    (void)INTR::attach(PIN_ROT_ENC_A, encoder_rotate, IRQC_CHANGE);
    (void)INTR::attach(PIN_ROT_ENC_B, encoder_rotate, IRQC_CHANGE);
    (void)INTR::attach(PIN_ROT_ENC_SW, switch_pressed, IRQC_CHANGE);

    (void)INTR::attach(PIN_ROT_ENC_BR_A, encoder_brightness, IRQC_CHANGE);

    (void)NVM::read16(NVM_COUNT_INDEX, _total_count);
    (void)NVM::read16(NVM_TIME_INDEX, _default_minutes);
    _minutes = _default_minutes;
}

void UserInterface::encoderInit(void)
{
    // Since initial state of the encoder may not be in the nominal position
    // of both reading zero, need to set initial number of transitions
    // before a turn is counted.
    s_encoder_state = (GPIO::read(PIN_ROT_ENC_A) << 1) | GPIO::read(PIN_ROT_ENC_B);

    switch (s_encoder_state)
    {
        case 0:
            s_encoder_upper = 4;
            s_encoder_lower = -4;
            break;

        case 1:
            s_encoder_upper = 5;
            s_encoder_lower = -3;
            break;

        case 2:
            s_encoder_upper = 3;
            s_encoder_lower = -5;
            break;

        case 3:
            s_encoder_upper = 6;
            s_encoder_lower = -6;
            break;

        default:
            break;
    }
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

        case UI_STATE__PASS:
            break;
    }
}

ui_state_t UserInterface::getState(void)
{
    if (GPIO::read(PIN_ROT_SWI_A) == LOW)
        _ui_state = UI_STATE__COUNT;
    else if (GPIO::read(PIN_ROT_SWI_B) == LOW)
        _ui_state = UI_STATE__SET;
    else if (GPIO::read(PIN_ROT_SWI_C) == LOW)
        _ui_state = UI_STATE__TIMER;
    else
        _ui_state = UI_STATE__PASS;

    return _ui_state;
}

bool UserInterface::sleep(void)
{
    static uint8_t brightness = 0;

    switch (_ui_state)
    {
        case UI_STATE__COUNT:
        case UI_STATE__SET:
        case UI_STATE__PASS:
            break;

        case UI_STATE__TIMER:
            if (!_timer_pause)
            {
                // If timer is active can just stop player, but have to
                // continue processing.
                if (Player::occupied() || Player::isStopped())
                    _sleep = millis();
                else if ((millis() - _sleep) > STOP_TIME_PLAYER)
                    Player::stop();

                return false;
            }
            break;
    }

    if ((_encoder_turn != 0) || (_switch_state != SWITCH_STATE__NONE))
    {
        _sleep = millis();

        // Treat encoder turn or switch press as a wakeup
        // and don't process in usual way
        if (!Display::isAwake())
        {
            Display::wake();
            FastLED.show(s_brightness);
            return true;
        }

        return false;
    }
    else if (brightness != _brightness)
    {
        brightness = _brightness;
        _sleep = millis();

        return false;
    }
    else if (!Display::isAwake() && Player::occupied())
    {
        // Display is asleep but Player is occupied - playing or button is pressed
        _sleep = millis();
        return true;
    }
    else if ((millis() - _sleep) < _sleep_time)
    {
        return false;
    }

    // If the player is occupied just turn the leds and display off
    if (Player::occupied())
    {
        Display::standby();
        FastLED.show(0);
        return true;
    }

    // Turns CPU clocks off saving ~44mA on MCU.
    // If the VS1053 also gets turned off (~14mA) should get a total of ~58mA
    // savings putting things ~17mA which should be about what the LED on the
    // PowerBoost is consuming and the Neopixel strip which despite being "off"
    // still consumes ~7mA.

    LLWU::enable();

    if (!LLWU::wakeupPinEnable(PIN_ROT_ENC_SW, WUPE_RISING)
            || !LLWU::wakeupPinEnable(PIN_ROT_ENC_A, WUPE_CHANGE)
            || !LLWU::wakeupPinEnable(PIN_ROT_ENC_BR_A, WUPE_CHANGE)
            || !LLWU::wakeupPinEnable(PIN_AUDIO_PLAY, WUPE_RISING)
            || !LLWU::wakeupLPTMREnable(LLWU_LPTMR_PERIOD))
    {
        LLWU::disable();
        return false;
    }

    INTR::suspend();

    Display::standby();
    FastLED.show(0);

    wus_t wakeup_source;
    int8_t wakeup_pin = -1;

    // Sleep for number of periods as long as wakeup source is LPTMR
    for (uint8_t i = 0; i < LLWU_LPTMR_PERIODS; i++)
    {
        wakeup_source = LLWU::sleep();
        if (wakeup_source != WUS_TIMER)
            break;
    }

    // If the wakeup source is still LPTMR, disable it and put the
    // VS1053 to sleep.
    if ((wakeup_source == WUS_TIMER) && LLWU::wakeupLPTMRDisable())
    {
        // This will turn the player off saving ~15mA more
        Player::stop();
        wakeup_source = LLWU::sleep();
    }

    if (wakeup_source == WUS_PIN)
        wakeup_pin = LLWU::wakeupPin();

    LLWU::disable();

    while (GPIO::read(PIN_ROT_ENC_SW) == HIGH);
    while (GPIO::read(PIN_AUDIO_PLAY) == HIGH);

    INTR::clear();
    INTR::resume();

    if (wakeup_pin == PIN_AUDIO_PLAY)
    {
        Player::resume();
    }
    else
    {
        Display::wake();
        FastLED.show(s_brightness);
    }

    _sleep = millis();

    return true;
}

ui_state_t UserInterface::getInput(void)
{
    uint32_t switch_depressed = 0;

    noInterrupts();

    if (getState() != _last_ui_state)
    {
        stateInit();
        _last_ui_state = _ui_state;
        _sleep = millis();
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
    else if (s_brightness != _brightness)
    {
        _brightness = s_brightness;
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

    if (sleep())
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
                (void)NVM::write(NVM_COUNT_INDEX, _total_count);
                Display::count(_total_count);
                break;

            case SWITCH_STATE__LONG_PRESS:
                _count = _total_count = 0;
                (void)NVM::write(NVM_COUNT_INDEX, 0);
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
                (void)NVM::write(NVM_TIME_INDEX, _default_minutes);
                break;

            case SWITCH_STATE__LONG_PRESS:
                (void)NVM::read16(NVM_TIME_INDEX, _default_minutes);
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
            _sleep = millis();
            break;

        case SWITCH_STATE__LONG_PRESS:
            FastLED.clear(true);
            timerStop();
            _timer_state = TIMER_STATE__INIT;
            _timer_pause = 1;
            Display::time(0, _minutes);
            _sleep = millis();
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
            {
                _timer_state = TIMER_STATE__DONE;
                _timer_pause = 0;
            }
            else
            {
                timerAlert();  // Blocking until switch pressed
            }
            break;

        case TIMER_STATE__DONE:
            if (switch_pressed)
            {
                Display::time(0, _minutes);
                _timer_pause = 1;
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
    _display_timer.begin(timer_display, 1000000);
    _led_timer.begin(timer_leds, ((float)(_minutes * 60) / TIMER_HUE_MAX) * 1000000);
}

void UserInterface::timerStop(void)
{
    _display_timer.end();
    _led_timer.end();
}

void UserInterface::timerAlert(void)
{
    for (uint8_t i = 0; i < NUM_LEDS; i++)
        s_leds[i] = CRGB(255, 0, 0);

    while (s_switch_state == SWITCH_STATE__NONE)
    {
        Display::done();
        FastLED.show(s_brightness);
        GPIO::set(PIN_BEEPER);

        delay(500);

        Display::blank();
        FastLED.show(0);
        GPIO::clear(PIN_BEEPER);

        delay(80);
    }

    FastLED.clear(true);
}
