#include "player.h"
#include "spi.h"
#include "spi_util.h"
#include "fat32.h"
#include "intr.h"
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
#define VS1053_SM_RESET  0x0004

#define PLAYER_WRITE_SIZE   32
#define PLAYER_STOP_TIME  2000

namespace Player
{
    void reset(void);
    void hardReset(void);
    void softReset(void);
    bool ready(void);
    void setClock(uint16_t val);
    void set(vs1053_reg_t reg, uint16_t val);
    uint16_t get(vs1053_reg_t reg);
    void send32(uint8_t *buf);
    void send(uint8_t* buf, uint8_t len);

    void pause(void);
    void next(void);
    void prev(void);

    uint32_t cta_sci = 0;
    uint32_t cta_sdi = 0;

    uint8_t volume = PLAYER_VOLUME_MAX;

    volatile bool disabled = false;
    volatile bool stopped = false;
    volatile int paused = 1;
    volatile bool pause_depressed = false;
    volatile bool prev_depressed = false;
    volatile bool next_depressed = false;
    volatile uint8_t ffwd = 0;
    volatile uint8_t rwd = 0;

    const char* extensions[3] = {
        "MP3",
        "M4A",
        NULL
    };

    inline bool buttonDepressed(void)
    {
        return (pause_depressed || prev_depressed || next_depressed);
    }
};

void Player::pause(void)
{
    if (Player::disabled)
        return;

    static int depressed = 0;
    static unsigned long depressed_start = 0;

    depressed ^= 1;

    if (depressed)
    {
        depressed_start = millis();
        Player::pause_depressed = true;
    }
    else
    {
        if ((millis() - depressed_start) < PLAYER_STOP_TIME)
        {
            if (Player::stopped)
            {
                Player::resume();
            }
            else
            {
                Player::paused ^= 1;
                GPIO::toggle(PIN_AMP_SDWN);
            }
        }
        else if (Player::stopped)
        {
            Player::resume();
        }
        else
        {
            Player::stop();
        }

        Player::pause_depressed = false;
    }
}

void Player::next(void)
{
    if (Player::disabled || Player::paused || Player::stopped)
        return;

    static int depressed = 0;
    static unsigned long depressed_start = 0;

    depressed ^= 1;

    if (depressed)
    {
        depressed_start = millis();
        Player::next_depressed = true;
    }
    else   // Released
    {
        // Fast forward an extra file per second (actually 1024 ms)
        Player::ffwd = (uint8_t)((millis() - depressed_start) >> 10) + 1;
        Player::next_depressed = false;
    }
}

void Player::prev(void)
{
    if (Player::disabled || Player::paused || Player::stopped)
        return;

    static int depressed = 0;
    static unsigned long depressed_start = 0;

    depressed ^= 1;

    if (depressed)
    {
        depressed_start = millis();
        Player::prev_depressed = true;
    }
    else   // Released
    {
        // Rewind an extra file per second (actually 1024 ms)
        Player::rwd = (uint8_t)((millis() - depressed_start) >> 10) + 1;
        Player::prev_depressed = false;
    }
}

// VS1053
// Max writes are CLKI/4 and reads CLKI/7
// Initially set SPI clock to 1Mhz until SCI_CLOCKF is set which should be around 43MHz
// Then to 10MHz for writes and 6MHz for reads.
// Maybe just set to 10MHz when sending data and 6MHz otherwise
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

    INTR::attach(PIN_AUDIO_PLAY, Player::pause, IRQC_CHANGE);
    INTR::attach(PIN_AUDIO_PREV, Player::prev, IRQC_CHANGE);
    INTR::attach(PIN_AUDIO_NEXT, Player::next, IRQC_CHANGE);

    Player::reset();

    return true;
}

void Player::reset(void)
{
    Player::hardReset();

    // This needs to be set low until clock is set
    // CLKI == XTALI == 12288000
    // CLKI / 7 = 12288000 / 7 = 1755428
    Player::cta_sci = CTAR(1000000, 5, 82, 164);

    // Set CLOCKI to 3.5 x XTALI = 3.5 * 12288000 = 43008000
    Player::setClock(0x8800);
    Player::setVolume(Player::volume);

    // CLKI / 7 = 43008000 / 7 = 6144000
    //Player::cta_sci = CTAR(6000000, 5, 24, 48);
    Player::cta_sci = CTAR(6000000, 0, 0, 0);

    // CLKI / 4 = 43008000 / 4 = 10752000
    //Player::cta_sdi = CTAR(10000000, 5, 24, 0);
    Player::cta_sdi = CTAR(10000000, 0, 0, 0);
}

bool Player::occupied(void)
{
    return (!Player::paused || Player::buttonDepressed());
}

void Player::stop(void)
{
    if (Player::disabled || Player::stopped)
        return;

    GPIO::clear(PIN_AMP_SDWN);
    GPIO::clear(PIN_AUDIO_RST);

    Player::stopped = true;
    Player::paused = 1;
}

void Player::resume(void)
{
    if (Player::disabled || (!Player::stopped && !Player::paused))
        return;

    if (Player::stopped)
    {
        Player::reset();
        Player::stopped = false;

        if (!FAT32::rewind())
            Player::disabled = true;
    }

    Player::paused = 0;
    GPIO::set(PIN_AMP_SDWN);
}

void Player::disable(void)
{
    if (Player::disabled)
        return;

    GPIO::clear(PIN_AMP_SDWN);
    GPIO::clear(PIN_AUDIO_RST);
    Player::disabled = true;
}

bool Player::isPaused(void)
{
    return Player::paused;
}

bool Player::isStopped(void)
{
    return Player::stopped;
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
    Player::set(VS1053_CMD_MODE, mode | VS1053_SM_RESET);
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

void Player::setVolume(uint8_t vol)
{
    // The lower the value the higher the volume, so subtract the
    // value passed in from 255, the max uint8_t.
    // If 1 is passed in, then the value will be: 0xFE = Mute
    // If 0 is passed in, then the value will be:
    //   0xFF = disable analog drivers for power savings.
    uint16_t v = 255 - vol;
    Player::set(VS1053_CMD_VOL, (v << 8) | v);
    Player::volume = vol;
}

uint8_t Player::getVolume(void)
{
    return Player::volume;
    //return (255 - (uint8_t)Player::get(VS1053_CMD_VOL));
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

    if (Player::disabled || Player::paused
            || Player::stopped || Player::buttonDepressed())
    {
        return;
    }

    // Wait a second for VS1053 2048 byte buffer to finish playing file
    // before resetting.
    if (FAT32::eof() && (msec < 1000))
        return;

    if (FAT32::eof() || Player::rwd || Player::ffwd)
    {
        int16_t new_file = 1;

        if (!FAT32::eof())
        {
            noInterrupts();

            new_file = (int16_t)(Player::ffwd - Player::rwd);
            Player::ffwd = 0;
            Player::rwd = 0;

            interrupts();
        }

        Player::softReset();

        bool nfile = (new_file < 0) ? FAT32::prev(-new_file) : FAT32::next(new_file);
        if (!nfile)
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
