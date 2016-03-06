// HT16K33 7 Segment Display **************************************************
#include "display.h"
#include "i2c.h"
#include <cstdint>

// I2C Device Address
#define HT16K33_DEV_ADDR    0x70

#define HT16K33_SYSTEM_SETUP_REG  0x20
#define HT16K33_OSCILLATOR_OFF    0x00
#define HT16K33_OSCILLATOR_ON     0x01
#define HT16K33_STANDBY           HT16K33_SYSTEM_SETUP_REG | HT16K33_OSCILLATOR_OFF
#define HT16K33_WAKEUP            HT16K33_SYSTEM_SETUP_REG | HT16K33_OSCILLATOR_ON

#define HT16K33_DISPLAY_SETUP_REG  0x80 
#define HT16K33_DISPLAY_OFF        0x00 
#define HT16K33_DISPLAY_ON         0x01 
#define HT16K33_BLINK_OFF         (0 << 1)
#define HT16K33_BLINK_2HZ         (1 << 1)
#define HT16K33_BLINK_1HZ         (2 << 1)
#define HT16K33_BLINK_HALFHZ      (3 << 1)
#define HT16K33_ON                HT16K33_DISPLAY_SETUP_REG | HT16K33_DISPLAY_ON | HT16K33_BLINK_OFF
#define HT16K33_OFF               HT16K33_DISPLAY_SETUP_REG | HT16K33_DISPLAY_OFF | HT16K33_BLINK_OFF

#define HT16K33_BRIGHTNESS_REG  0xE0 
#define HT16K33_MAX_BRIGHTNESS  0x0F
#define HT16K33_MID_BRIGHTNESS  0x07
#define HT16K33_LOW_BRIGHTNESS  0x00

#define HT16K33_COLON  0x02

namespace Display
{
    bool awake = false;
    bool lit = false;
    uint8_t blevel = HT16K33_MID_BRIGHTNESS;

    // Maps a number to bit mask to display number via HT16K33
    const uint8_t number_table[] = {
        0x3F,  // 0
        0x06,  // 1
        0x5B,  // 2
        0x4F,  // 3
        0x66,  // 4
        0x6D,  // 5
        0x7D,  // 6
        0x07,  // 7
        0x7F,  // 8
        0x6F,  // 9
        0x77,  // A
        0x7C,  // B
        0x39,  // C
        0x5E,  // D
        0x79,  // E
        0x71   // F
    };
};

bool Display::init(void)
{
    if (!I2C::init())
        return false;

    Display::wake();
    Display::brightness(Display::blevel);
    Display::blank();
    Display::on();

    return true;
}

void Display::wake(void)
{
    if (!Display::awake)
    {
        I2C::begin(HT16K33_DEV_ADDR);
        I2C::write(HT16K33_WAKEUP);
        I2C::end();

        Display::awake = true;
    }
}

void Display::standby(void)
{
    if (Display::awake)
    {
        I2C::begin(HT16K33_DEV_ADDR);
        I2C::write(HT16K33_STANDBY);
        I2C::end();

        Display::awake = false;
    }
}

bool Display::isAwake(void)
{
    return Display::awake;
}

void Display::on(void)
{
    if (!Display::lit)
    {
        I2C::begin(HT16K33_DEV_ADDR);
        I2C::write(HT16K33_ON);
        I2C::end();

        Display::lit = true;
    }
}

void Display::off(void)
{
    if (Display::lit)
    {
        I2C::begin(HT16K33_DEV_ADDR);
        I2C::write(HT16K33_OFF);
        I2C::end();

        Display::lit = false;
    }
}

bool Display::isOn(void)
{
    return Display::lit;
}

void Display::brightness(uint8_t level)
{
    if (level > HT16K33_MAX_BRIGHTNESS)
        level = HT16K33_MAX_BRIGHTNESS;

    Display::blevel = level;

    I2C::begin(HT16K33_DEV_ADDR);
    I2C::write(HT16K33_BRIGHTNESS_REG | level);
    I2C::end();
}

void Display::count(uint16_t count)
{
    if (!Display::awake)
        Display::wake();

    I2C::begin(HT16K33_DEV_ADDR);
    I2C::write(0x00);  // Command code

    if (count > 9999)
    {
        I2C::write(Display::number_table[9]);
        I2C::write(0x00);

        I2C::write(Display::number_table[9]);
        I2C::write(0x00);

        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(Display::number_table[9]);
        I2C::write(0x00);

        I2C::write(Display::number_table[9]);
        I2C::write(0x00);
    }
    else if (count >= 1000)
    {
        I2C::write(Display::number_table[count / 1000]);
        I2C::write(0x00);
        count %= 1000;

        I2C::write(Display::number_table[count / 100]);
        I2C::write(0x00);
        count %= 100;

        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(Display::number_table[count / 10]);
        I2C::write(0x00);
        count %= 10;

        I2C::write(Display::number_table[count]);
        I2C::write(0x00);
    }
    else if (count >= 100)
    {
        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(Display::number_table[count / 100]);
        I2C::write(0x00);
        count %= 100;

        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(Display::number_table[count / 10]);
        I2C::write(0x00);
        count %= 10;

        I2C::write(Display::number_table[count]);
        I2C::write(0x00);
    }
    else if (count >= 10)
    {
        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(Display::number_table[count / 10]);
        I2C::write(0x00);
        count %= 10;

        I2C::write(Display::number_table[count]);
        I2C::write(0x00);
    }
    else
    {
        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(0x00);
        I2C::write(0x00);

        I2C::write(Display::number_table[count]);
        I2C::write(0x00);
    }

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        I2C::write(0x00);
        I2C::write(0x00);
    }

    I2C::end();

    if (!Display::lit)
        Display::on();
}

