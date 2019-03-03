#ifndef TITUSPLAYER_h
#define TITUSPLAYER_h


#define ADLIB_DATA_COUNT 10
#define ADLIB_INSTRUMENT_COUNT 19
#define ADLIB_SFX_COUNT 14

enum _OUTPUT_FORMAT{ADLIB, BUZZER};
typedef enum _OUTPUT_FORMAT OUTPUT_FORMAT;

typedef struct {
    unsigned char op[2][5]; //Two operators and five data settings 
    unsigned char fb_alg;
    unsigned char vox; //(only for perc instruments, 0xFE if this is melodic instrument, 0xFF if this instrument is disabled)
} ADLIB_INSTR;

typedef struct {
    unsigned char duration[ADLIB_DATA_COUNT];
    unsigned char volume[ADLIB_DATA_COUNT];
    unsigned char tempo[ADLIB_DATA_COUNT];
    unsigned char triple_duration[ADLIB_DATA_COUNT];
    unsigned char lie[ADLIB_DATA_COUNT];
    unsigned char vox[ADLIB_DATA_COUNT]; //(range: 0-10)
    ADLIB_INSTR *instrument[ADLIB_DATA_COUNT];

    unsigned char delay_counter[ADLIB_DATA_COUNT];
    unsigned char freq[ADLIB_DATA_COUNT];
    unsigned char octave[ADLIB_DATA_COUNT];
    unsigned int return_point[ADLIB_DATA_COUNT];
    unsigned char loop_counter[ADLIB_DATA_COUNT];
    unsigned int pointer[ADLIB_DATA_COUNT];
    unsigned char lie_late[ADLIB_DATA_COUNT];

    unsigned char perc_stat;

    unsigned char skip_delay;
    unsigned char skip_delay_counter;

    signed int cutsong; //Contains the number of active music channels

    int data_size;

    ADLIB_INSTR instrument_data[ADLIB_INSTRUMENT_COUNT];

} ADLIB_DATA;

int load_data(ADLIB_DATA *aad, int song_number);
int load_data_buzzer(ADLIB_DATA *aad, int song_number);
int fillchip(ADLIB_DATA *aad);

//External
uint8_t getHeaderByte(uint16_t offset);
void playBuzzerFreq(uint16_t freq);
extern OUTPUT_FORMAT output_format;

#endif
