#include "player.h"
#include "vs1053_plugins.h"
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

#define VS1053_CMD_WRITE    0x02
#define VS1053_CMD_READ     0x03

#define VS1053_SM_DIFF            0x0001
#define VS1053_SM_LAYER12         0x0002
#define VS1053_SM_RESET           0x0004
#define VS1053_SM_CANCEL          0x0008
#define VS1053_SM_EARSPEAKER_LO   0x0010
#define VS1053_SM_TESTS           0x0020
#define VS1053_SM_STREAM          0x0040
#define VS1053_SM_EARSPEAKER_HI   0x0080
#define VS1053_SM_DACT            0x0100
#define VS1053_SM_SDIORD          0x0200
#define VS1053_SM_SDISHARE        0x0400
#define VS1053_SM_SDINEW          0x0800
#define VS1053_SM_ADPCM           0x1000
#define VS1053_SM_RESERVED        0x2000
#define VS1053_SM_LINE1           0x4000
#define VS1053_SM_CLK_RANGE       0x8000
#define VS1053_SM_DEFAULT   (VS1053_SM_LINE1 | VS1053_SM_SDINEW)

#define VS1053_END_FILL_BYTE_ADDR  0x1E06

#define PLAYER_WRITE_SIZE   32
#define PLAYER_STOP_TIME  2000

namespace Player
{
    void reset(bool hr);
    void hardReset(void);
    void softReset(void);
    bool ready(void);
    void setClock(uint16_t val);
    bool cancel(void);
    bool finish(void);
    void set(vs1053_reg_t reg, uint16_t val);
    uint16_t get(vs1053_reg_t reg);
    void send32(uint8_t *buf);
    void send(uint8_t* buf, uint8_t len);
    void send32(uint8_t byte);
    void send(uint8_t byte, uint8_t n);

    void pause(void);
    void next(void);
    void prev(void);

    void loadPlugin(const uint16_t* plugin, uint16_t plugin_size);

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
                //GPIO::toggle(PIN_AMP_SDWN);
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

    Player::reset(true);
    GPIO::set(PIN_AMP_SDWN);

    return true;
}

void Player::reset(bool hr)
{
    if (hr)
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
    else
    {
        Player::softReset();
    }

    Player::loadPlugin(vs1053b_patches_plugin, VS1053B_PATCH_PLUGIN_SIZE);
}

bool Player::occupied(void)
{
    return (!Player::paused || Player::buttonDepressed());
}

void Player::stop(void)
{
    if (Player::disabled || Player::stopped)
        return;

    //GPIO::clear(PIN_AMP_SDWN);
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
        Player::reset(true);
        Player::stopped = false;

        if (!FAT32::rewind())
            Player::disabled = true;
    }

    Player::paused = 0;
    //GPIO::set(PIN_AMP_SDWN);
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
    // Don't get and set using old value since it might have SM_CANCEL set.
    // Set to default startup since this is all that should be set.
    //uint16_t mode = Player::get(VS1053_CMD_MODE);
    //Player::set(VS1053_CMD_MODE, mode | VS1053_SM_RESET);
    Player::set(VS1053_CMD_MODE, VS1053_SM_DEFAULT | VS1053_SM_RESET);
    delay(5);
    //while (!Player::ready());
}

bool Player::ready(void)
{
    return (GPIO::read(PIN_AUDIO_DREQ) == HIGH);
}

bool Player::cancel(void)
{
    if (Player::disabled)
        return true;

    if (FAT32::eof())
        return Player::finish();

    // Set SM_CANCEL
    uint16_t mode = Player::get(VS1053_CMD_MODE);
    Player::set(VS1053_CMD_MODE, mode | VS1053_SM_CANCEL);

    // A max of 2048 more bytes of current file needs to be sent until
    // SM_CANCEL is cleared.  Will be sending 32 bytes at a time so 64 sends.
    // If not cleared within 2048 bytes need to software reset (documentation
    // says this should be extremely rare).
    // Documentation seems to be wrong.  In testing it is *not* rare for this
    // to happen, in fact it happens at least one in ten times.
    uint8_t sends;

    for (sends = 0; sends < 64; sends++)
    {
        uint8_t* p;
        int bytes = FAT32::read(&p);

        if (bytes == PLAYER_WRITE_SIZE)  // 32 bytes
            Player::send32(p);
        else
            Player::send(p, bytes);

        if (!(Player::get(VS1053_CMD_MODE) & VS1053_SM_CANCEL))
            break;

        // If the SM_CANCEL flag hasn't cleared and at end of file return
        // false to do software reset.
        if (bytes != PLAYER_WRITE_SIZE)
            return false;
    }

    if (sends == 64)
        return false;

    // Get endFillByte
    Player::set(VS1053_CMD_WRAMADDR, VS1053_END_FILL_BYTE_ADDR);
    uint16_t end_fill_byte = Player::get(VS1053_CMD_WRAM);

    // Need to send 2052 bytes of endFillByte

    // Send 2048 bytes, 32 bytes at a time, so 64 times
    for (int i = 0; i < 64; i++)
        Player::send32(end_fill_byte);

    // Send last 4 bytes of endFillByte
    Player::send(end_fill_byte, 4);

    // According to documentation, both of these should read 0.
    // Again, documentation seems wrong.  HDAT1 is almost *never* 0.
    // There was a user that posted to the VLSI forum and said that
    // it often took ~15 ms for these to clear:
    // http://www.vsdsp-forum.com/phpbb/viewtopic.php?t=920
    // Also, it doesn't seem to affect decoding the next song so just
    // don't bother checking.
    //uint16_t hdat0 = Player::get(VS1053_CMD_HDAT0);
    //uint16_t hdat1 = Player::get(VS1053_CMD_HDAT1);
    //if ((hdat0 != 0) || (hdat1 != 0))
    //    return false;

    return true;
}