void Display::time(uint8_t second, uint8_t minute)
{
    if (!Display::awake)
        Display::wake();

    I2C::begin(HT16K33_DEV_ADDR);
    I2C::write(0x00);  // Command code

    // First and second digits
    if (minute == 0)
    {
        I2C::write(0x00);
        I2C::write(0x00);
        I2C::write(0x00);
    }
    else if (minute < 10)
    {
        //I2C::write(Display::number_table[0]);
        I2C::write(0x00);
        I2C::write(0x00);
        I2C::write(Display::number_table[minute]);
    }
    else if (minute < 100)
    {
        I2C::write(Display::number_table[minute / 10]);
        I2C::write(0x00);
        I2C::write(Display::number_table[minute % 10]);
    }
    else
    {
        I2C::write(Display::number_table[9]);
        I2C::write(0x00);
        I2C::write(Display::number_table[9]);
    }

    // For some reason 16 bits needs to be written but only need 8 bits to represent digit
    I2C::write(0x00);

    // Colon
    I2C::write(HT16K33_COLON);
    I2C::write(0x00);

    // Third and fourth digits
    if (second < 10)
    {
        I2C::write(Display::number_table[0]);
        I2C::write(0x00);
        I2C::write(Display::number_table[second]);
    }
    else if (second < 60)
    {
        I2C::write(Display::number_table[second / 10]);
        I2C::write(0x00);
        I2C::write(Display::number_table[second % 10]);
    }
    else
    {
        I2C::write(Display::number_table[5]);
        I2C::write(0x00);
        I2C::write(Display::number_table[9]);
    }

    I2C::write(0x00);

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        I2C::write(0x00);
        I2C::write(0x00);
    }

    I2C::end();

    if (!Display::lit)
        Display::on();
}

void Display::blank(void)
{
    if (!Display::awake)
        Display::wake();

    I2C::begin(HT16K33_DEV_ADDR);
    I2C::write(0x00);  // Command code

    // Doesn't turn it off or put it in standby but just blanks it
    for (int i = 0; i < 8; i++)
    {
        I2C::write(0x00);
        I2C::write(0x00);
    }

    I2C::end();
}

void Display::dashes(void)
{
    if (!Display::awake)
        Display::wake();

    I2C::begin(HT16K33_DEV_ADDR);
    I2C::write(0x00);  // Command code

    for (int i = 0; i < 6; i++)
    {
        I2C::write(0x40);  // Dash
        I2C::write(0x00);
    }

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        I2C::write(0x00);
        I2C::write(0x00);
    }

    I2C::end();

    if (!Display::lit)
        Display::on();
}

void Display::done(void)
{
    if (!Display::awake)
        Display::wake();

    I2C::begin(HT16K33_DEV_ADDR);
    I2C::write(0x00);  // Command code

    // d
    I2C::write(0x5E);
    I2C::write(0x00);

    // o
    I2C::write(0x5C);
    I2C::write(0x00);

    // No colon
    I2C::write(0x00);
    I2C::write(0x00);

    // n
    I2C::write(0x54);
    I2C::write(0x00);

    // E
    I2C::write(0x79);
    I2C::write(0x00);

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        I2C::write(0x00);
        I2C::write(0x00);
    }

    I2C::end();

    if (!Display::lit)
        Display::on();
}

void Display::lowBattery(void)
{
    if (!Display::awake)
        Display::wake();

    I2C::begin(HT16K33_DEV_ADDR);
    I2C::write(0x00);  // Command code

    // L
    I2C::write(0x38);
    I2C::write(0x00);

    // o.
    //I2C::write(0x5C);
    I2C::write(0xDC);
    I2C::write(0x00);

    // No colon
    I2C::write(0x00);
    I2C::write(0x00);

    // b
    I2C::write(0x7C);
    I2C::write(0x00);

    // A.
    //I2C::write(0x77);
    I2C::write(0xF7);
    I2C::write(0x00);

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        I2C::write(0x00);
        I2C::write(0x00);
    }

    I2C::end();

    if (!Display::lit)
        Display::on();
}

