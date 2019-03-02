//Titus the fox music player for ArduinoOPL2 board
//
//Music is extracted from original game, and placed in flash, for playback.
//
//Data files can be found in OpenTitus_0.8.1_src (SourceForge / https://github.com/stople/OpenTitusAudioplayStandalone )
//Audio player is based on OpenTitus audio player standalone (SourceForge / https://github.com/stople/OpenTitusAudioplayStandalone )
//
// Dependencies:
// ArduinoOPL2 by DhrBaksteen

#include <EEPROM.h>
#include <OPL2.h>
#include "titusplayer.h"
#define CURRENT_SONG_EEPROM_ADDRESS 0

OPL2 opl2;
ADLIB_DATA aad;


//Source file: Titus, Moktar or Blues Brothers
#define SOURCE_TTF
//#define SOURCE_MOK
//#define SOURCE_BB

//Songs: 0-15 (BB: 0-3)

uint8_t song = 15;
//#define CYCLE_SONG_EEPROM
//#define RANDOM_SONG

#ifdef SOURCE_TTF
//Titus the Fox
#include "music_ttf.h"
#define DATA_ARRAY music_ttf_bin
#define SONG_COUNT 16
#endif

#ifdef SOURCE_MOK
//Moktar
//Is equal to ttf, except song 12 and 14
#include "music_mok.h"
#define DATA_ARRAY music_mok_bin
#define SONG_COUNT 16
#endif

#ifdef SOURCE_BB
//Blues Brothers
#include "music_bb.h"
#define DATA_ARRAY music_bb_bin
#define SONG_COUNT 4
#endif


uint8_t getHeaderByte(uint16_t offset)
{
  return pgm_read_byte_near(DATA_ARRAY + offset);
}



void setup() {
  opl2.init();

  #ifdef CYCLE_SONG_EEPROM
  //Note: ATMega328 have guaranteed 100.000 EEPROM write/erase cycles
  song = EEPROM.read(CURRENT_SONG_EEPROM_ADDRESS);
  EEPROM.write(CURRENT_SONG_EEPROM_ADDRESS, (song + 1) % SONG_COUNT);
  #endif
  
  #ifdef RANDOM_SONG
  randomSeed(analogRead(5)); //Make sure the pin is unconnected
  song = random(0, SONG_COUNT);
  #endif

  song %= SONG_COUNT;
}

void loop() {
  load_data(&aad, song);

  while (1) {
    fillchip(&aad);
    delay(13);
  }
}
