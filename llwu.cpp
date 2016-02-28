#include "deep_sleep.h"
#include "pins.h"
#include <kinetis.h>
#include <cstdint>

#define SCB_SCR_SLEEPDEEP        0x00000004
#define SMC_PMCTRL_STOPM_LLS     SMC_PMCTRL_STOPM(0x03)
#define SIM_SOPT1_OSC32KSEL_LPO  SIM_SOPT1_OSC32KSEL(0x03)

#define LPTMR_PCS_MCGIRCLK   0x00   // Internal reference clock
#define LPTMR_PCS_LPO_1KHZ   0x01   // LPO - 1kHz clock
#define LPTMR_PCS_ERCLK32K   0x02   // Secondary external reference clock
#define LPTMR_PCS_OSCERCLK   0x03   // External reference clock

#define LLWU_WUPE0_SET(wupe)   (LLWU_PE1 |= (wupe << 0))
#define LLWU_WUPE1_SET(wupe)   (LLWU_PE1 |= (wupe << 2))
#define LLWU_WUPE2_SET(wupe)   (LLWU_PE1 |= (wupe << 4))
#define LLWU_WUPE3_SET(wupe)   (LLWU_PE1 |= (wupe << 6))

#define LLWU_WUPE4_SET(wupe)   (LLWU_PE2 |= (wupe << 0))
#define LLWU_WUPE5_SET(wupe)   (LLWU_PE2 |= (wupe << 2))
#define LLWU_WUPE6_SET(wupe)   (LLWU_PE2 |= (wupe << 4))
#define LLWU_WUPE7_SET(wupe)   (LLWU_PE2 |= (wupe << 6))

#define LLWU_WUPE8_SET(wupe)   (LLWU_PE3 |= (wupe << 0))
#define LLWU_WUPE9_SET(wupe)   (LLWU_PE3 |= (wupe << 2))
#define LLWU_WUPE10_SET(wupe)  (LLWU_PE3 |= (wupe << 4))
#define LLWU_WUPE11_SET(wupe)  (LLWU_PE3 |= (wupe << 6))

#define LLWU_WUPE12_SET(wupe)  (LLWU_PE4 |= (wupe << 0))
#define LLWU_WUPE13_SET(wupe)  (LLWU_PE4 |= (wupe << 2))
#define LLWU_WUPE14_SET(wupe)  (LLWU_PE4 |= (wupe << 4))
#define LLWU_WUPE15_SET(wupe)  (LLWU_PE4 |= (wupe << 6))

#define LLWU_WUPE0_GET   ((LLWU_PE1 | (0x03 << 0)) >> 0)
#define LLWU_WUPE1_GET   ((LLWU_PE1 | (0x03 << 2)) >> 2)
#define LLWU_WUPE2_GET   ((LLWU_PE1 | (0x03 << 4)) >> 4)
#define LLWU_WUPE3_GET   ((LLWU_PE1 | (0x03 << 6)) >> 6)

#define LLWU_WUPE4_GET   ((LLWU_PE2 | (0x03 << 0)) >> 0)
#define LLWU_WUPE5_GET   ((LLWU_PE2 | (0x03 << 2)) >> 2)
#define LLWU_WUPE6_GET   ((LLWU_PE2 | (0x03 << 4)) >> 4)
#define LLWU_WUPE7_GET   ((LLWU_PE2 | (0x03 << 6)) >> 6)

#define LLWU_WUPE8_GET   ((LLWU_PE3 | (0x03 << 0)) >> 0)
#define LLWU_WUPE9_GET   ((LLWU_PE3 | (0x03 << 2)) >> 2)
#define LLWU_WUPE10_GET  ((LLWU_PE3 | (0x03 << 4)) >> 4)
#define LLWU_WUPE11_GET  ((LLWU_PE3 | (0x03 << 6)) >> 6)

#define LLWU_WUPE12_GET  ((LLWU_PE4 | (0x03 << 0)) >> 0)
#define LLWU_WUPE13_GET  ((LLWU_PE4 | (0x03 << 2)) >> 2)
#define LLWU_WUPE14_GET  ((LLWU_PE4 | (0x03 << 4)) >> 4)
#define LLWU_WUPE15_GET  ((LLWU_PE4 | (0x03 << 6)) >> 6)

