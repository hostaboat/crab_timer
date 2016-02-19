#ifndef _SPI_H_
#define _SPI_H_

#include <cstdint>

#define SPI_SR_TXCTR  ((SPI0_SR & 0x0000F000) >> 12)
#define SPI_SR_RXCTR  ((SPI0_SR & 0x000000F0) >>  4)

#define SPI0_TRANSFER_COMPLETE  (SPI0_SR & SPI0_SR_TCF)

#define SPI_PCS_NONE 0xFF

namespace SPI
{
    bool init(void);

    void enable(void);
    void disable(void);
    bool enabled(void);
    void start(void);
    void stop(void);
    bool running(void);

    void begin(uint8_t pcs, uint32_t cta);
    void end(void);

    void clear(void);
    void flush(void);

    void push8(uint8_t tx);
    uint8_t pop8(void);
    uint8_t trans8(uint8_t tx);

    void push16(uint16_t tx);
    uint16_t pop16(void);
    uint16_t trans16(uint16_t tx);

    uint32_t trans32(uint32_t tx);
};

#endif
