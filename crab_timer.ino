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
    GPIO::set(PIN_AUDIO_RST);

    // Beeper
    pinMode(PIN_BEEPER, OUTPUT);
    GPIO::clear(PIN_BEEPER);

    // Neopixel base transistor switch
    pinMode(PIN_NEO_TRANS, OUTPUT);
    GPIO::set(PIN_NEO_TRANS);

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
    pinMode(PIN_AUDIO_PLAY, INPUT_PULLUP);
    pinMode(PIN_AUDIO_NEXT, INPUT_PULLUP);
    pinMode(PIN_AUDIO_PREV, INPUT_PULLUP);

    // PowerBoost LBO - Low Battery Output
    pinMode(PIN_LOW_BATT, INPUT);

    // Rotary encoder and push switch
    pinMode(PIN_ROT_ENC_A, INPUT_PULLUP);
    pinMode(PIN_ROT_ENC_B, INPUT_PULLUP);
    pinMode(PIN_ROT_ENC_SW, INPUT);

    // Rotary switch
    pinMode(PIN_ROT_SWI_A, INPUT_PULLUP);
    pinMode(PIN_ROT_SWI_B, INPUT_PULLUP);
    pinMode(PIN_ROT_SWI_C, INPUT_PULLUP);

    s_interface.init();
}

void loop(void)
{
    s_interface.update();
}