#define LLWU_WUPE0_CLEAR   (LLWU_PE1 &= ~(0x03 << 0))
#define LLWU_WUPE1_CLEAR   (LLWU_PE1 &= ~(0x03 << 2))
#define LLWU_WUPE2_CLEAR   (LLWU_PE1 &= ~(0x03 << 4))
#define LLWU_WUPE3_CLEAR   (LLWU_PE1 &= ~(0x03 << 6))

#define LLWU_WUPE4_CLEAR   (LLWU_PE2 &= ~(0x03 << 0))
#define LLWU_WUPE5_CLEAR   (LLWU_PE2 &= ~(0x03 << 2))
#define LLWU_WUPE6_CLEAR   (LLWU_PE2 &= ~(0x03 << 4))
#define LLWU_WUPE7_CLEAR   (LLWU_PE2 &= ~(0x03 << 6))

#define LLWU_WUPE8_CLEAR   (LLWU_PE3 &= ~(0x03 << 0))
#define LLWU_WUPE9_CLEAR   (LLWU_PE3 &= ~(0x03 << 2))
#define LLWU_WUPE10_CLEAR  (LLWU_PE3 &= ~(0x03 << 4))
#define LLWU_WUPE11_CLEAR  (LLWU_PE3 &= ~(0x03 << 6))

#define LLWU_WUPE12_CLEAR  (LLWU_PE4 &= ~(0x03 << 0))
#define LLWU_WUPE13_CLEAR  (LLWU_PE4 &= ~(0x03 << 2))
#define LLWU_WUPE14_CLEAR  (LLWU_PE4 &= ~(0x03 << 4))
#define LLWU_WUPE15_CLEAR  (LLWU_PE4 &= ~(0x03 << 6))

#define LLWU_WUF0_CHECK    (LLWU_F1 & 0x01)
#define LLWU_WUF1_CHECK    (LLWU_F1 & 0x02)
#define LLWU_WUF2_CHECK    (LLWU_F1 & 0x04)
#define LLWU_WUF3_CHECK    (LLWU_F1 & 0x08)
#define LLWU_WUF4_CHECK    (LLWU_F1 & 0x10)
#define LLWU_WUF5_CHECK    (LLWU_F1 & 0x20)
#define LLWU_WUF6_CHECK    (LLWU_F1 & 0x40)
#define LLWU_WUF7_CHECK    (LLWU_F1 & 0x80)

#define LLWU_WUF8_CHECK    (LLWU_F2 & 0x01)
#define LLWU_WUF9_CHECK    (LLWU_F2 & 0x02)
#define LLWU_WUF10_CHECK   (LLWU_F2 & 0x04)
#define LLWU_WUF11_CHECK   (LLWU_F2 & 0x08)
#define LLWU_WUF12_CHECK   (LLWU_F2 & 0x10)
#define LLWU_WUF13_CHECK   (LLWU_F2 & 0x20)
#define LLWU_WUF14_CHECK   (LLWU_F2 & 0x40)
#define LLWU_WUF15_CHECK   (LLWU_F2 & 0x80)

#define LLWU_WUF0_CLEAR    (LLWU_F1 |= 0x01)
#define LLWU_WUF1_CLEAR    (LLWU_F1 |= 0x02)
#define LLWU_WUF2_CLEAR    (LLWU_F1 |= 0x04)
#define LLWU_WUF3_CLEAR    (LLWU_F1 |= 0x08)
#define LLWU_WUF4_CLEAR    (LLWU_F1 |= 0x10)
#define LLWU_WUF5_CLEAR    (LLWU_F1 |= 0x20)
#define LLWU_WUF6_CLEAR    (LLWU_F1 |= 0x40)
#define LLWU_WUF7_CLEAR    (LLWU_F1 |= 0x80)

