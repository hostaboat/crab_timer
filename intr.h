#ifndef _INTR_H_
#define _INTR_H_

#include "pins.h"
#include <cstdint>

#define PORT_PIN0_ISFR  (*(volatile uint32_t *)0x42941440)
#define PORT_PIN1_ISFR  (*(volatile uint32_t *)0x42941444)
#define PORT_PIN2_ISFR  (*(volatile uint32_t *)0x42981400)
#define PORT_PIN3_ISFR  (*(volatile uint32_t *)0x42921430)
#define PORT_PIN4_ISFR  (*(volatile uint32_t *)0x42921434)
#define PORT_PIN5_ISFR  (*(volatile uint32_t *)0x4298141c)
#define PORT_PIN6_ISFR  (*(volatile uint32_t *)0x42981410)
#define PORT_PIN7_ISFR  (*(volatile uint32_t *)0x42981408)
#define PORT_PIN8_ISFR  (*(volatile uint32_t *)0x4298140c)
#define PORT_PIN9_ISFR  (*(volatile uint32_t *)0x4296140c)
#define PORT_PIN10_ISFR  (*(volatile uint32_t *)0x42961410)
#define PORT_PIN11_ISFR  (*(volatile uint32_t *)0x42961418)
#define PORT_PIN12_ISFR  (*(volatile uint32_t *)0x4296141c)
#define PORT_PIN13_ISFR  (*(volatile uint32_t *)0x42961414)
#define PORT_PIN14_ISFR  (*(volatile uint32_t *)0x42981404)
#define PORT_PIN15_ISFR  (*(volatile uint32_t *)0x42961400)
#define PORT_PIN16_ISFR  (*(volatile uint32_t *)0x42941400)
#define PORT_PIN17_ISFR  (*(volatile uint32_t *)0x42941404)
#define PORT_PIN18_ISFR  (*(volatile uint32_t *)0x4294140c)
#define PORT_PIN19_ISFR  (*(volatile uint32_t *)0x42941408)
#define PORT_PIN20_ISFR  (*(volatile uint32_t *)0x42981414)
#define PORT_PIN21_ISFR  (*(volatile uint32_t *)0x42981418)
#define PORT_PIN22_ISFR  (*(volatile uint32_t *)0x42961404)
#define PORT_PIN23_ISFR  (*(volatile uint32_t *)0x42961408)

typedef void (*isr_t)(void);

typedef enum
{
    IRQC_LOW     = 0x08,
    IRQC_RISING  = 0x09,
    IRQC_FALLING = 0x0A,
    IRQC_CHANGE  = 0x0B,
    IRQC_HIGH    = 0x0C

} irqc_t;

namespace INTR
{
    void init(void);
    bool attach(uint8_t pin, isr_t isr, irqc_t irqc);
    bool detach(uint8_t pin);
    void suspend(void);
    void resume(void);
    void clear(void);

    inline uint8_t check(uint8_t pin) __attribute__((always_inline));
    inline void clear(uint8_t pin) __attribute__((always_inline));

    volatile uint32_t* const isfr_bitband_alias[] = {
        &PORT_PIN0_ISFR,
        &PORT_PIN1_ISFR,
        &PORT_PIN2_ISFR,
        &PORT_PIN3_ISFR,
        &PORT_PIN4_ISFR,
        &PORT_PIN5_ISFR,
        &PORT_PIN6_ISFR,
        &PORT_PIN7_ISFR,
        &PORT_PIN8_ISFR,
        &PORT_PIN9_ISFR,
        &PORT_PIN10_ISFR,
        &PORT_PIN11_ISFR,
        &PORT_PIN12_ISFR,
        &PORT_PIN13_ISFR,
        &PORT_PIN14_ISFR,
        &PORT_PIN15_ISFR,
        &PORT_PIN16_ISFR,
        &PORT_PIN17_ISFR,
        &PORT_PIN18_ISFR,
        &PORT_PIN19_ISFR,
        &PORT_PIN20_ISFR,
        &PORT_PIN21_ISFR,
        &PORT_PIN22_ISFR,
        &PORT_PIN23_ISFR
    };

