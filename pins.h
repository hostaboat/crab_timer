#ifndef _PINS_H_
#define _PINS_H_

#include <cstdint>

#define PIN_ROT_SWI_A    0   // Rotary Switch A
#define PIN_ROT_SWI_B    1   // Rotary Switch B
#define PIN_ROT_SWI_C    2   // Rotary Switch C
#define PIN_AUDIO_PREV   3   // Play previous audio file buttion
#define PIN_AUDIO_PLAY   4   // Play / Pause button
#define PIN_AUDIO_NEXT   5   // Play next audio file button
#define PIN_NEOPIXEL     6   // Neopixel strip
#define PIN_ROT_ENC_BR_A 7   // Rotary Encoder A - Brightness
#define PIN_ROT_ENC_BR_B 8   // Rotary Encoder B - Brightness
#define PIN_AMP_SDWN     9   // Amplifier Shutdown
#define PIN_AUDIO_DREQ  10   // VS1053 Data Request
#define PIN_MOSI        11   // SPI - MOSI / DOUT
#define PIN_MISO        12   // SPI - MISO / DIN
#define PIN_AUDIO_RST   13   // VS1053 Hardware Reset
#define PIN_SCK         14   // SPI - SCK
#define PIN_AUDIO_CS    15   // SPI - VS1053 Command Chip Select (SCI)
#define PIN_SD_CS       16   // SPI - SD Chip Select
#define PIN_AUDIO_DCS   17   // SPI - VS1053 Data Chip Select (SDI)
#define PIN_SDA         18   // I2C - SDA  (7-Segment Display)
#define PIN_SCL         19   // I2C - SCL  (7-Segment Display)
#define PIN_BEEPER      20   // Beeper
#define PIN_ROT_ENC_SW  21   // Rotary Encoder Switch
#define PIN_ROT_ENC_A   22   // Rotary Encoder A
#define PIN_ROT_ENC_B   23   // Rotary Encoder B

#define PIN_MAX   24

#define PIN0_PORT_REG    (*(volatile uint32_t *)0x4004A040)
#define PIN1_PORT_REG    (*(volatile uint32_t *)0x4004A044)
#define PIN2_PORT_REG    (*(volatile uint32_t *)0x4004C000)
#define PIN3_PORT_REG    (*(volatile uint32_t *)0x40049030)
#define PIN4_PORT_REG    (*(volatile uint32_t *)0x40049034)
#define PIN5_PORT_REG    (*(volatile uint32_t *)0x4004C01C)
#define PIN6_PORT_REG    (*(volatile uint32_t *)0x4004C010)
#define PIN7_PORT_REG    (*(volatile uint32_t *)0x4004C008)
#define PIN8_PORT_REG    (*(volatile uint32_t *)0x4004C00C)
#define PIN9_PORT_REG    (*(volatile uint32_t *)0x4004B00C)
#define PIN10_PORT_REG   (*(volatile uint32_t *)0x4004B010)
#define PIN11_PORT_REG   (*(volatile uint32_t *)0x4004B018)
#define PIN12_PORT_REG   (*(volatile uint32_t *)0x4004B01C)
#define PIN13_PORT_REG   (*(volatile uint32_t *)0x4004B014)
#define PIN14_PORT_REG   (*(volatile uint32_t *)0x4004C004)
#define PIN15_PORT_REG   (*(volatile uint32_t *)0x4004B000)
#define PIN16_PORT_REG   (*(volatile uint32_t *)0x4004A000)
#define PIN17_PORT_REG   (*(volatile uint32_t *)0x4004A004)
#define PIN18_PORT_REG   (*(volatile uint32_t *)0x4004A00C)
#define PIN19_PORT_REG   (*(volatile uint32_t *)0x4004A008)
#define PIN20_PORT_REG   (*(volatile uint32_t *)0x4004C014)
#define PIN21_PORT_REG   (*(volatile uint32_t *)0x4004C018)
#define PIN22_PORT_REG   (*(volatile uint32_t *)0x4004B004)
#define PIN23_PORT_REG   (*(volatile uint32_t *)0x4004B008)

#define PIN_PORT_REG(pin)   PIN ## pin ## _PORT_REG

#define PIN0_GPIO   (((PIN0_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN1_GPIO   (((PIN1_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN2_GPIO   (((PIN2_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN3_GPIO   (((PIN3_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN4_GPIO   (((PIN4_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN5_GPIO   (((PIN5_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN6_GPIO   (((PIN6_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN7_GPIO   (((PIN7_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN8_GPIO   (((PIN8_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN9_GPIO   (((PIN9_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN10_GPIO  (((PIN10_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN11_GPIO  (((PIN11_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN12_GPIO  (((PIN12_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN13_GPIO  (((PIN13_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN14_GPIO  (((PIN14_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN15_GPIO  (((PIN15_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN16_GPIO  (((PIN16_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN17_GPIO  (((PIN17_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN18_GPIO  (((PIN18_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN19_GPIO  (((PIN19_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN20_GPIO  (((PIN20_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN21_GPIO  (((PIN21_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN22_GPIO  (((PIN22_PORT_REG & 0x00000700) >> 8) == 1)
#define PIN23_GPIO  (((PIN23_PORT_REG & 0x00000700) >> 8) == 1)