#define LLWU_WUF8_CLEAR    (LLWU_F2 |= 0x01)
#define LLWU_WUF9_CLEAR    (LLWU_F2 |= 0x02)
#define LLWU_WUF10_CLEAR   (LLWU_F2 |= 0x04)
#define LLWU_WUF11_CLEAR   (LLWU_F2 |= 0x08)
#define LLWU_WUF12_CLEAR   (LLWU_F2 |= 0x10)
#define LLWU_WUF13_CLEAR   (LLWU_F2 |= 0x20)
#define LLWU_WUF14_CLEAR   (LLWU_F2 |= 0x40)
#define LLWU_WUF15_CLEAR   (LLWU_F2 |= 0x80)

#define PIN2_WUPE_SET(wupe)    LLWU_WUPE12_SET(wupe)
#define PIN4_WUPE_SET(wupe)    LLWU_WUPE4_SET(wupe)
#define PIN6_WUPE_SET(wupe)    LLWU_WUPE14_SET(wupe)
#define PIN7_WUPE_SET(wupe)    LLWU_WUPE13_SET(wupe)
#define PIN9_WUPE_SET(wupe)    LLWU_WUPE7_SET(wupe)
#define PIN10_WUPE_SET(wupe)   LLWU_WUPE8_SET(wupe)
#define PIN11_WUPE_SET(wupe)   LLWU_WUPE10_SET(wupe)
#define PIN13_WUPE_SET(wupe)   LLWU_WUPE9_SET(wupe)
#define PIN16_WUPE_SET(wupe)   LLWU_WUPE5_SET(wupe)
#define PIN21_WUPE_SET(wupe)   LLWU_WUPE15_SET(wupe)
#define PIN22_WUPE_SET(wupe)   LLWU_WUPE6_SET(wupe)

#define PIN_WUPE_SET(pin, wupe)  PIN ## pin ## _WUPE_SET(wupe)

#define PIN2_WUPE_GET    LLWU_WUPE12_GET
#define PIN4_WUPE_GET    LLWU_WUPE4_GET
#define PIN6_WUPE_GET    LLWU_WUPE14_GET
#define PIN7_WUPE_GET    LLWU_WUPE13_GET
#define PIN9_WUPE_GET    LLWU_WUPE7_GET
#define PIN10_WUPE_GET   LLWU_WUPE8_GET
#define PIN11_WUPE_GET   LLWU_WUPE10_GET
#define PIN13_WUPE_GET   LLWU_WUPE9_GET
#define PIN16_WUPE_GET   LLWU_WUPE5_GET
#define PIN21_WUPE_GET   LLWU_WUPE15_GET
#define PIN22_WUPE_GET   LLWU_WUPE6_GET

#define PIN_WUPE_GET(pin)  PIN ## pin ## _WUPE_GET

#define PIN2_WUPE_CLEAR    LLWU_WUPE12_CLEAR
#define PIN4_WUPE_CLEAR    LLWU_WUPE4_CLEAR
#define PIN6_WUPE_CLEAR    LLWU_WUPE14_CLEAR
#define PIN7_WUPE_CLEAR    LLWU_WUPE13_CLEAR
#define PIN9_WUPE_CLEAR    LLWU_WUPE7_CLEAR
#define PIN10_WUPE_CLEAR   LLWU_WUPE8_CLEAR
#define PIN11_WUPE_CLEAR   LLWU_WUPE10_CLEAR
#define PIN13_WUPE_CLEAR   LLWU_WUPE9_CLEAR
#define PIN16_WUPE_CLEAR   LLWU_WUPE5_CLEAR
#define PIN21_WUPE_CLEAR   LLWU_WUPE15_CLEAR
#define PIN22_WUPE_CLEAR   LLWU_WUPE6_CLEAR

#define PIN_WUPE_CLEAR(pin)  PIN ## pin ## _WUPE_CLEAR

#define PIN2_WUF_CHECK   LLWU_WUF12_CHECK
#define PIN4_WUF_CHECK   LLWU_WUF4_CHECK
#define PIN6_WUF_CHECK   LLWU_WUF14_CHECK
#define PIN7_WUF_CHECK   LLWU_WUF13_CHECK
#define PIN9_WUF_CHECK   LLWU_WUF7_CHECK
#define PIN10_WUF_CHECK  LLWU_WUF8_CHECK
#define PIN11_WUF_CHECK  LLWU_WUF10_CHECK
#define PIN13_WUF_CHECK  LLWU_WUF9_CHECK
#define PIN16_WUF_CHECK  LLWU_WUF5_CHECK
#define PIN21_WUF_CHECK  LLWU_WUF15_CHECK
#define PIN22_WUF_CHECK  LLWU_WUF6_CHECK

