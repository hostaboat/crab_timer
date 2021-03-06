#include "sd.h"
#include "spi.h"
#include "spi_util.h"
#include "pins.h"
#include <elapsedMillis.h>
#include <cstdint>

// Number of clocks to send *without* chip select asserted before sending
// first command.  Specification says a minimum of 74 clocks.
// Send 80, 8-bit (shift divides by 8), so 10 writes.
#define SD_INIT_CLOCKS  80

#define SD_FLOATING_BUS  0xFF

// In milliseconds
#define CMD0_TIMEOUT     250
#define ACMD41_TIMEOUT  1500
#define READ_TIMEOUT     100

// In bytes, i.e number of 8-bit transfers
#define R1_RESP_TRIES  8

#define REQ_START     0x00
#define REQ_TRANSMIT  0x40
#define REQ_CRC       0xF7   // Doesn't matter - see below
#define REQ_END       0x01

#define REQ_HEAD(command_index)  (uint8_t)(REQ_START | REQ_TRANSMIT | (command_index & 0x3F))
#define REQ_TAIL(crc7)           (uint8_t)(((uint8_t)crc7 << 1) | REQ_END)

// GO_IDLE_STATE
// Resets SD Memory Card.  Used in this context to put the card into SPI mode.
// R1 response
#define REQ_CMD0_HEAD   REQ_HEAD(0)
#define REQ_CMD0_ARG    0x00000000
// CRC only necessary for CMD0 since when sending this the card is
// in SD as opposed to SPI mode which requires a checksum.  After
// this command is sent, the card is in SPI mode which does not require
// a valid checksum unless explicitly set via CMD59
#define REQ_CMD0_CRC7   0x4A  // 0x95 after left bit shift and end bit added
#define REQ_CMD0_TAIL   REQ_TAIL(REQ_CMD0_CRC7)

// SEND_IF_COND
// Sends SD Memory Card interface condition that includes host supply voltage
// information and asks the accessed card whether it can operate in the
// supplied voltage range.
// This command was introduced in the SD Group's Physical Layer Specification Version 2.00.
// If the illegal command bit is set in the R1 response, then the card is a Version 1.X
// SD Memory Card and therefore a Standard Capacity SD Memory Card - SDSC
// A valid CRC7 is always required for this command regardless of the CMD59 setting.
// R7 response
#define CMD8_CHECK_PATTERN          0xAA    // Has to be echoed in response
#define CMD8_VHS_UNDEFINED          0x00
#define CMD8_VHS_2_7_TO_3_6         0x01
#define CMD8_VHS_RESVD_LOW_VOLTAGE  0x02
#define CMD8_VHS_RESERVED           0x0C

#define REQ_CMD8_HEAD   REQ_HEAD(8)
#define REQ_CMD8_ARG    (uint32_t)((CMD8_VHS_2_7_TO_3_6 << 8) | CMD8_CHECK_PATTERN)
#define REQ_CMD8_CRC7   0x43  // 0x87 after left bit shift and end bit added
                              // and assuming 0xAA is the check pattern
#define REQ_CMD8_TAIL   REQ_TAIL(REQ_CMD8_CRC7)

// APP_CMD
// Must be sent before application specific commands, i.e. ACMDs.  Really used
// in this context before sending an ACMD41 which is one of many that are already
// "reserved for the SD Memory Card proprietary applications and shall not be used
// by any SD Memory Card manufacturer".
// R1 response
#define REQ_CMD55_HEAD  REQ_HEAD(55)
#define REQ_CMD55_ARG   0x00000000
#define REQ_CMD55_TAIL  REQ_TAIL(REQ_CRC)

// SD_SEND_OP_COND
// Sends host capacity support information and activates the card's
// initialization process.  Set HCS bit to say we support high
// capacity cards if CMD8 was recognized.  This command must be repeatedly
// sent until the idle state bit is cleared in the response.
// R1 response
#define REQ_ACMD41_HEAD      REQ_HEAD(41)
#define REQ_ACMD41_ARG(hcs)  (uint32_t)((hcs) ? (1 << 30) : 0)  // HCS == Host Capacity Support
#define REQ_ACMD41_TAIL      REQ_TAIL(REQ_CRC)

// READ_OCR
// Reads the OCR register of the card.  If the CCS (Card Capacity Status) bit
// is set in the response, then this is a high capacity card - SDHC or SDXC -
// otherwise it is standard capacity - SDSC.  The difference matters in block
// reads, writes and locks.  SDSC argument is a byte offset and SDHC and
// SDXC argument is a 512 byte block offset.
// R3 response
#define REQ_CMD58_HEAD  REQ_HEAD(58)
#define REQ_CMD58_ARG   0x00000000
#define REQ_CMD58_TAIL  REQ_TAIL(REQ_CRC)

