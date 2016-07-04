#include "intr.h"
#include "pins.h"
#include <kinetis.h>
#include <cstdint>

#define ISFR_CLEAR_ALL   0xFFFFFFFF

void porta_isr(void);
void portb_isr(void);
void portc_isr(void);
void portd_isr(void);

namespace INTR
{
    isr_t isr[PIN_MAX];
    uint32_t porta_mask = 0;
    uint32_t portb_mask = 0;
    uint32_t portc_mask = 0;
    uint32_t portd_mask = 0;
    uint32_t* pin_mask[PIN_MAX];
    bool enabled = false;
    bool suspended = true;

    volatile uint32_t* getPort(uint8_t pin);

    inline bool validPin(uint8_t pin)
    {
        if (pin >= PIN_MAX)
            return false;

        return true;
    }

    inline bool pinAttached(uint8_t pin)
    {
        if (!validPin(pin))
            return false;

        uint32_t check_mask = porta_mask | portb_mask | portc_mask | portd_mask;

        return (check_mask & (1 << pin)) ? true : false;
    }
};

void INTR::init(void)
{
    if (INTR::enabled)
        return;

    for (uint8_t pin = 0; pin < PIN_MAX; pin++)
    {
        INTR::isr[pin] = (isr_t)0;

        switch (pin)
        {
            // Pin detect (Port A)
            case 3:
            case 4:
                INTR::pin_mask[pin] = &INTR::porta_mask;
                break;

                // Pin detect (Port B)
            case 0:
            case 1:
            case 16:
            case 17:
            case 18:
            case 19:
                INTR::pin_mask[pin] = &INTR::portb_mask;
                break;

                // Pin detect (Port C)
            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 15:
            case 22:
            case 23:
                INTR::pin_mask[pin] = &INTR::portc_mask;
                break;

                // Pin detect (Port D)
            case 2:
            case 5:
            case 6:
            case 7:
            case 8:
            case 14:
            case 20:
            case 21:
                INTR::pin_mask[pin] = &INTR::portd_mask;
                break;
        }
    }

    INTR::enabled = true;
    INTR::suspended = false;
}

void INTR::suspend(void)
{
    INTR::suspended = true;
}

void INTR::resume(void)
{
    INTR::suspended = false;
}

volatile uint32_t* INTR::getPort(uint8_t pin)
{
    switch (pin)
    {
        // Pin detect (Port A)
        case 3:
            return &PIN_PORT_REG(3);
        case 4:
            return &PIN_PORT_REG(4);

        // Pin detect (Port B)
        case 0:
            return &PIN_PORT_REG(0);
        case 1:
            return &PIN_PORT_REG(1);
        case 16:
            return &PIN_PORT_REG(16);
        case 17:
            return &PIN_PORT_REG(17);
        case 18:
            return &PIN_PORT_REG(18);
        case 19:
            return &PIN_PORT_REG(19);

        // Pin detect (Port C)
        case 9:
            return &PIN_PORT_REG(9);
        case 10:
            return &PIN_PORT_REG(10);
        case 11:
            return &PIN_PORT_REG(11);
        case 12:
            return &PIN_PORT_REG(12);
        case 13:
            return &PIN_PORT_REG(13);
        case 15:
            return &PIN_PORT_REG(15);
        case 22:
            return &PIN_PORT_REG(22);
        case 23:
            return &PIN_PORT_REG(23);

        // Pin detect (Port D)
        case 2:
            return &PIN_PORT_REG(2);
        case 5:
            return &PIN_PORT_REG(5);
        case 6:
            return &PIN_PORT_REG(6);
        case 7:
            return &PIN_PORT_REG(7);
        case 8:
            return &PIN_PORT_REG(8);
        case 14:
            return &PIN_PORT_REG(14);
        case 20:
            return &PIN_PORT_REG(20);
        case 21:
            return &PIN_PORT_REG(21);

        default:
            break;
    }

    return (volatile uint32_t*)0;
}