#define PIN_WUF_CHECK(pin)   PIN ## pin ## _WUF_CHECK

#define PIN2_WUF_CLEAR   LLWU_WUF12_CLEAR
#define PIN4_WUF_CLEAR   LLWU_WUF4_CLEAR
#define PIN6_WUF_CLEAR   LLWU_WUF14_CLEAR
#define PIN7_WUF_CLEAR   LLWU_WUF13_CLEAR
#define PIN9_WUF_CLEAR   LLWU_WUF7_CLEAR
#define PIN10_WUF_CLEAR  LLWU_WUF8_CLEAR
#define PIN11_WUF_CLEAR  LLWU_WUF10_CLEAR
#define PIN13_WUF_CLEAR  LLWU_WUF9_CLEAR
#define PIN16_WUF_CLEAR  LLWU_WUF5_CLEAR
#define PIN21_WUF_CLEAR  LLWU_WUF15_CLEAR
#define PIN22_WUF_CLEAR  LLWU_WUF6_CLEAR

#define PIN_WUF_CLEAR(pin)   PIN ## pin ## _WUF_CLEAR

#define LLWU_WUME0_SET   (LLWU_ME |= 0x01)
#define LLWU_WUME1_SET   (LLWU_ME |= 0x02)
#define LLWU_WUME2_SET   (LLWU_ME |= 0x04)
#define LLWU_WUME3_SET   (LLWU_ME |= 0x08)
#define LLWU_WUME4_SET   (LLWU_ME |= 0x10)
#define LLWU_WUME5_SET   (LLWU_ME |= 0x20)
#define LLWU_WUME6_SET   (LLWU_ME |= 0x40)
#define LLWU_WUME7_SET   (LLWU_ME |= 0x80)

#define LLWU_WUME0_GET   (LLWU_ME | 0x01)
#define LLWU_WUME1_GET   (LLWU_ME | 0x02)
#define LLWU_WUME2_GET   (LLWU_ME | 0x04)
#define LLWU_WUME3_GET   (LLWU_ME | 0x08)
#define LLWU_WUME4_GET   (LLWU_ME | 0x10)
#define LLWU_WUME5_GET   (LLWU_ME | 0x20)
#define LLWU_WUME6_GET   (LLWU_ME | 0x40)
#define LLWU_WUME7_GET   (LLWU_ME | 0x80)

#define LLWU_WUME0_CLEAR   (LLWU_ME &= ~0x01)
#define LLWU_WUME1_CLEAR   (LLWU_ME &= ~0x02)
#define LLWU_WUME2_CLEAR   (LLWU_ME &= ~0x04)
#define LLWU_WUME3_CLEAR   (LLWU_ME &= ~0x08)
#define LLWU_WUME4_CLEAR   (LLWU_ME &= ~0x10)
#define LLWU_WUME5_CLEAR   (LLWU_ME &= ~0x20)
#define LLWU_WUME6_CLEAR   (LLWU_ME &= ~0x40)
#define LLWU_WUME7_CLEAR   (LLWU_ME &= ~0x80)

#define LLWU_MWUF0_CHECK    (LLWU_F3 & 0x01)
#define LLWU_MWUF1_CHECK    (LLWU_F3 & 0x02)
#define LLWU_MWUF2_CHECK    (LLWU_F3 & 0x04)
#define LLWU_MWUF3_CHECK    (LLWU_F3 & 0x08)
#define LLWU_MWUF4_CHECK    (LLWU_F3 & 0x10)
#define LLWU_MWUF5_CHECK    (LLWU_F3 & 0x20)
#define LLWU_MWUF6_CHECK    (LLWU_F3 & 0x40)
#define LLWU_MWUF7_CHECK    (LLWU_F3 & 0x80)

