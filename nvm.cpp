#include "nvm.h"
#include <kinetis.h>
#include <cstdint>

// XXX Make sure to set this to what is wanted on *FIRST* use as there
// is no easy way to change it after FlexRAM has been initialized as EEPROM.
// The smaller the value, the higher the write endurance.
// Also this code enforces 32-bit reads and writes so the effective size
// is divided by 4.
#define FLEX_RAM_SIZE  2048

#define FR_SIZE_2048   0x03
#define FR_SIZE_1024   0x04
#define FR_SIZE_512    0x05
#define FR_SIZE_256    0x06
#define FR_SIZE_128    0x07
#define FR_SIZE_64     0x08
#define FR_SIZE_32     0x09
#define FR_SIZE_0      0x0F

#define FR_SIZE(n)  FR_SIZE_ ## n

#if FLEX_RAM_SIZE == 2048
# define FR_SIZE_CODE  (0x30 | FR_SIZE(2048))
#elif FLEX_RAM_SIZE == 1024
# define FR_SIZE_CODE  (0x30 | FR_SIZE(1024))
#elif FLEX_RAM_SIZE == 512
# define FR_SIZE_CODE  (0x30 | FR_SIZE(512))
#elif FLEX_RAM_SIZE == 256
# define FR_SIZE_CODE  (0x30 | FR_SIZE(256))
#elif FLEX_RAM_SIZE == 128
# define FR_SIZE_CODE  (0x30 | FR_SIZE(128))
#elif FLEX_RAM_SIZE == 64
# define FR_SIZE_CODE  (0x30 | FR_SIZE(64))
#elif FLEX_RAM_SIZE == 32
# define FR_SIZE_CODE  (0x30 | FR_SIZE(32))
#else
# error "Invalid FLEX_RAM_SIZE for EEPROM"
#endif

#define FR_MAX_INDEX(eeeprom_size)   (uint16_t)((eeeprom_size - 1) >> 2)

#define SIM_FCFG1_EESIZE   ((uint8_t)((SIM_FCFG1 & 0x000F0000) >> 16))

namespace NVM
{
    uint32_t* const addr = (uint32_t*)0x14000000;
    const int ready_times = 96000;
    bool initialized = false;
    uint16_t max_index = 0;

    bool ready(uint16_t index);
};

// The majority of this function is taken from the teensy3 core file eeprom.c
bool NVM::init(void)
{
    if (NVM::initialized)
      return true;

    // This will only execute if FlexRAM has not been configured as EEPROM.
    // It's a one time deal so be sure to set FLEX_RAM_SIZE accordingly the
    // first time this is used.
    if (FTFL_FCNFG & FTFL_FCNFG_RAMRDY)
    {
        uint16_t do_flash_cmd[] =
        {
            0xf06f, 0x037f,   // 0x00: mvn.w  r3, #127      ; 0x7F
            0x7003,           // 0x04: strb   r3, [r0, #0]
            0x7803,           // 0x06: ldrb   r3, [r0, #0]
            0xf013, 0x0f80,   // 0x08: tst.w  r3, #128      ; 0x80
            0xd0fb,           // 0x0C: beq.n  6 <do_flash_cmd+0x6>
            0x4770            // 0x0E: bx     lr
        };

        // FlexRAM is configured as traditional RAM and needs to be
        // configured for EEPROM usage.
        FTFL_FCCOB0 = 0x80;           // PGMPART = Program Partition Command
        FTFL_FCCOB4 = FR_SIZE_CODE;   // EEPROM Size Code
        FTFL_FCCOB5 = 0x03;           // 0K for Dataflash, 32K for EEPROM backup

        __disable_irq();

        // PJRC: do_flash_cmd() must execute from RAM.  Luckily the C syntax is simple...
        // KTW: I have no idea what the " | 1" is for.
        (*((void (*)(volatile uint8_t *))((uint32_t)do_flash_cmd | 1)))(&FTFL_FSTAT);

        __enable_irq();

        uint8_t status = FTFL_FSTAT;
        if (status & 0x70)
        {
            FTFL_FSTAT = (status & 0x70);
            return false;
        }

        NVM::max_index = FR_MAX_INDEX(FLEX_RAM_SIZE);
    }
    else
    {
        switch (SIM_FCFG1_EESIZE)
        {
            case FR_SIZE(2048):
                NVM::max_index = FR_MAX_INDEX(2048);
                break;
            case FR_SIZE(1024):
                NVM::max_index = FR_MAX_INDEX(1024);
                break;
            case FR_SIZE(512):
                NVM::max_index = FR_MAX_INDEX(512);
                break;
            case FR_SIZE(256):
                NVM::max_index = FR_MAX_INDEX(256);
                break;
            case FR_SIZE(128):
                NVM::max_index = FR_MAX_INDEX(128);
                break;
            case FR_SIZE(64):
                NVM::max_index = FR_MAX_INDEX(64);
                break;
            case FR_SIZE(32):
                NVM::max_index = FR_MAX_INDEX(32);
                break;
            case FR_SIZE(0):
                return false;
        }
    }

    NVM::initialized = true;

    return true;
}

bool NVM::ready(uint16_t index)
{
    // Must have called init() and index must be in bounds
    if (!NVM::initialized || (index > NVM::max_index))
        return false;

    int i;
    for (i = 0; i < NVM::ready_times; i++)
    {
        if (FTFL_FCNFG & FTFL_FCNFG_EEERDY)
            break;
    }

    return (i < NVM::ready_times) ? true : false;
}

bool NVM::read8(uint16_t index, uint8_t& value)
{
    if (!NVM::ready(index))
        return false;

    value = (uint8_t)NVM::addr[index];

    return true;
}

bool NVM::read16(uint16_t index, uint16_t& value)
{
    if (!NVM::ready(index))
        return false;

    value = (uint16_t)NVM::addr[index];

    return true;
}

bool NVM::read32(uint16_t index, uint32_t& value)
{
    if (!NVM::ready(index))
        return false;

    value = NVM::addr[index];

    return true;
}

bool NVM::write(uint16_t index, uint32_t value)
{
    if (!NVM::ready(index))
        return false;

    if (NVM::addr[index] != value)
        NVM::addr[index] = value;

    return true;
}