bool INTR::attach(uint8_t pin, isr_t isr, irqc_t irqc)
{
    if (!INTR::enabled || INTR::suspended
            || !INTR::validPin(pin) || INTR::pinAttached(pin))
    {
        return false;
    }

    volatile uint32_t* port_reg = INTR::getPort(pin);

    *INTR::pin_mask[pin] |= (1 << pin);

    // XXX Is this necessary?
    __disable_irq();

    INTR::isr[pin] = isr;

    *port_reg &= ~PORT_PCR_IRQC(0x0F);
    *port_reg |= PORT_PCR_IRQC(irqc) | PORT_PCR_ISF;

    __enable_irq();

    return true;
}

bool INTR::detach(uint8_t pin)
{
    if (!INTR::enabled || INTR::suspended
            || !INTR::validPin(pin) || !INTR::pinAttached(pin))
    {
        return false;
    }

    volatile uint32_t* port_reg = INTR::getPort(pin);

    *INTR::pin_mask[pin] &= ~(1 << pin);

    // XXX Is this necessary?
    __disable_irq();

    *port_reg &= ~PORT_PCR_IRQC(0x0F);
    *port_reg |= PORT_PCR_ISF;

    INTR::isr[pin] = (isr_t)0;

    __enable_irq();

    return true;
}

void INTR::clear(void)
{
    PORTA_ISFR = ISFR_CLEAR_ALL;
    PORTB_ISFR = ISFR_CLEAR_ALL;
    PORTC_ISFR = ISFR_CLEAR_ALL;
    PORTD_ISFR = ISFR_CLEAR_ALL;

    // Not sure if this is necessary but I don't think it can hurt
    NVIC_CLEAR_PENDING(IRQ_PORTA);
    NVIC_CLEAR_PENDING(IRQ_PORTB);
    NVIC_CLEAR_PENDING(IRQ_PORTC);
    NVIC_CLEAR_PENDING(IRQ_PORTD);
}

void porta_isr(void)
{
    if (INTR::suspended)
    {
        PORTA_ISFR = ISFR_CLEAR_ALL;
        return;
    }

    uint32_t pin_mask = INTR::porta_mask;

    while (pin_mask)
    {
        switch (__builtin_ctz(pin_mask))
        {
            case 3:  // PTA12
                if (INTR::check(3))
                {
                    INTR::clear(3);
                    INTR::isr[3]();
                }
                pin_mask &= ~(1 << 3);
                break;
            case 4:  // PTA13
                if (INTR::check(4))
                {
                    INTR::clear(4);
                    INTR::isr[4]();
                }
                pin_mask &= ~(1 << 4);
                break;
            default:  // Shouldn't happen
                pin_mask = 0;
                PORTA_ISFR = ISFR_CLEAR_ALL;
                break;
        }
    }
}

void portb_isr(void)
{
    if (INTR::suspended)
    {
        PORTB_ISFR = ISFR_CLEAR_ALL;
        return;
    }

    uint32_t pin_mask = INTR::portb_mask;

    while (pin_mask)
    {
        switch (__builtin_ctz(pin_mask))
        {
            case 0:  // PTB16
                if (INTR::check(0))
                {
                    INTR::clear(0);
                    INTR::isr[0]();
                }
                pin_mask &= ~(1 << 0);
                break;
            case 1:  // PTB17
                if (INTR::check(1))
                {
                    INTR::clear(1);
                    INTR::isr[1]();
                }
                pin_mask &= ~(1 << 1);
                break;
            case 16:  // PTB0
                if (INTR::check(16))
                {
                    INTR::clear(16);
                    INTR::isr[16]();
                }
                pin_mask &= ~(1 << 16);
                break;
            case 17:  // PTB1
                if (INTR::check(17))
                {
                    INTR::clear(17);
                    INTR::isr[17]();
                }
                pin_mask &= ~(1 << 17);
                break;
            case 18:  // PTB3
                if (INTR::check(18))
                {
                    INTR::clear(18);
                    INTR::isr[18]();
                }
                pin_mask &= ~(1 << 18);
                break;
            case 19:  // PTB2
                if (INTR::check(19))
                {
                    INTR::clear(19);
                    INTR::isr[19]();
                }
                pin_mask &= ~(1 << 19);
                break;
            default:  // Shouldn't happen
                pin_mask = 0;
                PORTB_ISFR = ISFR_CLEAR_ALL;
                break;
        }
    }
}