#define PIN_GPIO(pin)     PIN ## pin ## _GPIO

#define GPIO_PIN0_PDOR  (*(volatile uint32_t *)0x43fe0840)
#define GPIO_PIN0_PSOR  (*(volatile uint32_t *)0x43fe08c0)
#define GPIO_PIN0_PCOR  (*(volatile uint32_t *)0x43fe0940)
#define GPIO_PIN0_PTOR  (*(volatile uint32_t *)0x43fe09c0)
#define GPIO_PIN0_PDIR  (*(volatile uint32_t *)0x43fe0a40)
#define GPIO_PIN0_PDDR  (*(volatile uint32_t *)0x43fe0ac0)

#define GPIO_PIN1_PDOR  (*(volatile uint32_t *)0x43fe0844)
#define GPIO_PIN1_PSOR  (*(volatile uint32_t *)0x43fe08c4)
#define GPIO_PIN1_PCOR  (*(volatile uint32_t *)0x43fe0944)
#define GPIO_PIN1_PTOR  (*(volatile uint32_t *)0x43fe09c4)
#define GPIO_PIN1_PDIR  (*(volatile uint32_t *)0x43fe0a44)
#define GPIO_PIN1_PDDR  (*(volatile uint32_t *)0x43fe0ac4)

#define GPIO_PIN2_PDOR  (*(volatile uint32_t *)0x43fe1800)
#define GPIO_PIN2_PSOR  (*(volatile uint32_t *)0x43fe1880)
#define GPIO_PIN2_PCOR  (*(volatile uint32_t *)0x43fe1900)
#define GPIO_PIN2_PTOR  (*(volatile uint32_t *)0x43fe1980)
#define GPIO_PIN2_PDIR  (*(volatile uint32_t *)0x43fe1a00)
#define GPIO_PIN2_PDDR  (*(volatile uint32_t *)0x43fe1a80)

#define GPIO_PIN3_PDOR  (*(volatile uint32_t *)0x43fe0030)
#define GPIO_PIN3_PSOR  (*(volatile uint32_t *)0x43fe00b0)
#define GPIO_PIN3_PCOR  (*(volatile uint32_t *)0x43fe0130)
#define GPIO_PIN3_PTOR  (*(volatile uint32_t *)0x43fe01b0)
#define GPIO_PIN3_PDIR  (*(volatile uint32_t *)0x43fe0230)
#define GPIO_PIN3_PDDR  (*(volatile uint32_t *)0x43fe02b0)

#define GPIO_PIN4_PDOR  (*(volatile uint32_t *)0x43fe0034)
#define GPIO_PIN4_PSOR  (*(volatile uint32_t *)0x43fe00b4)
#define GPIO_PIN4_PCOR  (*(volatile uint32_t *)0x43fe0134)
#define GPIO_PIN4_PTOR  (*(volatile uint32_t *)0x43fe01b4)
#define GPIO_PIN4_PDIR  (*(volatile uint32_t *)0x43fe0234)
#define GPIO_PIN4_PDDR  (*(volatile uint32_t *)0x43fe02b4)

#define GPIO_PIN5_PDOR  (*(volatile uint32_t *)0x43fe181c)
#define GPIO_PIN5_PSOR  (*(volatile uint32_t *)0x43fe189c)
#define GPIO_PIN5_PCOR  (*(volatile uint32_t *)0x43fe191c)
#define GPIO_PIN5_PTOR  (*(volatile uint32_t *)0x43fe199c)
#define GPIO_PIN5_PDIR  (*(volatile uint32_t *)0x43fe1a1c)
#define GPIO_PIN5_PDDR  (*(volatile uint32_t *)0x43fe1a9c)

#define GPIO_PIN6_PDOR  (*(volatile uint32_t *)0x43fe1810)
#define GPIO_PIN6_PSOR  (*(volatile uint32_t *)0x43fe1890)
#define GPIO_PIN6_PCOR  (*(volatile uint32_t *)0x43fe1910)
#define GPIO_PIN6_PTOR  (*(volatile uint32_t *)0x43fe1990)
#define GPIO_PIN6_PDIR  (*(volatile uint32_t *)0x43fe1a10)
#define GPIO_PIN6_PDDR  (*(volatile uint32_t *)0x43fe1a90)

#define GPIO_PIN7_PDOR  (*(volatile uint32_t *)0x43fe1808)
#define GPIO_PIN7_PSOR  (*(volatile uint32_t *)0x43fe1888)
#define GPIO_PIN7_PCOR  (*(volatile uint32_t *)0x43fe1908)
#define GPIO_PIN7_PTOR  (*(volatile uint32_t *)0x43fe1988)
#define GPIO_PIN7_PDIR  (*(volatile uint32_t *)0x43fe1a08)
#define GPIO_PIN7_PDDR  (*(volatile uint32_t *)0x43fe1a88)