    inline uint8_t check(uint8_t pin)
    {
        if (__builtin_constant_p(pin)) {
            if (pin == 0) { return (uint8_t)PORT_PIN0_ISFR; }
            else if (pin == 1) { return (uint8_t)PORT_PIN1_ISFR; }
            else if (pin == 2) { return (uint8_t)PORT_PIN2_ISFR; }
            else if (pin == 3) { return (uint8_t)PORT_PIN3_ISFR; }
            else if (pin == 4) { return (uint8_t)PORT_PIN4_ISFR; }
            else if (pin == 5) { return (uint8_t)PORT_PIN5_ISFR; }
            else if (pin == 6) { return (uint8_t)PORT_PIN6_ISFR; }
            else if (pin == 7) { return (uint8_t)PORT_PIN7_ISFR; }
            else if (pin == 8) { return (uint8_t)PORT_PIN8_ISFR; }
            else if (pin == 9) { return (uint8_t)PORT_PIN9_ISFR; }
            else if (pin == 10) { return (uint8_t)PORT_PIN10_ISFR; }
            else if (pin == 11) { return (uint8_t)PORT_PIN11_ISFR; }
            else if (pin == 12) { return (uint8_t)PORT_PIN12_ISFR; }
            else if (pin == 13) { return (uint8_t)PORT_PIN13_ISFR; }
            else if (pin == 14) { return (uint8_t)PORT_PIN14_ISFR; }
            else if (pin == 15) { return (uint8_t)PORT_PIN15_ISFR; }
            else if (pin == 16) { return (uint8_t)PORT_PIN16_ISFR; }
            else if (pin == 17) { return (uint8_t)PORT_PIN17_ISFR; }
            else if (pin == 18) { return (uint8_t)PORT_PIN18_ISFR; }
            else if (pin == 19) { return (uint8_t)PORT_PIN19_ISFR; }
            else if (pin == 20) { return (uint8_t)PORT_PIN20_ISFR; }
            else if (pin == 21) { return (uint8_t)PORT_PIN21_ISFR; }
            else if (pin == 22) { return (uint8_t)PORT_PIN22_ISFR; }
            else if (pin == 23) { return (uint8_t)PORT_PIN23_ISFR; }
            else { return 0; }
        } else {
            return *isfr_bitband_alias[pin];
        }
    }

    inline void clear(uint8_t pin)
    {
        if (__builtin_constant_p(pin)) {
            if (pin == 0) { PORT_PIN0_ISFR = 1; }
            else if (pin == 1) { PORT_PIN1_ISFR = 1; }
            else if (pin == 2) { PORT_PIN2_ISFR = 1; }
            else if (pin == 3) { PORT_PIN3_ISFR = 1; }
            else if (pin == 4) { PORT_PIN4_ISFR = 1; }
            else if (pin == 5) { PORT_PIN5_ISFR = 1; }
            else if (pin == 6) { PORT_PIN6_ISFR = 1; }
            else if (pin == 7) { PORT_PIN7_ISFR = 1; }
            else if (pin == 8) { PORT_PIN8_ISFR = 1; }
            else if (pin == 9) { PORT_PIN9_ISFR = 1; }
            else if (pin == 10) { PORT_PIN10_ISFR = 1; }
            else if (pin == 11) { PORT_PIN11_ISFR = 1; }
            else if (pin == 12) { PORT_PIN12_ISFR = 1; }
            else if (pin == 13) { PORT_PIN13_ISFR = 1; }
            else if (pin == 14) { PORT_PIN14_ISFR = 1; }
            else if (pin == 15) { PORT_PIN15_ISFR = 1; }
            else if (pin == 16) { PORT_PIN16_ISFR = 1; }
            else if (pin == 17) { PORT_PIN17_ISFR = 1; }
            else if (pin == 18) { PORT_PIN18_ISFR = 1; }
            else if (pin == 19) { PORT_PIN19_ISFR = 1; }
            else if (pin == 20) { PORT_PIN20_ISFR = 1; }
            else if (pin == 21) { PORT_PIN21_ISFR = 1; }
            else if (pin == 22) { PORT_PIN22_ISFR = 1; }
            else if (pin == 23) { PORT_PIN23_ISFR = 1; }
        } else {
            *isfr_bitband_alias[pin] = 1;
        }
    }
};

#endif
