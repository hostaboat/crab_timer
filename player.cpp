#include "player.h"
#include "spi.h"
#include "spi_util.h"
#include "fat32.h"
#include "pins.h"
#include <elapsedMillis.h>
#include <cstdint>

typedef enum
{
    VS1053_CMD_MODE = 0,     // RW
    VS1053_CMD_STATUS,       // RW
    VS1053_CMD_BASS,         // RW
    VS1053_CMD_CLOCKF,       // RW
    VS1053_CMD_DECODE_TIME,  // RW
    VS1053_CMD_AUDATA,       // RW
    VS1053_CMD_WRAM,         // RW
    VS1053_CMD_WRAMADDR,     //  W
    VS1053_CMD_HDAT0,        // R
    VS1053_CMD_HDAT1,        // R
    VS1053_CMD_AIADDR,       // RW
    VS1053_CMD_VOL,          // RW
    VS1053_CMD_AICTRL0,      // RW
    VS1053_CMD_AICTRL1,      // RW
    VS1053_CMD_AICTRL2,      // RW
    VS1053_CMD_AICTRL3       // RW

} vs1053_reg_t;

#define VS1053_CMD_WRITE  0x02
#define VS1053_CMD_READ   0x03

#define SM_RESET  0x0004
#define SM_CANCEL 0x0008

#define PLAYER_WRITE_SIZE  32

#define PLAYER_BUTTON_DEBOUNCE  100

namespace Player
{
    void hardReset(void);
    void softReset(void);
    bool ready(void);
    void setClock(uint16_t val);
    void setVolume(uint8_t l, uint8_t r);
    void set(vs1053_reg_t reg, uint16_t val);
    uint16_t get(vs1053_reg_t reg);
    void send32(uint8_t *buf);
    void send(uint8_t* buf, uint8_t len);

    void pause(void);
    void next(void);
    void prev(void);

    uint32_t cta_sci = 0;
    uint32_t cta_sdi = 0;
    bool disabled = false;
    int paused = 1;
    int8_t next_file = 0;

    const char* extensions[3] = {
        "MP3",
        "M4A",
        NULL
    };
};

void Player::pause(void)
{
    if (Player::disabled)
        return;

    static uint32_t last_update = 0;

    if ((millis() - last_update) > PLAYER_BUTTON_DEBOUNCE)
    {
        Player::paused ^= 1;
        GPIO::toggle(PIN_AMP_SDWN);
        last_update = millis();
    }
}

void Player::next(void)
{
    if (Player::disabled || Player::next_file)
        return;

    static uint32_t last_update = 0;

    if ((millis() - last_update) > PLAYER_BUTTON_DEBOUNCE)
    {
        Player::next_file++;
        last_update = millis();
    }
}

void Player::prev(void)
{
    if (Player::disabled || Player::next_file)
        return;

    static uint32_t last_update = 0;

    if ((millis() - last_update) > PLAYER_BUTTON_DEBOUNCE)
    {
        Player::next_file--;
        last_update = millis();
    }
}

// VS1053
// Max writes are CLKI/4 and reads CLKI/7
// Initially set SPI clock to 1Mhz until SCI_CLOCKF is set which should be around 43MHz
// Then to 10MHz for writes and 6MHz for reads.
// Maybe just set to 10MHz when writing data and 6MHz otherwise
bool Player::init(void)
{
    if (!SPI::init())
    {
        Player::disable();
        return false;
    }

    if (!FAT32::init(PLAYER_WRITE_SIZE, Player::extensions))
    {
        Player::disable();
        return false;
    }

    // XXX Not sure if resetting is necessary since it will have been
    // just powered on anyway
    Player::hardReset();

    // This needs to be set low until clock is set
    // CLKI == XTALI == 12288000
    // CLKI / 7 = 12288000 / 7 = 1755428
    Player::cta_sci = CTAR(1000000, 5, 82, 164);

    // Set CLOCKI to 3.5 x XTALI = 3.5 * 12288000 = 43008000
    Player::setClock(0x8800);
    Player::setVolume(0, 0);   // Loudest
    //Player::setVolume(0xFE, 0xFE);   // Mute

    // CLKI / 7 = 43008000 / 7 = 6144000
    //Player::cta_sci = CTAR(6000000, 5, 24, 48);
    Player::cta_sci = CTAR(6000000, 0, 0, 0);

    // CLKI / 4 = 43008000 / 4 = 10752000
    //Player::cta_sdi = CTAR(10000000, 5, 24, 0);
    Player::cta_sdi = CTAR(10000000, 0, 0, 0);

    attachInterrupt(PIN_AUDIO_PLAY, Player::pause, FALLING);
    attachInterrupt(PIN_AUDIO_NEXT, Player::next, FALLING);
    attachInterrupt(PIN_AUDIO_PREV, Player::prev, FALLING);

    return true;
}

void Player::disable(void)
{
    GPIO::clear(PIN_AMP_SDWN);
    GPIO::clear(PIN_AUDIO_RST);
    Player::disabled = true;
}

bool Player::isDisabled(void)
{
    return Player::disabled;
}

void Player::hardReset(void)
{
    GPIO::clear(PIN_AUDIO_RST);
    delay(100);
    GPIO::set(PIN_AUDIO_RST);
    delay(5);  // Datasheet says about a 1.8 ms delay before DREQ goes back up
}

