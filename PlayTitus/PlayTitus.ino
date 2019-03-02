//Titus the fox music player for ArduinoOPL2 board
//
//Music is extracted from original game, and placed in flash, for playback.
//
//Data file can be found in OpenTitus_0.8.1_src (SourceForge)
//Audio player is based on OpenTitus audio player standalone (SourceForge)
//
// Dependencies:
// ArduinoOPL2 by DhrBaksteen

#include <EEPROM.h>
#include <OPL2.h>
#include "titusplayer.h"
#define CURRENT_SONG_EEPROM_ADDRESS 0

OPL2 opl2;
ADLIB_DATA aad;


//Songs: 0-15

uint8_t song = 15;
//#define CYCLE_SONG_EEPROM
//#define RANDOM_SONG


void setup() {
  opl2.init();

  #ifdef CYCLE_SONG_EEPROM
  //Note: ATMega328 have guaranteed 100.000 EEPROM write/erase cycles
  song = EEPROM.read(CURRENT_SONG_EEPROM_ADDRESS);
  song &= 0x0F;
  EEPROM.write(CURRENT_SONG_EEPROM_ADDRESS, (song + 1) & 0x0F);
  #endif
  
  #ifdef RANDOM_SONG
  randomSeed(analogRead(5)); //Make sure the pin is unconnected
  song = random(0, 15);
  #endif
}

void loop() {
  uint16_t data_size = 18749;

  load_data(&aad, data_size, MUS_OFFSET, INS_OFFSET, song);

  while (1) {
    fillchip(&aad);
    delay(13);
  }
}