#define GPIO_PIN8_PDOR  (*(volatile uint32_t *)0x43fe180c)
#define GPIO_PIN8_PSOR  (*(volatile uint32_t *)0x43fe188c)
#define GPIO_PIN8_PCOR  (*(volatile uint32_t *)0x43fe190c)
#define GPIO_PIN8_PTOR  (*(volatile uint32_t *)0x43fe198c)
#define GPIO_PIN8_PDIR  (*(volatile uint32_t *)0x43fe1a0c)
#define GPIO_PIN8_PDDR  (*(volatile uint32_t *)0x43fe1a8c)

#define GPIO_PIN9_PDOR  (*(volatile uint32_t *)0x43fe100c)
#define GPIO_PIN9_PSOR  (*(volatile uint32_t *)0x43fe108c)
#define GPIO_PIN9_PCOR  (*(volatile uint32_t *)0x43fe110c)
#define GPIO_PIN9_PTOR  (*(volatile uint32_t *)0x43fe118c)
#define GPIO_PIN9_PDIR  (*(volatile uint32_t *)0x43fe120c)
#define GPIO_PIN9_PDDR  (*(volatile uint32_t *)0x43fe128c)

#define GPIO_PIN10_PDOR  (*(volatile uint32_t *)0x43fe1010)
#define GPIO_PIN10_PSOR  (*(volatile uint32_t *)0x43fe1090)
#define GPIO_PIN10_PCOR  (*(volatile uint32_t *)0x43fe1110)
#define GPIO_PIN10_PTOR  (*(volatile uint32_t *)0x43fe1190)
#define GPIO_PIN10_PDIR  (*(volatile uint32_t *)0x43fe1210)
#define GPIO_PIN10_PDDR  (*(volatile uint32_t *)0x43fe1290)

#define GPIO_PIN11_PDOR  (*(volatile uint32_t *)0x43fe1018)
#define GPIO_PIN11_PSOR  (*(volatile uint32_t *)0x43fe1098)
#define GPIO_PIN11_PCOR  (*(volatile uint32_t *)0x43fe1118)
#define GPIO_PIN11_PTOR  (*(volatile uint32_t *)0x43fe1198)
#define GPIO_PIN11_PDIR  (*(volatile uint32_t *)0x43fe1218)
#define GPIO_PIN11_PDDR  (*(volatile uint32_t *)0x43fe1298)

#define GPIO_PIN12_PDOR  (*(volatile uint32_t *)0x43fe101c)
#define GPIO_PIN12_PSOR  (*(volatile uint32_t *)0x43fe109c)
#define GPIO_PIN12_PCOR  (*(volatile uint32_t *)0x43fe111c)
#define GPIO_PIN12_PTOR  (*(volatile uint32_t *)0x43fe119c)
#define GPIO_PIN12_PDIR  (*(volatile uint32_t *)0x43fe121c)
#define GPIO_PIN12_PDDR  (*(volatile uint32_t *)0x43fe129c)

#define GPIO_PIN13_PDOR  (*(volatile uint32_t *)0x43fe1014)
#define GPIO_PIN13_PSOR  (*(volatile uint32_t *)0x43fe1094)
#define GPIO_PIN13_PCOR  (*(volatile uint32_t *)0x43fe1114)
#define GPIO_PIN13_PTOR  (*(volatile uint32_t *)0x43fe1194)
#define GPIO_PIN13_PDIR  (*(volatile uint32_t *)0x43fe1214)
#define GPIO_PIN13_PDDR  (*(volatile uint32_t *)0x43fe1294)

#define GPIO_PIN14_PDOR  (*(volatile uint32_t *)0x43fe1804)
#define GPIO_PIN14_PSOR  (*(volatile uint32_t *)0x43fe1884)
#define GPIO_PIN14_PCOR  (*(volatile uint32_t *)0x43fe1904)
#define GPIO_PIN14_PTOR  (*(volatile uint32_t *)0x43fe1984)
#define GPIO_PIN14_PDIR  (*(volatile uint32_t *)0x43fe1a04)
#define GPIO_PIN14_PDDR  (*(volatile uint32_t *)0x43fe1a84)

#define GPIO_PIN15_PDOR  (*(volatile uint32_t *)0x43fe1000)
#define GPIO_PIN15_PSOR  (*(volatile uint32_t *)0x43fe1080)
#define GPIO_PIN15_PCOR  (*(volatile uint32_t *)0x43fe1100)
#define GPIO_PIN15_PTOR  (*(volatile uint32_t *)0x43fe1180)
#define GPIO_PIN15_PDIR  (*(volatile uint32_t *)0x43fe1200)
#define GPIO_PIN15_PDDR  (*(volatile uint32_t *)0x43fe1280)