#define LLWU_WUME_SET(wume)     LLWU_WUME ## wume ## _SET
#define LLWU_WUME_GET(wume)     LLWU_WUME ## wume ## _GET
#define LLWU_WUME_CLEAR(wume)   LLWU_WUME ## wume ## _CLEAR
#define LLWU_MWUF_CHECK(wume)   LLWU_MWUF ## wume ## _CHECK

#define LPTMR_WUME_SET     LLWU_WUME_SET(0)
#define LPTMR_WUME_GET     LLWU_WUME_GET(0)
#define LPTMR_WUME_CLEAR   LLWU_WUME_CLEAR(0)
#define LPTMR_MWUF_CHECK   LLWU_WUME_CHECK(0)

#define PIN_GPIO(pin)     PIN ## pin ## _GPIO
#define GPIO_INPUT(pin)  (GPIO::ddir(pin) == 0)

void wakeup_isr(void);
//void lptmr_isr(void);

namespace LLWU
{
    const uint32_t valid_pin_mask =
          (1 << 2)
        | (1 << 4)
        | (1 << 6)
        | (1 << 7)
        | (1 << 9)
        | (1 << 10)
        | (1 << 11)
        | (1 << 13)
        | (1 << 16)
        | (1 << 21)
        | (1 << 22);

    uint32_t pin_mask = 0;
    uint32_t stop_mode = (uint32_t)SMC_PMCTRL_STOPM_LLS;
    uint32_t osc32ksel = (uint32_t)SIM_SOPT1_OSC32KSEL_LPO;
    bool enabled = false;
    bool lptmr_enabled = false;
    volatile wus_t wakeup_source = WUS_NONE;

    inline bool validPin(uint8_t pin)
    {
        if (pin >= 32)
            return false;

        return (valid_pin_mask & (1 << pin)) ? true : false;
    }

    inline bool pinEnabled(uint8_t pin)
    {
        if (!validPin(pin))
            return false;

        return (LLWU::pin_mask & (1 << pin)) ? true : false;
    }

    inline void stopMode(void)
    {
        uint32_t sm = stop_mode;
        stop_mode = SMC_PMCTRL & SMC_PMCTRL_STOPM(0x07);
        SMC_PMCTRL &= ~stop_mode;
        SMC_PMCTRL |= sm;
    }

    inline void osc32ClockSelect(void)
    {
        uint32_t osc = osc32ksel;
        osc32ksel = SIM_SOPT1 & SIM_SOPT1_OSC32KSEL(0x03);
        SIM_SOPT1 &= ~osc32ksel;
        SIM_SOPT1 |= osc;
    }

    inline void clearWakeupFlags(void)
    {
        LLWU_F1 = 0xFF; LLWU_F2 = 0xFF; // LLWU_F3 = 0xFF;  F3 flags are read-only
    }
};

void LLWU::enable(void)
{
    if (LLWU::enabled)
        return;

    NVIC_SET_PRIORITY(IRQ_LLWU, 64);
    NVIC_ENABLE_IRQ(IRQ_LLWU);
    SCB_SCR |= SCB_SCR_SLEEPDEEP;
    LLWU::stopMode();

    LLWU::enabled = true;
}

void LLWU::disable(void)
{
    if (!LLWU::enabled)
        return;

    LLWU::stopMode();
    SCB_SCR &= ~SCB_SCR_SLEEPDEEP;
    NVIC_DISABLE_IRQ(IRQ_LLWU);
    NVIC_SET_PRIORITY(IRQ_LLWU, 128);

    // Disable pins
    while (LLWU::pin_mask)
        (void)LLWU::wakeupPinDisable(__builtin_ctz(LLWU::pin_mask));

    if (LLWU::lptmr_enabled)
        (void)LLWU::wakeupLPTMRDisable();

    LLWU::enabled = false;
}