// READ_SINGLE_BLOCK
// For SDSC, reads a block of the size selected by the SET_BLOCKLEN command
// (CMD16) which if not explicitly set, supposedly defaults to 512 bytes.
// For SDHC and SDXC, reads a block of size 512 bytes regardless of SET_BLOCKLEN
// command.  Argument for SDSC is a byte offset and for SDHC and SDXC, a block offset.
// R1 response followed by a start block token then block size bytes of data and
// suffixed with a 16-bit CRC generated by the standard CCITT
// polynomial x^16 + x^12 + x^5 + 1.
// Also from the specification, it sounds like instead of a start block token,
// a data error token may be sent if the card can't provide the required data.
#define REQ_CMD17_HEAD            REQ_HEAD(17)
#define REQ_CMD17_ARG(addr, hc)   ((hc) ? (addr) : ((addr) << 9))
#define REQ_CMD17_TAIL            REQ_TAIL(REQ_CRC)

#define RESP_START_BLOCK_TOKEN   0xFE

#define RESP_DATA_ERROR_TOKEN_MASK  0x0F
#define RESP_DATA_ERROR_TOKEN_GENERAL_OR_UNKNOWN   0x01
#define RESP_DATA_ERROR_TOKEN_CARD_CONTROLLER      0x02
#define RESP_DATA_ERROR_TOKEN_CARD_ECC_FAILED      0x04
#define RESP_DATA_ERROR_TOKEN_OUT_OF_RANGE_OR_CSD  0x08

#define RESP_R1_READY            0x00
#define RESP_R1_IDLE_STATE       0x01
#define RESP_R1_ERASE_RESET      0x02
#define RESP_R1_ERR_ILLEGAL_CMD  0x04
#define RESP_R1_ERR_CMD_CRC      0x08
#define RESP_R1_ERR_ERASE_SEQ    0x10
#define RESP_R1_ERR_ADDRESS      0x20
#define RESP_R1_ERR_PARAMETER    0x40
#define RESP_R1_TRANSMIT         0x80   // Should NEVER be set
#define RESP_R1_ERROR (         \
    RESP_R1_ERR_ILLEGAL_CMD |   \
    RESP_R1_ERR_CMD_CRC     |   \
    RESP_R1_ERR_ERASE_SEQ   |   \
    RESP_R1_ERR_ADDRESS     |   \
    RESP_R1_ERR_PARAMETER   )

#define RESP_R2_ERR_CARD_LOCKED                    0x01
#define RESP_R2_ERR_WP_ERASE_OR_LOCK_UNLOCK_FAILED 0x02
#define RESP_R2_ERR_GENERAL_OR_UNKNOWN             0x04
#define RESP_R2_ERR_CARD_CONTROLLER                0x08
#define RESP_R2_ERR_CARD_ECC_FAILED                0x10
#define RESP_R2_ERR_WRITE_PROTECT_VIOLATION        0x20
#define RESP_R2_ERR_INVALID_ERASE_PARAM            0x40
#define RESP_R2_ERR_OUT_OF_RANGE_OR_CSD            0x80
#define RESP_R2_ERROR (                             \
    RESP_R2_ERR_CARD_LOCKED                     |   \
    RESP_R2_ERR_WP_ERASE_OR_LOCK_UNLOCK_FAILED  |   \
    RESP_R2_ERR_GENERAL_UNKNOWN                 |   \
    RESP_R2_ERR_CARD_CONTROLLER                 |   \
    RESP_R2_ERR_CARD_ECC_FAILED                 |   \
    RESP_R2_ERR_WRITE_PROTECT_VIOLATION         |   \
    RESP_R2_ERR_INVALID_ERASE_PARAM             |   \
    RESP_R2_ERR_OUT_OF_RANGE_OR_CSD             )