void portc_isr(void)
{
    if (INTR::suspended)
    {
        PORTC_ISFR = ISFR_CLEAR_ALL;
        return;
    }

    uint32_t pin_mask = INTR::portc_mask;

    while (pin_mask)
    {
        switch (__builtin_ctz(pin_mask))
        {
            case 9:  // PTC3
                if (INTR::check(9))
                {
                    INTR::clear(9);
                    INTR::isr[9]();
                }
                pin_mask &= ~(1 << 9);
                break;
            case 10:  // PTC4
                if (INTR::check(10))
                {
                    INTR::clear(10);
                    INTR::isr[10]();
                }
                pin_mask &= ~(1 << 10);
                break;
            case 11:  // PTC6
                if (INTR::check(11))
                {
                    INTR::clear(11);
                    INTR::isr[11]();
                }
                pin_mask &= ~(1 << 11);
                break;
            case 12:  // PTC7
                if (INTR::check(12))
                {
                    INTR::clear(12);
                    INTR::isr[12]();
                }
                pin_mask &= ~(1 << 12);
                break;
            case 13:  // PTC5
                if (INTR::check(13))
                {
                    INTR::clear(13);
                    INTR::isr[13]();
                }
                pin_mask &= ~(1 << 13);
                break;
            case 15:  // PTC0
                if (INTR::check(15))
                {
                    INTR::clear(15);
                    INTR::isr[15]();
                }
                pin_mask &= ~(1 << 15);
                break;
            case 22:  // PTC1
                if (INTR::check(22))
                {
                    INTR::clear(22);
                    INTR::isr[22]();
                }
                pin_mask &= ~(1 << 22);
                break;
            case 23:  // PTC2
                if (INTR::check(23))
                {
                    INTR::clear(23);
                    INTR::isr[23]();
                }
                pin_mask &= ~(1 << 23);
                break;
            default:  // Shouldn't happen
                pin_mask = 0;
                PORTC_ISFR = ISFR_CLEAR_ALL;
                break;
        }
    }
}

void portd_isr(void)
{
    if (INTR::suspended)
    {
        PORTD_ISFR = ISFR_CLEAR_ALL;
        return;
    }

    uint32_t pin_mask = INTR::portd_mask;

    while (pin_mask)
    {
        switch (__builtin_ctz(pin_mask))
        {
            case 2:  // PTD0
                if (INTR::check(2))
                {
                    INTR::clear(2);
                    INTR::isr[2]();
                }
                pin_mask &= ~(1 << 2);
                break;
            case 5:  // PTD7
                if (INTR::check(5))
                {
                    INTR::clear(5);
                    INTR::isr[5]();
                }
                pin_mask &= ~(1 << 5);
                break;
            case 6:  // PTD4
                if (INTR::check(6))
                {
                    INTR::clear(6);
                    INTR::isr[6]();
                }
                pin_mask &= ~(1 << 6);
                break;
            case 7:  // PTD2
                if (INTR::check(7))
                {
                    INTR::clear(7);
                    INTR::isr[7]();
                }
                pin_mask &= ~(1 << 7);
                break;
            case 8:  // PTD3
                if (INTR::check(8))
                {
                    INTR::clear(8);
                    INTR::isr[8]();
                }
                pin_mask &= ~(1 << 8);
                break;
            case 14:  // PTD1
                if (INTR::check(14))
                {
                    INTR::clear(14);
                    INTR::isr[14]();
                }
                pin_mask &= ~(1 << 14);
                break;
            case 20:  // PTD5
                if (INTR::check(20))
                {
                    INTR::clear(20);
                    INTR::isr[20]();
                }
                pin_mask &= ~(1 << 20);
                break;
            case 21:  // PTD6
                if (INTR::check(21))
                {
                    INTR::clear(21);
                    INTR::isr[21]();
                }
                pin_mask &= ~(1 << 21);
                break;
            default:  // Shouldn't happen
                pin_mask = 0;
                PORTD_ISFR = ISFR_CLEAR_ALL;
                break;
        }
    }
}