bool LLWU::wakeupPinEnable(uint8_t pin, wupe_t wakeup_pin_edge)
{
    if (!LLWU::enabled || !LLWU::validPin(pin))
        return false;

    switch (pin)
    {
        case 2:
            // Pin must be configured for GPIO and as an INPUT
            if (!PIN_GPIO(2) || !GPIO_INPUT(2))
                return false;
            // Enable pin wake up for rising, falling or change
            PIN_WUPE_SET(2, wakeup_pin_edge);
            break;
        case 4:
            if (!PIN_GPIO(4) || !GPIO_INPUT(4))
                return false;
            PIN_WUPE_SET(4, wakeup_pin_edge);
            break;
        case 6:
            if (!PIN_GPIO(6) || !GPIO_INPUT(6))
                return false;
            PIN_WUPE_SET(6, wakeup_pin_edge);
            break;
        case 7:
            if (!PIN_GPIO(7) || !GPIO_INPUT(7))
                return false;
            PIN_WUPE_SET(7, wakeup_pin_edge);
            break;
        case 9:
            if (!PIN_GPIO(9) || !GPIO_INPUT(9))
                return false;
            PIN_WUPE_SET(9, wakeup_pin_edge);
            break;
        case 10:
            if (!PIN_GPIO(10) || !GPIO_INPUT(10))
                return false;
            PIN_WUPE_SET(10, wakeup_pin_edge);
            break;
        case 11:
            if (!PIN_GPIO(11) || !GPIO_INPUT(11))
                return false;
            PIN_WUPE_SET(11, wakeup_pin_edge);
            break;
        case 13:
            if (!PIN_GPIO(13) || !GPIO_INPUT(13))
                return false;
            PIN_WUPE_SET(13, wakeup_pin_edge);
            break;
        case 16:
            if (!PIN_GPIO(16) || !GPIO_INPUT(16))
                return false;
            PIN_WUPE_SET(16, wakeup_pin_edge);
            break;
        case 21:
            if (!PIN_GPIO(21) || !GPIO_INPUT(21))
                return false;
            PIN_WUPE_SET(21, wakeup_pin_edge);
            break;
        case 22:
            if (!PIN_GPIO(22) || !GPIO_INPUT(22))
                return false;
            PIN_WUPE_SET(22, wakeup_pin_edge);
            break;
        default:  // Shouldn't get here due to initial check
            return false;
    }

    LLWU::pin_mask |= (1 << pin);

    return true;
}

bool LLWU::wakeupPinDisable(uint8_t pin)
{
    if (!LLWU::enabled || !LLWU::pinEnabled(pin))
        return false;

    switch (pin)
    {
        case 2:
            PIN_WUPE_CLEAR(2);
            break;
        case 4:
            PIN_WUPE_CLEAR(4);
            break;
        case 6:
            PIN_WUPE_CLEAR(6);
            break;
        case 7:
            PIN_WUPE_CLEAR(7);
            break;
        case 9:
            PIN_WUPE_CLEAR(9);
            break;
        case 10:
            PIN_WUPE_CLEAR(10);
            break;
        case 11:
            PIN_WUPE_CLEAR(11);
            break;
        case 13:
            PIN_WUPE_CLEAR(13);
            break;
        case 16:
            PIN_WUPE_CLEAR(16);
            break;
        case 21:
            PIN_WUPE_CLEAR(21);
            break;
        case 22:
            PIN_WUPE_CLEAR(22);
            break;
        default:  // Shouldn't get here due to initial check
            return false;
    }

    LLWU::pin_mask &= ~(1 << pin);

    return true;
}

bool LLWU::wakeupLPTMREnable(uint16_t msecs)
{
    if (!LLWU::enabled || LLWU::lptmr_enabled)
        return false;

    LLWU::osc32ClockSelect();
    SIM_SCGC5 |= SIM_SCGC5_LPTIMER;

    LPTMR0_PSR = LPTMR_PSR_PBYP | LPTMR_PSR_PCS(LPTMR_PCS_LPO_1KHZ);
    LPTMR0_CMR = (uint32_t)msecs;  // Register is 32-bit but only first 16 bits are used
    LPTMR_WUME_SET;

    NVIC_ENABLE_IRQ(IRQ_LPTMR);

    LLWU::lptmr_enabled = true;

    return true;
}

bool LLWU::wakeupLPTMRDisable(void)
{
    if (!LLWU::enabled || !LLWU::lptmr_enabled)
        return false;

    NVIC_DISABLE_IRQ(IRQ_LPTMR);

    LPTMR_WUME_CLEAR;
    LPTMR0_CMR = 0;
    LPTMR0_PSR = 0;

    SIM_SCGC5 &= ~SIM_SCGC5_LPTIMER;
    LLWU::osc32ClockSelect();

    LLWU::lptmr_enabled = false;

    return true;
}