bool Player::finish(void)
{
    if (Player::disabled)
        return true;

    if (!FAT32::eof())
        return Player::cancel();

    // Get endFillByte
    Player::set(VS1053_CMD_WRAMADDR, VS1053_END_FILL_BYTE_ADDR);
    uint16_t end_fill_byte = Player::get(VS1053_CMD_WRAM);

    // Need to send at least 2052 bytes of endFillByte

    // Send 2048 bytes, 32 bytes at a time, so 64 times
    for (int i = 0; i < 64; i++)
        Player::send32(end_fill_byte);

    // Send last 4 bytes of endFillByte
    Player::send(end_fill_byte, 4);

    // Set SM_CANCEL
    uint16_t mode = Player::get(VS1053_CMD_MODE);
    Player::set(VS1053_CMD_MODE, mode | VS1053_SM_CANCEL);

    // Send endFillByte 32 bytes at a time checking SM_CANCEL
    // after each send.  If 2048 bytes or more are sent and
    // SM_CANCEL still isn't clear need to do a soft reset.
    // Potentially sending 2048 bytes, 32 bytes at a time, so 64 times.
    uint8_t sends;

    for (sends = 0; sends < 64; sends++)
    {
        Player::send32(end_fill_byte);

        if (!(Player::get(VS1053_CMD_MODE) & VS1053_SM_CANCEL))
            break;
    }

    if (sends == 64)
        return false;

    uint16_t hdat0 = Player::get(VS1053_CMD_HDAT0);
    uint16_t hdat1 = Player::get(VS1053_CMD_HDAT1);

    if ((hdat0 != 0) || (hdat1 != 0))
        return false;

    return true;
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

    while (!Player::ready());

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

    while (!Player::ready());

    for (uint8_t i = 0; i < len16; i++)
    {
        SPI::push16(__builtin_bswap16(*p++));

        while (SPI_SR_TXCTR > 3);
    }

    while (SPI_SR_TXCTR);

    while (SPI_SR_RXCTR)
        SPI::pop16();

    if (len & 0x01)
        (void)SPI::trans8(buf[len - 1]);

    SPI::end();
}

void Player::send32(uint8_t byte)
{
    SPI::begin(PIN_AUDIO_DCS, Player::cta_sdi);

    while (!Player::ready());

    for (uint8_t i = 0; i < 4; i++)
    {
        SPI::push16((uint16_t)byte << 8 | (uint16_t)byte);
        SPI::push16((uint16_t)byte << 8 | (uint16_t)byte);
        SPI::push16((uint16_t)byte << 8 | (uint16_t)byte);
        SPI::push16((uint16_t)byte << 8 | (uint16_t)byte);

        while (SPI_SR_TXCTR);
    }

    while (SPI_SR_RXCTR)
        SPI::pop16();

    SPI::end();
}

void Player::send(uint8_t byte, uint8_t n)
{
    uint8_t n16 = (n & 0x01) ? ((n - 1) >> 1) : (n >> 1);

    SPI::begin(PIN_AUDIO_DCS, Player::cta_sdi);

    while (!Player::ready());

    for (uint8_t i = 0; i < n16; i++)
    {
        SPI::push16((uint16_t)byte << 8 | (uint16_t)byte);

        while (SPI_SR_TXCTR > 3);
    }

    while (SPI_SR_TXCTR);

    while (SPI_SR_RXCTR)
        SPI::pop16();

    if (n & 0x01)
        (void)SPI::trans8(byte);

    SPI::end();
}

void Player::loadPlugin(const uint16_t* plugin, uint16_t plugin_size)
{
    uint16_t i = 0;

    while (i < plugin_size)
    {
        vs1053_reg_t reg;
        uint16_t n, val;

        reg = (vs1053_reg_t)plugin[i++];
        n = plugin[i++];

        if (n & 0x8000)  // RLE run, replicate n samples
        {
            n &= 0x7FFF;
            val = plugin[i++];
            while (n--)
                Player::set(reg, val);
        }
        else  // Copy run, copy n samples
        {
            while (n--)
            {
                val = plugin[i++];
                Player::set(reg, val);
            }
        }
    }
}

//#define PLAYER_PLAY_USE_SOFT_RESET

#ifdef PLAYER_PLAY_USE_SOFT_RESET
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

        Player::reset(false);

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
#else
void Player::play(void)
{
    if (Player::disabled || Player::paused
            || Player::stopped || Player::buttonDepressed())
    {
        return;
    }

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

            // Don't use cancel() since it doesn't work reliably (despite
            // using the exact algorithm defined in the spec) and the
            // amount of time wasted might as well be spent just doing
            // a software reset.
            //if (!Player::cancel())
                Player::reset(false);
        }
        else
        {
            if (!Player::finish())
                Player::reset(false);
        }

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
    }
}
#endif