void Player::softReset(void)
{
    uint16_t mode = Player::get(VS1053_CMD_MODE);
    Player::set(VS1053_CMD_MODE, mode | SM_RESET);
    delay(5);
    //while (!Player::ready());
}

bool Player::ready(void)
{
    return (GPIO::read(PIN_AUDIO_DREQ) == HIGH);
}

void Player::setClock(uint16_t val)
{
    Player::set(VS1053_CMD_CLOCKF, val);
}

void Player::setVolume(uint8_t l, uint8_t r)
{
    Player::set(VS1053_CMD_VOL, ((uint16_t)l << 8) | (uint16_t)r);
}

void Player::set(vs1053_reg_t reg, uint16_t val)
{
    SPI::begin(PIN_AUDIO_CS, Player::cta_sci);

    // XXX Need timeout
    while (!Player::ready());

    switch (reg)
    {
        case VS1053_CMD_MODE:
        case VS1053_CMD_STATUS:
        case VS1053_CMD_BASS:
        case VS1053_CMD_CLOCKF:
        case VS1053_CMD_DECODE_TIME:
        case VS1053_CMD_AUDATA:
        case VS1053_CMD_WRAM:
        case VS1053_CMD_WRAMADDR:
        case VS1053_CMD_AIADDR:
        case VS1053_CMD_VOL:
        case VS1053_CMD_AICTRL0:
        case VS1053_CMD_AICTRL1:
        case VS1053_CMD_AICTRL2:
        case VS1053_CMD_AICTRL3:
            (void)SPI::trans8(VS1053_CMD_WRITE);
            (void)SPI::trans8(reg);
            (void)SPI::trans16(val);
            break;

        case VS1053_CMD_HDAT0:
        case VS1053_CMD_HDAT1:
            break;
    }

    SPI::end();
}

uint16_t Player::get(vs1053_reg_t reg)
{
    uint16_t val = 0;

    SPI::begin(PIN_AUDIO_CS, Player::cta_sci);

    // XXX Need timeout
    while (!Player::ready());

    switch (reg)
    {
        case VS1053_CMD_MODE:
        case VS1053_CMD_STATUS:
        case VS1053_CMD_BASS:
        case VS1053_CMD_CLOCKF:
        case VS1053_CMD_DECODE_TIME:
        case VS1053_CMD_AUDATA:
        case VS1053_CMD_WRAM:
        case VS1053_CMD_HDAT0:
        case VS1053_CMD_HDAT1:
        case VS1053_CMD_AIADDR:
        case VS1053_CMD_VOL:
        case VS1053_CMD_AICTRL0:
        case VS1053_CMD_AICTRL1:
        case VS1053_CMD_AICTRL2:
        case VS1053_CMD_AICTRL3:
            (void)SPI::trans8(VS1053_CMD_READ);
            (void)SPI::trans8(reg);
            val = SPI::trans16(0xFFFF);
            break;

        case VS1053_CMD_WRAMADDR:
            break;
    }

    SPI::end();

    return val;
}

void Player::send32(uint8_t *buf)
{
    uint16_t* p = (uint16_t*)buf;

    SPI::begin(PIN_AUDIO_DCS, Player::cta_sdi);

    //while (!Player::ready());

    for (uint8_t i = 0; i < 4; i++)
    {
        SPI::push16(__builtin_bswap16(*p++));
        SPI::push16(__builtin_bswap16(*p++));
        SPI::push16(__builtin_bswap16(*p++));
        SPI::push16(__builtin_bswap16(*p++));

        while (SPI_SR_TXCTR);
    }

    while (SPI_SR_RXCTR)
        SPI::pop16();

    SPI::end();
}

void Player::send(uint8_t* buf, uint8_t len)
{
    uint16_t* p = (uint16_t*)buf;
    uint8_t len16 = (len & 0x01) ? ((len - 1) >> 1) : (len >> 1);

    SPI::begin(PIN_AUDIO_DCS, Player::cta_sdi);

    //while (!Player::ready());

    for (uint8_t i = 0; i < len16; i++)
    {
        SPI::push16(__builtin_bswap16(*p++));

        while (SPI_SR_TXCTR > 3);
    }

    if (len & 0x01)
        SPI::push8(buf[len - 1]);

    while (SPI_SR_TXCTR);

    while (SPI_SR_RXCTR)
        SPI::pop16();

    SPI::end();
}

void Player::play(void)
{
    static elapsedMillis msec = 0;

    if (Player::disabled || Player::paused)
        return;

    // Wait a second for VS1053 2048 byte buffer to finish playing file
    // before resetting.
    if (FAT32::eof() && (msec < 1000))
        return;

    if (FAT32::eof() || Player::next_file)
    {
        int8_t next_file = 1;

        if (!FAT32::eof())
        {
            noInterrupts();

            next_file = Player::next_file;
            Player::next_file = 0;

            interrupts();
        }

        Player::softReset();

        if (!FAT32::newFile(next_file))
        {
            Player::disable();
            return;
        }
    }

    if (Player::ready())
    {
        uint8_t* p;
        int bytes = FAT32::read(&p);

        if (bytes < 0)
        {
            Player::disable();
            return;
        }

        if (bytes == PLAYER_WRITE_SIZE)  // 32 bytes
            Player::send32(p);
        else
            Player::send(p, bytes);

        if (FAT32::eof())
            msec = 0;
    }
}