#define GPIO_PIN16_PDOR  (*(volatile uint32_t *)0x43fe0800)
#define GPIO_PIN16_PSOR  (*(volatile uint32_t *)0x43fe0880)
#define GPIO_PIN16_PCOR  (*(volatile uint32_t *)0x43fe0900)
#define GPIO_PIN16_PTOR  (*(volatile uint32_t *)0x43fe0980)
#define GPIO_PIN16_PDIR  (*(volatile uint32_t *)0x43fe0a00)
#define GPIO_PIN16_PDDR  (*(volatile uint32_t *)0x43fe0a80)

#define GPIO_PIN17_PDOR  (*(volatile uint32_t *)0x43fe0804)
#define GPIO_PIN17_PSOR  (*(volatile uint32_t *)0x43fe0884)
#define GPIO_PIN17_PCOR  (*(volatile uint32_t *)0x43fe0904)
#define GPIO_PIN17_PTOR  (*(volatile uint32_t *)0x43fe0984)
#define GPIO_PIN17_PDIR  (*(volatile uint32_t *)0x43fe0a04)
#define GPIO_PIN17_PDDR  (*(volatile uint32_t *)0x43fe0a84)

#define GPIO_PIN18_PDOR  (*(volatile uint32_t *)0x43fe080c)
#define GPIO_PIN18_PSOR  (*(volatile uint32_t *)0x43fe088c)
#define GPIO_PIN18_PCOR  (*(volatile uint32_t *)0x43fe090c)
#define GPIO_PIN18_PTOR  (*(volatile uint32_t *)0x43fe098c)
#define GPIO_PIN18_PDIR  (*(volatile uint32_t *)0x43fe0a0c)
#define GPIO_PIN18_PDDR  (*(volatile uint32_t *)0x43fe0a8c)

#define GPIO_PIN19_PDOR  (*(volatile uint32_t *)0x43fe0808)
#define GPIO_PIN19_PSOR  (*(volatile uint32_t *)0x43fe0888)
#define GPIO_PIN19_PCOR  (*(volatile uint32_t *)0x43fe0908)
#define GPIO_PIN19_PTOR  (*(volatile uint32_t *)0x43fe0988)
#define GPIO_PIN19_PDIR  (*(volatile uint32_t *)0x43fe0a08)
#define GPIO_PIN19_PDDR  (*(volatile uint32_t *)0x43fe0a88)

#define GPIO_PIN20_PDOR  (*(volatile uint32_t *)0x43fe1814)
#define GPIO_PIN20_PSOR  (*(volatile uint32_t *)0x43fe1894)
#define GPIO_PIN20_PCOR  (*(volatile uint32_t *)0x43fe1914)
#define GPIO_PIN20_PTOR  (*(volatile uint32_t *)0x43fe1994)
#define GPIO_PIN20_PDIR  (*(volatile uint32_t *)0x43fe1a14)
#define GPIO_PIN20_PDDR  (*(volatile uint32_t *)0x43fe1a94)

#define GPIO_PIN21_PDOR  (*(volatile uint32_t *)0x43fe1818)
#define GPIO_PIN21_PSOR  (*(volatile uint32_t *)0x43fe1898)
#define GPIO_PIN21_PCOR  (*(volatile uint32_t *)0x43fe1918)
#define GPIO_PIN21_PTOR  (*(volatile uint32_t *)0x43fe1998)
#define GPIO_PIN21_PDIR  (*(volatile uint32_t *)0x43fe1a18)
#define GPIO_PIN21_PDDR  (*(volatile uint32_t *)0x43fe1a98)

#define GPIO_PIN22_PDOR  (*(volatile uint32_t *)0x43fe1004)
#define GPIO_PIN22_PSOR  (*(volatile uint32_t *)0x43fe1084)
#define GPIO_PIN22_PCOR  (*(volatile uint32_t *)0x43fe1104)
#define GPIO_PIN22_PTOR  (*(volatile uint32_t *)0x43fe1184)
#define GPIO_PIN22_PDIR  (*(volatile uint32_t *)0x43fe1204)
#define GPIO_PIN22_PDDR  (*(volatile uint32_t *)0x43fe1284)

#define GPIO_PIN23_PDOR  (*(volatile uint32_t *)0x43fe1008)
#define GPIO_PIN23_PSOR  (*(volatile uint32_t *)0x43fe1088)
#define GPIO_PIN23_PCOR  (*(volatile uint32_t *)0x43fe1108)
#define GPIO_PIN23_PTOR  (*(volatile uint32_t *)0x43fe1188)
#define GPIO_PIN23_PDIR  (*(volatile uint32_t *)0x43fe1208)
#define GPIO_PIN23_PDDR  (*(volatile uint32_t *)0x43fe1288)

#define GPIO_PDOR  0
#define GPIO_PSOR  1
#define GPIO_PCOR  2
#define GPIO_PTOR  3
#define GPIO_PDIR  4
#define GPIO_PDDR  5
#define GPIO_PMAX  6

#define GPIO_INPUT(pin)   (GPIO::ddir(pin) == 0)
#define GPIO_OUTPUT(pin)  (GPIO::ddir(pin) == 1)

