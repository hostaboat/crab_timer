// HT16K33 7 Segment Display **************************************************
#include "display.h"
#include <cstdint>
#include <Wire.h>

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

void Display::init(void)
{
    Wire.begin();

    Display::wake();
    Display::on();
    Display::brightness(Display::blevel);
    Display::blank();
}

void Display::wake(void)
{
    if (!Display::awake)
    {
        Wire.beginTransmission(HT16K33_DEV_ADDR);
        Wire.write(HT16K33_WAKEUP);
        Wire.endTransmission();

        Display::awake = true;
    }
}

void Display::standby(void)
{
    if (Display::awake)
    {
        Wire.beginTransmission(HT16K33_DEV_ADDR);
        Wire.write(HT16K33_STANDBY);
        Wire.endTransmission();

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
        Wire.beginTransmission(HT16K33_DEV_ADDR);
        Wire.write(HT16K33_ON);
        Wire.endTransmission();

        Display::lit = true;
    }
}

void Display::off(void)
{
    if (Display::lit)
    {
        Wire.beginTransmission(HT16K33_DEV_ADDR);
        Wire.write(HT16K33_OFF);
        Wire.endTransmission();

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

    Wire.beginTransmission(HT16K33_DEV_ADDR);
    Wire.write(HT16K33_BRIGHTNESS_REG | level);
    Wire.endTransmission();
}

void Display::count(uint16_t count)
{
    if (!Display::awake)
        Display::wake();

    Wire.beginTransmission(HT16K33_DEV_ADDR);
    Wire.write(0x00);  // Command code

    if (count > 9999)
    {
        Wire.write(Display::number_table[9]);
        Wire.write(0x00);

        Wire.write(Display::number_table[9]);
        Wire.write(0x00);

        Wire.write(Display::number_table[9]);
        Wire.write(0x00);

        Wire.write(Display::number_table[9]);
        Wire.write(0x00);
    }
    else if (count >= 1000)
    {
        Wire.write(Display::number_table[count / 1000]);
        Wire.write(0x00);
        count %= 1000;

        Wire.write(Display::number_table[count / 100]);
        Wire.write(0x00);
        count %= 100;

        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(Display::number_table[count / 10]);
        Wire.write(0x00);
        count %= 10;

        Wire.write(Display::number_table[count]);
        Wire.write(0x00);
    }
    else if (count >= 100)
    {
        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(Display::number_table[count / 100]);
        Wire.write(0x00);
        count %= 100;

        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(Display::number_table[count / 10]);
        Wire.write(0x00);
        count %= 10;

        Wire.write(Display::number_table[count]);
        Wire.write(0x00);
    }
    else if (count >= 10)
    {
        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(Display::number_table[count / 10]);
        Wire.write(0x00);
        count %= 10;

        Wire.write(Display::number_table[count]);
        Wire.write(0x00);
    }
    else
    {
        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(0x00);
        Wire.write(0x00);

        Wire.write(Display::number_table[count]);
        Wire.write(0x00);
    }

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        Wire.write(0x00);
        Wire.write(0x00);
    }

    Wire.endTransmission();

    if (!Display::lit)
        Display::on();
}

void Display::time(uint8_t second, uint8_t minute)
{
    if (!Display::awake)
        Display::wake();

    Wire.beginTransmission(HT16K33_DEV_ADDR);
    Wire.write(0x00);  // Command code

    // First and second digits
    if (minute == 0)
    {
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.write(0x00);
    }
    else if (minute < 10)
    {
        //Wire.write(Display::number_table[0]);
        Wire.write(0x00);
        Wire.write(0x00);
        Wire.write(Display::number_table[minute]);
    }
    else if (minute < 100)
    {
        Wire.write(Display::number_table[minute / 10]);
        Wire.write(0x00);
        Wire.write(Display::number_table[minute % 10]);
    }
    else
    {
        Wire.write(Display::number_table[9]);
        Wire.write(0x00);
        Wire.write(Display::number_table[9]);
    }

    // For some reason 16 bits needs to be written but only need 8 bits to represent digit
    Wire.write(0x00);

    // Colon
    Wire.write(HT16K33_COLON);
    Wire.write(0x00);

    // Third and fourth digits
    if (second < 10)
    {
        Wire.write(Display::number_table[0]);
        Wire.write(0x00);
        Wire.write(Display::number_table[second]);
    }
    else if (second < 60)
    {
        Wire.write(Display::number_table[second / 10]);
        Wire.write(0x00);
        Wire.write(Display::number_table[second % 10]);
    }
    else
    {
        Wire.write(Display::number_table[5]);
        Wire.write(0x00);
        Wire.write(Display::number_table[9]);
    }

    Wire.write(0x00);

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        Wire.write(0x00);
        Wire.write(0x00);
    }

    Wire.endTransmission();

    if (!Display::lit)
        Display::on();
}

void Display::blank(void)
{
    if (!Display::awake)
        Display::wake();

    Wire.beginTransmission(HT16K33_DEV_ADDR);
    Wire.write(0x00);  // Command code

    // Doesn't turn it off or put it in standby but just blanks it
    for (int i = 0; i < 8; i++)
    {
        Wire.write(0x00);
        Wire.write(0x00);
    }

    Wire.endTransmission();
}

void Display::dashes(void)
{
    if (!Display::awake)
        Display::wake();

    Wire.beginTransmission(HT16K33_DEV_ADDR);
    Wire.write(0x00);  // Command code

    for (int i = 0; i < 6; i++)
    {
        Wire.write(0x40);  // Dash
        Wire.write(0x00);
    }

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        Wire.write(0x00);
        Wire.write(0x00);
    }

    Wire.endTransmission();

    if (!Display::lit)
        Display::on();
}

void Display::done(void)
{
    if (!Display::awake)
        Display::wake();

    Wire.beginTransmission(HT16K33_DEV_ADDR);
    Wire.write(0x00);  // Command code

    // d
    Wire.write(0x5E);
    Wire.write(0x00);

    // o
    Wire.write(0x5C);
    Wire.write(0x00);

    // No colon
    Wire.write(0x00);
    Wire.write(0x00);

    // n
    Wire.write(0x54);
    Wire.write(0x00);

    // E
    Wire.write(0x79);
    Wire.write(0x00);

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        Wire.write(0x00);
        Wire.write(0x00);
    }

    Wire.endTransmission();

    if (!Display::lit)
        Display::on();
}

void Display::lowBattery(void)
{
    if (!Display::awake)
        Display::wake();

    Wire.beginTransmission(HT16K33_DEV_ADDR);
    Wire.write(0x00);  // Command code

    // L
    Wire.write(0x38);
    Wire.write(0x00);

    // o.
    //Wire.write(0x5C);
    Wire.write(0xDC);
    Wire.write(0x00);

    // No colon
    Wire.write(0x00);
    Wire.write(0x00);

    // b
    Wire.write(0x7C);
    Wire.write(0x00);

    // A.
    //Wire.write(0x77);
    Wire.write(0xF7);
    Wire.write(0x00);

    // Not sure if this is necessary
    for (int i = 6; i < 8; i++)
    {
        Wire.write(0x00);
        Wire.write(0x00);
    }

    Wire.endTransmission();

    if (!Display::lit)
        Display::on();
}

