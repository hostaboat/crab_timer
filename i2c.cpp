#include "i2c.h"
#include "pins.h"
#include <core_pins.h>
#include <kinetis.h>
#include <cstdint>

namespace I2C
{
    uint8_t tx_buffer[128];
    uint8_t tx_length = 0;
    bool busy = false;

    void setClock(uint32_t frequency);
};

bool I2C::init(void)
{
    if (I2C::enabled())
        return true;

    if ((PIN_SDA != 18) && (PIN_SCL != 19))
        return false;

    // XXX Check other I2C pins

    // Open gate
    I2C::enable();

    if (PIN_SDA == 18)
    {
        CORE_PIN18_CONFIG =
            PORT_PCR_MUX(2) | PORT_PCR_ODE | PORT_PCR_SRE | PORT_PCR_DSE;
    }

    if (PIN_SCL == 19)
    {
        CORE_PIN19_CONFIG =
            PORT_PCR_MUX(2) | PORT_PCR_ODE | PORT_PCR_SRE | PORT_PCR_DSE;
    }

    I2C::setClock(400000);
    I2C::start();

    return true;
}

void I2C::enable(void)
{
    SIM_SCGC4 |= SIM_SCGC4_I2C0;
}

void I2C::disable(void)
{
    SIM_SCGC4 &= ~SIM_SCGC4_I2C0;
}

bool I2C::enabled(void)
{
    return (SIM_SCGC4 & SIM_SCGC4_I2C0);
}

void I2C::start(void)
{
    if (I2C::running())
        return;

    I2C0_C2 = I2C_C2_HDRS;
    I2C0_C1 = I2C_C1_IICEN;
}

void I2C::stop(void)
{
    if (!I2C::running())
        return;

    I2C0_C1 = 0;
    I2C0_C2 = 0;
}

bool I2C::running(void)
{
    return (I2C0_C1 & I2C_C1_IICEN);
}

bool I2C::isBusy(void)
{
    return I2C::busy;
}

bool I2C::begin(uint8_t slave_addr)
{
    if (I2C::busy)
        return false;

    I2C::busy = true;

    I2C::tx_buffer[0] = (slave_addr << 1);
    I2C::tx_length = 1;

    return true;
}

bool I2C::write(uint8_t data)
{
    if (I2C::tx_length == sizeof(I2C::tx_buffer))
        return false;

    I2C::tx_buffer[I2C::tx_length++] = data;

    return true;
}

bool I2C::end(void)
{
    if (I2C::tx_length == 0)
    {
        I2C::busy = false;
        return false;
    }

    uint8_t tx_len = I2C::tx_length;

    I2C::tx_length = 0;

    I2C0_S = I2C_S_ARBL | I2C_S_IICIF;

    // If already master use repeated START
    if (I2C0_C1 & I2C_C1_MST)
        I2C0_C1 |= I2C_C1_RSTA | I2C_C1_TX;
    else
        I2C0_C1 |= I2C_C1_MST | I2C_C1_TX;

    // Wait until START condition
    while (!(I2C0_S & I2C_S_BUSY));

    // Number of times to try a repeated START
    // if byte transmission fails.
    uint8_t retries = 16;

    for (uint8_t i = 0; i < tx_len; i++)
    {
        I2C0_D = I2C::tx_buffer[i];

        while (!(I2C0_S & I2C_S_IICIF));

        I2C0_S |= I2C_S_IICIF;

        if (!(I2C0_S & I2C_S_TCF))
        {
            if (retries == 0)
            {
                I2C0_C1 &= ~I2C_C1_MST;
                I2C::busy = false;
                return false;
            }
            else
            {
                retries--;
                I2C0_C1 |= I2C_C1_RSTA;
            }
        }
    }

    I2C::busy = false;
    return true;
}

void I2C::setClock(uint32_t frequency)
{
#if F_BUS == 60000000
    if (frequency < 400000)
        I2C0_F = 0x2C;  // 104 kHz
    else if (frequency < 1000000)
        I2C0_F = 0x1C;  // 416 kHz
    else
        I2C0_F = 0x12;  // 938 kHz
    I2C0_FLT = 4;
#elif F_BUS == 56000000
    if (frequency < 400000)
        I2C0_F = 0x2B;  // 109 kHz
    else if (frequency < 1000000)
        I2C0_F = 0x1C;  // 389 kHz
    else
        I2C0_F = 0x0E;  // 1 MHz
    I2C0_FLT = 4;
#elif F_BUS == 48000000
    if (frequency < 400000)
        I2C0_F = 0x27;  // 100 kHz 
    else if (frequency < 1000000)
        I2C0_F = 0x1A;  // 400 kHz
    else
        I2C0_F = 0x0D;  // 1 MHz
    I2C0_FLT = 4;
#elif F_BUS == 40000000
    if (frequency < 400000)
        I2C0_F = 0x29;  // 104 kHz
    else if (frequency < 1000000)
        I2C0_F = 0x19;  // 416 kHz
    else
        I2C0_F = 0x0B;  // 1 MHz
    I2C0_FLT = 3;
#elif F_BUS == 36000000
    if (frequency < 400000)
        I2C0_F = 0x28;  // 113 kHz
    else if (frequency < 1000000)
        I2C0_F = 0x19;  // 375 kHz
    else
        I2C0_F = 0x0A;  // 1 MHz
    I2C0_FLT = 3;
#elif F_BUS == 24000000
    if (frequency < 400000)
        I2C0_F = 0x1F;  // 100 kHz
    else if (frequency < 1000000)
        I2C0_F = 0x12;  // 375 kHz
    else
        I2C0_F = 0x02;  // 1 MHz
    I2C0_FLT = 2;
#elif F_BUS == 16000000
    if (frequency < 400000)
        I2C0_F = 0x20;  // 100 kHz
    else if (frequency < 1000000)
        I2C0_F = 0x07;  // 400 kHz
    else
        I2C0_F = 0x00;  // 800 MHz
    I2C0_FLT = 1;
#elif F_BUS == 8000000
    if (frequency < 400000)
        I2C0_F = 0x14;  // 100 kHz
    else
        I2C0_F = 0x00;  // 400 kHz
    I2C0_FLT = 1;
#elif F_BUS == 4000000
    if (frequency < 400000)
        I2C0_F = 0x07;  // 100 kHz
    else
        I2C0_F = 0x00;  // 200 kHz
    I2C0_FLT = 1;
#elif F_BUS == 2000000
    I2C0_F = 0x00;  // 100 kHz
    I2C0_FLT = 1;
#else
# error "F_BUS not valid"
#endif
}