namespace GPIO
{
    inline uint8_t get(uint8_t pin) __attribute__((always_inline));
    inline void set(uint8_t pin) __attribute__((always_inline));
    inline void clear(uint8_t pin) __attribute__((always_inline));
    inline void toggle(uint8_t pin) __attribute__((always_inline));
    inline uint8_t read(uint8_t pin) __attribute__((always_inline));
    inline uint8_t ddir(uint8_t pin) __attribute__((always_inline));

    volatile uint32_t* const gpio_bitband_alias[][GPIO_PMAX] = {
        { &GPIO_PIN0_PDOR, &GPIO_PIN0_PSOR, &GPIO_PIN0_PCOR, &GPIO_PIN0_PTOR, &GPIO_PIN0_PDIR, &GPIO_PIN0_PDDR },
        { &GPIO_PIN1_PDOR, &GPIO_PIN1_PSOR, &GPIO_PIN1_PCOR, &GPIO_PIN1_PTOR, &GPIO_PIN1_PDIR, &GPIO_PIN1_PDDR },
        { &GPIO_PIN2_PDOR, &GPIO_PIN2_PSOR, &GPIO_PIN2_PCOR, &GPIO_PIN2_PTOR, &GPIO_PIN2_PDIR, &GPIO_PIN2_PDDR },
        { &GPIO_PIN3_PDOR, &GPIO_PIN3_PSOR, &GPIO_PIN3_PCOR, &GPIO_PIN3_PTOR, &GPIO_PIN3_PDIR, &GPIO_PIN3_PDDR },
        { &GPIO_PIN4_PDOR, &GPIO_PIN4_PSOR, &GPIO_PIN4_PCOR, &GPIO_PIN4_PTOR, &GPIO_PIN4_PDIR, &GPIO_PIN4_PDDR },
        { &GPIO_PIN5_PDOR, &GPIO_PIN5_PSOR, &GPIO_PIN5_PCOR, &GPIO_PIN5_PTOR, &GPIO_PIN5_PDIR, &GPIO_PIN5_PDDR },
        { &GPIO_PIN6_PDOR, &GPIO_PIN6_PSOR, &GPIO_PIN6_PCOR, &GPIO_PIN6_PTOR, &GPIO_PIN6_PDIR, &GPIO_PIN6_PDDR },
        { &GPIO_PIN7_PDOR, &GPIO_PIN7_PSOR, &GPIO_PIN7_PCOR, &GPIO_PIN7_PTOR, &GPIO_PIN7_PDIR, &GPIO_PIN7_PDDR },
        { &GPIO_PIN8_PDOR, &GPIO_PIN8_PSOR, &GPIO_PIN8_PCOR, &GPIO_PIN8_PTOR, &GPIO_PIN8_PDIR, &GPIO_PIN8_PDDR },
        { &GPIO_PIN9_PDOR, &GPIO_PIN9_PSOR, &GPIO_PIN9_PCOR, &GPIO_PIN9_PTOR, &GPIO_PIN9_PDIR, &GPIO_PIN9_PDDR },
        { &GPIO_PIN10_PDOR, &GPIO_PIN10_PSOR, &GPIO_PIN10_PCOR, &GPIO_PIN10_PTOR, &GPIO_PIN10_PDIR, &GPIO_PIN10_PDDR },
        { &GPIO_PIN11_PDOR, &GPIO_PIN11_PSOR, &GPIO_PIN11_PCOR, &GPIO_PIN11_PTOR, &GPIO_PIN11_PDIR, &GPIO_PIN11_PDDR },
        { &GPIO_PIN12_PDOR, &GPIO_PIN12_PSOR, &GPIO_PIN12_PCOR, &GPIO_PIN12_PTOR, &GPIO_PIN12_PDIR, &GPIO_PIN12_PDDR },
        { &GPIO_PIN13_PDOR, &GPIO_PIN13_PSOR, &GPIO_PIN13_PCOR, &GPIO_PIN13_PTOR, &GPIO_PIN13_PDIR, &GPIO_PIN13_PDDR },
        { &GPIO_PIN14_PDOR, &GPIO_PIN14_PSOR, &GPIO_PIN14_PCOR, &GPIO_PIN14_PTOR, &GPIO_PIN14_PDIR, &GPIO_PIN14_PDDR },
        { &GPIO_PIN15_PDOR, &GPIO_PIN15_PSOR, &GPIO_PIN15_PCOR, &GPIO_PIN15_PTOR, &GPIO_PIN15_PDIR, &GPIO_PIN15_PDDR },
        { &GPIO_PIN16_PDOR, &GPIO_PIN16_PSOR, &GPIO_PIN16_PCOR, &GPIO_PIN16_PTOR, &GPIO_PIN16_PDIR, &GPIO_PIN16_PDDR },
        { &GPIO_PIN17_PDOR, &GPIO_PIN17_PSOR, &GPIO_PIN17_PCOR, &GPIO_PIN17_PTOR, &GPIO_PIN17_PDIR, &GPIO_PIN17_PDDR },
        { &GPIO_PIN18_PDOR, &GPIO_PIN18_PSOR, &GPIO_PIN18_PCOR, &GPIO_PIN18_PTOR, &GPIO_PIN18_PDIR, &GPIO_PIN18_PDDR },
        { &GPIO_PIN19_PDOR, &GPIO_PIN19_PSOR, &GPIO_PIN19_PCOR, &GPIO_PIN19_PTOR, &GPIO_PIN19_PDIR, &GPIO_PIN19_PDDR },
        { &GPIO_PIN20_PDOR, &GPIO_PIN20_PSOR, &GPIO_PIN20_PCOR, &GPIO_PIN20_PTOR, &GPIO_PIN20_PDIR, &GPIO_PIN20_PDDR },
        { &GPIO_PIN21_PDOR, &GPIO_PIN21_PSOR, &GPIO_PIN21_PCOR, &GPIO_PIN21_PTOR, &GPIO_PIN21_PDIR, &GPIO_PIN21_PDDR },
        { &GPIO_PIN22_PDOR, &GPIO_PIN22_PSOR, &GPIO_PIN22_PCOR, &GPIO_PIN22_PTOR, &GPIO_PIN22_PDIR, &GPIO_PIN22_PDDR },
        { &GPIO_PIN23_PDOR, &GPIO_PIN23_PSOR, &GPIO_PIN23_PCOR, &GPIO_PIN23_PTOR, &GPIO_PIN23_PDIR, &GPIO_PIN23_PDDR }
    };

