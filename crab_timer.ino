#include "pins.h"
#include "interface.h"

UserInterface s_interface;

void setup(void)
{
    //Serial.begin(9600);
    //while (!Serial);

    // Amplifier shutdown
    pinMode(PIN_AMP_SDWN, OUTPUT);
    GPIO::clear(PIN_AMP_SDWN);

    // VS1053 hardware reset pin
    pinMode(PIN_AUDIO_RST, OUTPUT);
    GPIO::clear(PIN_AUDIO_RST);

    // Beeper
    pinMode(PIN_BEEPER, OUTPUT);
    GPIO::clear(PIN_BEEPER);

    //////////////////////////////////////
    // SPI Chip select pins //////////////
    //////////////////////////////////////
    // SD
    pinMode(PIN_SD_CS, OUTPUT);
    GPIO::set(PIN_SD_CS);

    // VS1053 SCI
    pinMode(PIN_AUDIO_CS, OUTPUT);
    GPIO::set(PIN_AUDIO_CS);

    // VS1053 SDI
    pinMode(PIN_AUDIO_DCS, OUTPUT);
    GPIO::set(PIN_AUDIO_DCS);
    //////////////////////////////////////

    // VS1053 data request pin
    pinMode(PIN_AUDIO_DREQ, INPUT);
    pinMode(PIN_AUDIO_PLAY, INPUT);
    pinMode(PIN_AUDIO_NEXT, INPUT);
    pinMode(PIN_AUDIO_PREV, INPUT);

    // Rotary encoder for brightness
    pinMode(PIN_ROT_ENC_BR_A, INPUT_PULLUP);
    pinMode(PIN_ROT_ENC_BR_B, INPUT_PULLUP);

    // Rotary encoder and push switch
    pinMode(PIN_ROT_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ROT_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ROT_ENC_SW, INPUT);

    // Rotary switch
    pinMode(PIN_ROT_SWI_1, INPUT_PULLUP);
    pinMode(PIN_ROT_SWI_2, INPUT_PULLUP);
    pinMode(PIN_ROT_SWI_3, INPUT_PULLUP);

    s_interface.init();
}

void loop(void)
{
    s_interface.update();
}