// In R3 response to CMD58
#define OCR_REG_RESERVED_1            0x00007FFF
#define OCR_REG_VDD_2_7_to_2_8        0x00008000
#define OCR_REG_VDD_2_8_to_2_9        0x00010000
#define OCR_REG_VDD_2_9_to_3_0        0x00020000
#define OCR_REG_VDD_3_0_to_3_1        0x00040000
#define OCR_REG_VDD_3_1_to_3_2        0x00080000
#define OCR_REG_VDD_3_2_to_3_3        0x00100000
#define OCR_REG_VDD_3_3_to_3_4        0x00200000
#define OCR_REG_VDD_3_4_to_3_5        0x00400000
#define OCR_REG_VDD_3_5_to_3_6        0x00800000
#define OCR_REG_VDD_1_8_ACCEPTED      0x01000000
#define OCR_REG_RESERVED_2            0x1E000000
#define OCR_REG_UHS_II_CARD_STATUS    0x20000000
#define OCR_REG_CARD_CAPACITY_STATUS  0x40000000
#define OCR_REG_CARD_POWER_UP_STATUS  0x80000000
#define OCR_REG_RESERVED  (OCR_REG_RESERVED_1 | OCR_REG_RESERVED_2)
#define OCR_REG_VDD (         \
    OCR_REG_VDD_2_7_to_2_8  |  \
    OCR_REG_VDD_2_8_to_2_9  |  \
    OCR_REG_VDD_2_9_to_3_0  |  \
    OCR_REG_VDD_3_0_to_3_1  |  \
    OCR_REG_VDD_3_1_to_3_2  |  \
    OCR_REG_VDD_3_2_to_3_3  |  \
    OCR_REG_VDD_3_3_to_3_4  |  \
    OCR_REG_VDD_3_4_to_3_5  |  \
    OCR_REG_VDD_3_5_to_3_6  )
#define SD_HIGH_CAPACITY(ocr)  (ocr & OCR_REG_CARD_CAPACITY_STATUS)

// In R7 response to CMD8
#define RESP_R7_CHECK_PATTERN    0x000000FF
#define RESP_R7_VHS              0x00000F00
#define RESP_R7_VHS_UNDEFINED    0x00000000
#define RESP_R7_VHS_2_7_TO_3_6   0x00000100
#define RESP_R7_VHS_RESVD_LOW_V  0x00000200
#define RESP_R7_VHS_RESERVED     0x00000C00

namespace SD
{
    void clock74(void);
    bool spiMode(void);
    bool cmd58(uint32_t& ocr);
    bool checkVersion(void);
    bool checkVoltage(void);
    bool ready(void);
    bool checkCapacity(void);

    uint8_t sendCmd(uint8_t head, uint32_t arg, uint8_t tail);
    void endCmd(void);

    bool hcs = false;
    bool hc = false;
    uint32_t cta = 0;
    bool initialized = false;
};

bool SD::init(void)
{
    if (SD::initialized)
        return true;

    // Start with a 375kHz clock - largest clock less than 400kHz
    // Not really clear from spec what other values should be
    SD::cta = CTAR(400000, 0, 0, 0);

    SD::clock74();

    if (!SD::spiMode())
        return false;

    if (!SD::checkVersion())
        return false;

    if (!SD::checkVoltage())
        return false;

    if (!SD::ready())
        return false;

    if (!SD::checkCapacity())
        return false;

    // Can set clock higher now.  May actually be able to do after ready.
    SD::cta = CTAR(25000000, 0, 0, 0);
    SD::initialized = true;

    return true;
}

void SD::clock74(void)
{
    // Specification says that at least 74 clocks must be sent *without*
    // chip select asserted.  Initialize with 80 clocks and use 16-bit frame
    // size so right shift clocks by 4 since each transmit will be 16 clocks.

    SPI::begin(SPI_PCS_NONE, SD::cta);

    for (uint8_t i = 0; i < (SD_INIT_CLOCKS >> 4); i++)
        (void)SPI::trans16(0xFFFF);

    SPI::end();
}

uint8_t SD::sendCmd(uint8_t head, uint32_t arg, uint8_t tail)
{
    SPI::begin(PIN_SD_CS, SD::cta);

    (void)SPI::trans8(head);
    (void)SPI::trans32(arg);
    (void)SPI::trans8(tail);

    // All commands will get an R1 response and according to the
    // specification it can take up to NCR (command response timeout)
    // times to get one.  A SanDisk spec sheet indicates this can be
    // a maximum of 8 units, each unit being 8 clock cycles, so
    // try 8 times to get a response.  It seems that the line
    // will be held high or floating until a valid repsonse is seen.
    // Also, a response never has the first bit set so that may be a
    // valid check as well.
    uint8_t r1;
    for (uint8_t i = 0; i < R1_RESP_TRIES; i++)
    {
        r1 = SPI::trans8(0xFF);
        if (r1 != SD_FLOATING_BUS)  //if (!(r1 & RESP_R1_TRANSMIT))
            break;
    }

    return r1;
}

void SD::endCmd(void)
{
    SPI::end();
    (void)SPI::trans8(0xFF);
}