wus_t LLWU::wakeupSource(void)
{
    return LLWU::wakeup_source;
}

wus_t LLWU::sleep(void)
{
    // If module hasn't been enabled return
    if (!LLWU::enabled)
        return WUS_ERROR;

    // If there is nothing that can wake the CPU up, return
    if (!LLWU::lptmr_enabled && !LLWU::pin_mask)
        return WUS_ERROR;

    LLWU::wakeup_source = WUS_NONE;

    if (LLWU::lptmr_enabled)
        LPTMR0_CSR = (LPTMR_CSR_TEN | LPTMR_CSR_TCF | LPTMR_CSR_TIE);

    // XXX This may not be necessary since they're cleared in wakeup_isr
    LLWU::clearWakeupFlags();

    // Freescale documentation says to do this:
    // http://cache.nxp.com/files/32bit/doc/app_note/AN4503.pdf
    // Disables the loss of clock monitoring circuit for the OSC0
    // in case it is configured to generate an interrupt or reset
    // since clocks will stop when the wfi instruction is executed.
    MCG_C6 &= ~MCG_C6_CME0;

    // XXX Is this necessary?
    // The Teensy Snooze code does this, but in testing it doesn't seem to be needed.
    //SYST_CSR &= ~SYST_CSR_TICKINT;

    // Wait For Interrupt instruction
    // Puts the MCU into DEEPSLEEP / LLS mode
    asm volatile("wfi");

    //SYST_CSR |= SYST_CSR_TICKINT;

    MCG_C6 |= MCG_C6_CME0;

    if (LLWU::lptmr_enabled)
        LPTMR0_CSR = 0;

    return LLWU::wakeup_source;
}

#if 0
// The prototype for this is defined as "unused".  The vector is already set in
// the NVIC in the Teensy initialization code so just need to define it.
// If the LPTMR_CSR_TCF flag is cleared and the pending interrupt is cleared
// in the wakeup_isr then this won't be called.
void lptmr_isr(void)
{
    LPTMR0_CSR |= LPTMR_CSR_TCF;
}
#endif

// The prototype for this is defined as "unused".  The vector is already set in
// the NVIC in the Teensy initialization code so just need to define it.
void wakeup_isr(void)
{
    // If there is a pending interrupt with the LPTMR then clear the flag
    // and pending interrupt in that order.  If the order is reversed then
    // after clearing the pending interrupt, another interrupt is immediately
    // generated since the LPTMR_CSR_TCF flag hasn't been cleared yet.
    if (NVIC_IS_PENDING(IRQ_LPTMR))
    {
        LPTMR0_CSR |= LPTMR_CSR_TCF;
        NVIC_CLEAR_PENDING(IRQ_LPTMR);

        LLWU::wakeup_source = WUS_TIMER;
    }
    else
    {
        LLWU::wakeup_source = WUS_PIN;
    }

    // XXX Could use pin_mask to check which pin caused the interrupt
    // if necessary.
    LLWU::clearWakeupFlags();

    // XXX Use pin_mask to clear PORT interrupts
    // This doesn't seem to be necessary.  Only this ISR is called and
    // not the ones configured in the PORT_PCR registers.

    // Should wake up into PBE mode - need to go back into PEE mode
    // Code basically taken from Freescale: K20 72MHz Bare Metal Examples
    if (((MCG_S & MCG_S_CLKST_MASK) == MCG_S_CLKST(0x02))  // check CLKS mux has selcted external reference
            && (!(MCG_S & MCG_S_IREFST))  // check FLL ref is external ref clk
            && (MCG_S & MCG_S_PLLST)      // check PLLS mux has selected PLL
            && (!(MCG_C2 & MCG_C2_LP)))   // check Low Power bit not set
    {
        // This lock check may not be necessary
        while (!(MCG_S & MCG_S_LOCK0));
        MCG_C1 &= ~MCG_C1_CLKS(0x03);
        while ((MCG_S & MCG_S_CLKST_MASK) != MCG_S_CLKST(0x03));
    }
}