    inline uint8_t get(uint8_t pin)
    {
        if (__builtin_constant_p(pin)) {
            if (pin == 0) { return (uint8_t)GPIO_PIN0_PDOR; }
            else if (pin == 1) { return (uint8_t)GPIO_PIN1_PDOR; }
            else if (pin == 2) { return (uint8_t)GPIO_PIN2_PDOR; }
            else if (pin == 3) { return (uint8_t)GPIO_PIN3_PDOR; }
            else if (pin == 4) { return (uint8_t)GPIO_PIN4_PDOR; }
            else if (pin == 5) { return (uint8_t)GPIO_PIN5_PDOR; }
            else if (pin == 6) { return (uint8_t)GPIO_PIN6_PDOR; }
            else if (pin == 7) { return (uint8_t)GPIO_PIN7_PDOR; }
            else if (pin == 8) { return (uint8_t)GPIO_PIN8_PDOR; }
            else if (pin == 9) { return (uint8_t)GPIO_PIN9_PDOR; }
            else if (pin == 10) { return (uint8_t)GPIO_PIN10_PDOR; }
            else if (pin == 11) { return (uint8_t)GPIO_PIN11_PDOR; }
            else if (pin == 12) { return (uint8_t)GPIO_PIN12_PDOR; }
            else if (pin == 13) { return (uint8_t)GPIO_PIN13_PDOR; }
            else if (pin == 14) { return (uint8_t)GPIO_PIN14_PDOR; }
            else if (pin == 15) { return (uint8_t)GPIO_PIN15_PDOR; }
            else if (pin == 16) { return (uint8_t)GPIO_PIN16_PDOR; }
            else if (pin == 17) { return (uint8_t)GPIO_PIN17_PDOR; }
            else if (pin == 18) { return (uint8_t)GPIO_PIN18_PDOR; }
            else if (pin == 19) { return (uint8_t)GPIO_PIN19_PDOR; }
            else if (pin == 20) { return (uint8_t)GPIO_PIN20_PDOR; }
            else if (pin == 21) { return (uint8_t)GPIO_PIN21_PDOR; }
            else if (pin == 22) { return (uint8_t)GPIO_PIN22_PDOR; }
            else if (pin == 23) { return (uint8_t)GPIO_PIN23_PDOR; }
            else { return 0; }
        } else {
            return *gpio_bitband_alias[pin][GPIO_PDOR];
        }
    }