bool SD::spiMode(void)
{
    uint8_t resp;
    elapsedMillis msec = 0;

    while (true)
    {
        resp = SD::sendCmd(REQ_CMD0_HEAD, REQ_CMD0_ARG, REQ_CMD0_TAIL);

        if ((resp == RESP_R1_IDLE_STATE) || (msec > CMD0_TIMEOUT))
            break;

        SD::endCmd();
    }

    SD::endCmd();

    return (resp == RESP_R1_IDLE_STATE);
}

bool SD::checkVersion(void)
{
    uint8_t resp = SD::sendCmd(REQ_CMD8_HEAD, REQ_CMD8_ARG, REQ_CMD8_TAIL);

    if ((resp & RESP_R1_ERROR) == RESP_R1_ERR_ILLEGAL_CMD)
    {
        SD::hcs = false;
        SD::hc = false;
    }
    else if (resp & RESP_R1_ERROR)
    {
        SD::endCmd();
        return false;
    }
    else
    {
        uint32_t r7 = SPI::trans32(0xFFFFFFFF);
        if ((r7 & REQ_CMD8_ARG) != REQ_CMD8_ARG)
        {
            SD::endCmd();
            return false;
        }

        SD::hcs = true;
    }

    SD::endCmd();

    return true;
}

bool SD::cmd58(uint32_t& ocr)
{
    uint8_t resp = SD::sendCmd(REQ_CMD58_HEAD, REQ_CMD58_ARG, REQ_CMD58_TAIL);

    if (resp & RESP_R1_ERROR)
    {
        SD::endCmd();
        return false;
    }

    ocr = SPI::trans32(0xFFFFFFFF);

    SD::endCmd();

    return true;
}

bool SD::checkVoltage(void)
{
    uint32_t ocr;

    if (!SD::cmd58(ocr))
        return false;

    // XXX Should probably zero in on a value or smaller set of values
    if (!(ocr & OCR_REG_VDD))
        return false;

    return true;
}

bool SD::ready(void)
{
    uint8_t resp;
    elapsedMillis msec = 0;

    while (true)
    {
        // CMD55
        resp = SD::sendCmd(REQ_CMD55_HEAD, REQ_CMD55_ARG, REQ_CMD55_TAIL);
        if (resp & RESP_R1_ERROR)
            break;

        SD::endCmd();

        // ACMD41
        resp = SD::sendCmd(REQ_ACMD41_HEAD, REQ_ACMD41_ARG(SD::hcs), REQ_ACMD41_TAIL);
        if ((resp != RESP_R1_IDLE_STATE) || (msec > ACMD41_TIMEOUT))
            break;

        SD::endCmd();
    }

    SD::endCmd();

    return (resp == RESP_R1_READY);
}

bool SD::checkCapacity(void)
{
    uint32_t ocr;

    if (!SD::hcs)
        return true;

    if (!SD::cmd58(ocr))
        return false;

    if (SD_HIGH_CAPACITY(ocr))
        SD::hc = true;
    else
        SD::hc = false;

    return true;
}

bool SD::read(uint32_t addr, uint8_t (&buf)[READ_BLK_LEN])
{
    uint8_t resp = SD::sendCmd(REQ_CMD17_HEAD, REQ_CMD17_ARG(addr, SD::hc), REQ_CMD17_TAIL);

    if (resp & RESP_R1_ERROR)
    {
        SD::endCmd();
        return false;
    }

    elapsedMillis msec = 0;
    while (true)
    {
        resp = SPI::trans8(0xFF);
        if ((resp != SD_FLOATING_BUS) || (msec > READ_TIMEOUT))
            break;
    }

    if (resp != RESP_START_BLOCK_TOKEN)
    {
        SD::endCmd();
        return false;
    }

    // Total of 514 bytes, 512 bytes data + 2 bytes CRC7

    uint16_t* p = (uint16_t*)buf;

    SPI::push16(0xFFFF);
    SPI::push16(0xFFFF);

    // Reads 510 bytes
    for (uint8_t i = 0; i < 255; i++)
    {
        // Wait while there is nothing to read OR not enough room to write
        while (!SPI_SR_RXCTR || (SPI_SR_TXCTR > 3));
        SPI::push16(0xFFFF);
        *p++ = __builtin_bswap16(SPI::pop16());  // Big to little endian
    }

    // Wait until there are the 2 last entries to read.
    // 2 bytes of data bringing total to 512 and 2 bytes of CRC7
    while (SPI_SR_RXCTR < 2);
    *p = __builtin_bswap16(SPI::pop16());
    (void)SPI::pop16();  // Ignore CRC7

    SD::endCmd();

    return true;
}

