#include "pins.h"
#include "spi.h"
#include <core_pins.h>
#include <kinetis.h>
#include <cstdint>
//#include <WProgram.h>   // For Serial debugging

#define CS_ASSERT(pin)    GPIO::clear(pin)
#define CS_DEASSERT(pin)  GPIO::set(pin)

namespace SPI
{
    uint8_t pcs = SPI_PCS_NONE;
    uint32_t cta = 0;
};

bool SPI::init(void)
{
    if (SPI::enabled())
        return true;

    if ((PIN_MOSI != 7) && (PIN_MOSI != 11))
        return false;

    if ((PIN_MISO != 8) && (PIN_MISO != 12))
        return false;

    if ((PIN_SCK != 13) && (PIN_SCK != 14))
        return false;

    // Open gate
    SPI::enable();

    // Stop if running - not sure if it's possible to be in this state
    // if the gate was closed.
    //if (_SPI_running())
    //    _SPI_stop();

    // PORT_PCR_DSE is Drive Strength Enable
    // The core teensy code sometimes uses this and sometimes not for
    // certain pins.  Not sure if/when it's necessary.
    // Found it to be necessary for pin 14 SCK

    // DOUT / MOSI
    if (PIN_MOSI == 7)
        CORE_PIN7_CONFIG = PORT_PCR_MUX(2);  // | PORT_PCR_DSE
    else // mosi == 11 
        CORE_PIN11_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE;

    // DIN / MISO
    if (PIN_MISO == 8)
        CORE_PIN8_CONFIG = PORT_PCR_MUX(2);  // | PORT_PCR_DSE
    else // miso == 12
        CORE_PIN12_CONFIG = PORT_PCR_MUX(2);  // | PORT_PCR_DSE

    // SCK
    if (PIN_SCK == 13)
        CORE_PIN13_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE;
    else // sck == 14
        CORE_PIN14_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE;

    SPI0_CTAR0 = SPI_CTAR_FMSZ(7);
    SPI0_CTAR1 = SPI_CTAR_FMSZ(15);

    SPI::start();

    return true;
}

void SPI::enable(void)
{
    SIM_SCGC6 |= SIM_SCGC6_SPI0;
}

void SPI::disable(void)
{
    SIM_SCGC6 &= ~SIM_SCGC6_SPI0;
}

bool SPI::enabled(void)
{
    return (SIM_SCGC6 & SIM_SCGC6_SPI0);
}

void SPI::start(void)
{
    if (SPI::running())
        return;

    SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
}

void SPI::stop(void)
{
    if (!SPI::running())
        return;

    SPI0_MCR = SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);   //  | SPI_MCR_MDIS

    while (SPI::running());
}

bool SPI::running(void)
{
    return (SPI0_SR & SPI_SR_TXRXS);
}

void SPI::begin(uint8_t pcs, uint32_t cta)
{
    if ((SPI::cta != cta) || SPI_SR_TXCTR || SPI_SR_RXCTR)
    {
        SPI::stop();

        if (SPI::cta != cta)
        {
            SPI0_CTAR0 = SPI_CTAR_FMSZ(7) | cta;
            SPI0_CTAR1 = SPI_CTAR_FMSZ(15) | cta;
        }

        SPI::start();
    }

    //SPI::clear();
    SPI::pcs = pcs;
    SPI::cta = cta;

    if (SPI::pcs != SPI_PCS_NONE)
        CS_ASSERT(SPI::pcs);

    // Simulated delay before SCK
    delayMicroseconds(1);
}

void SPI::end(void)
{
    // Simulated delay after SCK
    delayMicroseconds(1);
    
    if (SPI::pcs != SPI_PCS_NONE)
        CS_DEASSERT(SPI::pcs);

    // Simulated delay between chip select assertions
    delayMicroseconds(1);
}

void SPI::clear(void)
{
    SPI0_SR |=
        SPI_SR_TCF
        // | SPI_SR_TXRXS   // Not sure if this should be cleared
        | SPI_SR_EOQF
        | SPI_SR_TFUF
        | SPI_SR_TFFF
        | SPI_SR_RFOF
        | SPI_SR_RFDF;
}

void SPI::flush(void)
{
    while (SPI_SR_TXCTR);

    // XXX This may not work.  Might have to stop, clear in MCR then restart.
    // Then again if data is shifted out it should, at the same time, be
    // shifted in.
    while (SPI_SR_RXCTR)
        SPI0_POPR;
}

void SPI::push8(uint8_t tx)
{
    SPI0_PUSHR = tx | SPI_PUSHR_CTAS(0);
}

uint8_t SPI::pop8(void)
{
    return (uint8_t)SPI0_POPR;
}

uint8_t SPI::trans8(uint8_t tx)
{
    SPI::push8(tx);
    while (!SPI_SR_RXCTR);
    return SPI::pop8();
}

void SPI::push16(uint16_t tx)
{
    SPI0_PUSHR = tx | SPI_PUSHR_CTAS(1);
}

uint16_t SPI::pop16(void)
{
    return (uint16_t)SPI0_POPR;
}

uint16_t SPI::trans16(uint16_t tx)
{
    SPI::push16(tx);
    while (!SPI_SR_RXCTR);
    return SPI::pop16();
}

uint32_t SPI::trans32(uint32_t tx)
{
    SPI::push16(tx >> 16);
    SPI::push16(tx & 0xFFFF);
    while (SPI_SR_RXCTR < 2);
    return ((uint32_t)SPI::pop16() << 16) | (uint32_t)SPI::pop16();
}