    inline void set(uint8_t pin)
    {
        if (__builtin_constant_p(pin)) {
            if (pin == 0) { GPIO_PIN0_PSOR = 1; }
            else if (pin == 1) { GPIO_PIN1_PSOR = 1; }
            else if (pin == 2) { GPIO_PIN2_PSOR = 1; }
            else if (pin == 3) { GPIO_PIN3_PSOR = 1; }
            else if (pin == 4) { GPIO_PIN4_PSOR = 1; }
            else if (pin == 5) { GPIO_PIN5_PSOR = 1; }
            else if (pin == 6) { GPIO_PIN6_PSOR = 1; }
            else if (pin == 7) { GPIO_PIN7_PSOR = 1; }
            else if (pin == 8) { GPIO_PIN8_PSOR = 1; }
            else if (pin == 9) { GPIO_PIN9_PSOR = 1; }
            else if (pin == 10) { GPIO_PIN10_PSOR = 1; }
            else if (pin == 11) { GPIO_PIN11_PSOR = 1; }
            else if (pin == 12) { GPIO_PIN12_PSOR = 1; }
            else if (pin == 13) { GPIO_PIN13_PSOR = 1; }
            else if (pin == 14) { GPIO_PIN14_PSOR = 1; }
            else if (pin == 15) { GPIO_PIN15_PSOR = 1; }
            else if (pin == 16) { GPIO_PIN16_PSOR = 1; }
            else if (pin == 17) { GPIO_PIN17_PSOR = 1; }
            else if (pin == 18) { GPIO_PIN18_PSOR = 1; }
            else if (pin == 19) { GPIO_PIN19_PSOR = 1; }
            else if (pin == 20) { GPIO_PIN20_PSOR = 1; }
            else if (pin == 21) { GPIO_PIN21_PSOR = 1; }
            else if (pin == 22) { GPIO_PIN22_PSOR = 1; }
            else if (pin == 23) { GPIO_PIN23_PSOR = 1; }
        } else {
            *gpio_bitband_alias[pin][GPIO_PSOR] = 1;
        }
    }

    inline void clear(uint8_t pin)
    {
        if (__builtin_constant_p(pin)) {
            if (pin == 0) { GPIO_PIN0_PCOR = 1; }
            else if (pin == 1) { GPIO_PIN1_PCOR = 1; }
            else if (pin == 2) { GPIO_PIN2_PCOR = 1; }
            else if (pin == 3) { GPIO_PIN3_PCOR = 1; }
            else if (pin == 4) { GPIO_PIN4_PCOR = 1; }
            else if (pin == 5) { GPIO_PIN5_PCOR = 1; }
            else if (pin == 6) { GPIO_PIN6_PCOR = 1; }
            else if (pin == 7) { GPIO_PIN7_PCOR = 1; }
            else if (pin == 8) { GPIO_PIN8_PCOR = 1; }
            else if (pin == 9) { GPIO_PIN9_PCOR = 1; }
            else if (pin == 10) { GPIO_PIN10_PCOR = 1; }
            else if (pin == 11) { GPIO_PIN11_PCOR = 1; }
            else if (pin == 12) { GPIO_PIN12_PCOR = 1; }
            else if (pin == 13) { GPIO_PIN13_PCOR = 1; }
            else if (pin == 14) { GPIO_PIN14_PCOR = 1; }
            else if (pin == 15) { GPIO_PIN15_PCOR = 1; }
            else if (pin == 16) { GPIO_PIN16_PCOR = 1; }
            else if (pin == 17) { GPIO_PIN17_PCOR = 1; }
            else if (pin == 18) { GPIO_PIN18_PCOR = 1; }
            else if (pin == 19) { GPIO_PIN19_PCOR = 1; }
            else if (pin == 20) { GPIO_PIN20_PCOR = 1; }
            else if (pin == 21) { GPIO_PIN21_PCOR = 1; }
            else if (pin == 22) { GPIO_PIN22_PCOR = 1; }
            else if (pin == 23) { GPIO_PIN23_PCOR = 1; }
        } else {
            *gpio_bitband_alias[pin][GPIO_PCOR] = 1;
        }
    }

    inline void toggle(uint8_t pin)
    {
        if (__builtin_constant_p(pin)) {
            if (pin == 0) { GPIO_PIN0_PTOR = 1; }
            else if (pin == 1) { GPIO_PIN1_PTOR = 1; }
            else if (pin == 2) { GPIO_PIN2_PTOR = 1; }
            else if (pin == 3) { GPIO_PIN3_PTOR = 1; }
            else if (pin == 4) { GPIO_PIN4_PTOR = 1; }
            else if (pin == 5) { GPIO_PIN5_PTOR = 1; }
            else if (pin == 6) { GPIO_PIN6_PTOR = 1; }
            else if (pin == 7) { GPIO_PIN7_PTOR = 1; }
            else if (pin == 8) { GPIO_PIN8_PTOR = 1; }
            else if (pin == 9) { GPIO_PIN9_PTOR = 1; }
            else if (pin == 10) { GPIO_PIN10_PTOR = 1; }
            else if (pin == 11) { GPIO_PIN11_PTOR = 1; }
            else if (pin == 12) { GPIO_PIN12_PTOR = 1; }
            else if (pin == 13) { GPIO_PIN13_PTOR = 1; }
            else if (pin == 14) { GPIO_PIN14_PTOR = 1; }
            else if (pin == 15) { GPIO_PIN15_PTOR = 1; }
            else if (pin == 16) { GPIO_PIN16_PTOR = 1; }
            else if (pin == 17) { GPIO_PIN17_PTOR = 1; }
            else if (pin == 18) { GPIO_PIN18_PTOR = 1; }
            else if (pin == 19) { GPIO_PIN19_PTOR = 1; }
            else if (pin == 20) { GPIO_PIN20_PTOR = 1; }
            else if (pin == 21) { GPIO_PIN21_PTOR = 1; }
            else if (pin == 22) { GPIO_PIN22_PTOR = 1; }
            else if (pin == 23) { GPIO_PIN23_PTOR = 1; }
        } else {
            *gpio_bitband_alias[pin][GPIO_PTOR] = 1;
        }
    }

    inline uint8_t read(uint8_t pin)
    {
        if (__builtin_constant_p(pin)) {
            if (pin == 0) { return (uint8_t)GPIO_PIN0_PDIR; }
            else if (pin == 1) { return (uint8_t)GPIO_PIN1_PDIR; }
            else if (pin == 2) { return (uint8_t)GPIO_PIN2_PDIR; }
            else if (pin == 3) { return (uint8_t)GPIO_PIN3_PDIR; }
            else if (pin == 4) { return (uint8_t)GPIO_PIN4_PDIR; }
            else if (pin == 5) { return (uint8_t)GPIO_PIN5_PDIR; }
            else if (pin == 6) { return (uint8_t)GPIO_PIN6_PDIR; }
            else if (pin == 7) { return (uint8_t)GPIO_PIN7_PDIR; }
            else if (pin == 8) { return (uint8_t)GPIO_PIN8_PDIR; }
            else if (pin == 9) { return (uint8_t)GPIO_PIN9_PDIR; }
            else if (pin == 10) { return (uint8_t)GPIO_PIN10_PDIR; }
            else if (pin == 11) { return (uint8_t)GPIO_PIN11_PDIR; }
            else if (pin == 12) { return (uint8_t)GPIO_PIN12_PDIR; }
            else if (pin == 13) { return (uint8_t)GPIO_PIN13_PDIR; }
            else if (pin == 14) { return (uint8_t)GPIO_PIN14_PDIR; }
            else if (pin == 15) { return (uint8_t)GPIO_PIN15_PDIR; }
            else if (pin == 16) { return (uint8_t)GPIO_PIN16_PDIR; }
            else if (pin == 17) { return (uint8_t)GPIO_PIN17_PDIR; }
            else if (pin == 18) { return (uint8_t)GPIO_PIN18_PDIR; }
            else if (pin == 19) { return (uint8_t)GPIO_PIN19_PDIR; }
            else if (pin == 20) { return (uint8_t)GPIO_PIN20_PDIR; }
            else if (pin == 21) { return (uint8_t)GPIO_PIN21_PDIR; }
            else if (pin == 22) { return (uint8_t)GPIO_PIN22_PDIR; }
            else if (pin == 23) { return (uint8_t)GPIO_PIN23_PDIR; }
            else { return 0; }
        } else {
            return *gpio_bitband_alias[pin][GPIO_PDIR];
        }
    }

    inline uint8_t ddir(uint8_t pin)
    {
        if (__builtin_constant_p(pin)) {
            if (pin == 0) { return (uint8_t)GPIO_PIN0_PDDR; }
            else if (pin == 1) { return (uint8_t)GPIO_PIN1_PDDR; }
            else if (pin == 2) { return (uint8_t)GPIO_PIN2_PDDR; }
            else if (pin == 3) { return (uint8_t)GPIO_PIN3_PDDR; }
            else if (pin == 4) { return (uint8_t)GPIO_PIN4_PDDR; }
            else if (pin == 5) { return (uint8_t)GPIO_PIN5_PDDR; }
            else if (pin == 6) { return (uint8_t)GPIO_PIN6_PDDR; }
            else if (pin == 7) { return (uint8_t)GPIO_PIN7_PDDR; }
            else if (pin == 8) { return (uint8_t)GPIO_PIN8_PDDR; }
            else if (pin == 9) { return (uint8_t)GPIO_PIN9_PDDR; }
            else if (pin == 10) { return (uint8_t)GPIO_PIN10_PDDR; }
            else if (pin == 11) { return (uint8_t)GPIO_PIN11_PDDR; }
            else if (pin == 12) { return (uint8_t)GPIO_PIN12_PDDR; }
            else if (pin == 13) { return (uint8_t)GPIO_PIN13_PDDR; }
            else if (pin == 14) { return (uint8_t)GPIO_PIN14_PDDR; }
            else if (pin == 15) { return (uint8_t)GPIO_PIN15_PDDR; }
            else if (pin == 16) { return (uint8_t)GPIO_PIN16_PDDR; }
            else if (pin == 17) { return (uint8_t)GPIO_PIN17_PDDR; }
            else if (pin == 18) { return (uint8_t)GPIO_PIN18_PDDR; }
            else if (pin == 19) { return (uint8_t)GPIO_PIN19_PDDR; }
            else if (pin == 20) { return (uint8_t)GPIO_PIN20_PDDR; }
            else if (pin == 21) { return (uint8_t)GPIO_PIN21_PDDR; }
            else if (pin == 22) { return (uint8_t)GPIO_PIN22_PDDR; }
            else if (pin == 23) { return (uint8_t)GPIO_PIN23_PDDR; }
            else { return 0; }
        } else {
            return *gpio_bitband_alias[pin][GPIO_PDDR];
        }
    }
};

#endif
