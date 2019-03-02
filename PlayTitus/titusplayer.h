#ifndef TITUSPLAYER_h
#define TITUSPLAYER_h


#define ADLIB_DATA_COUNT 10
#define ADLIB_INSTRUMENT_COUNT 19

#define DATA_OFFSET 28677

#define MUS_OFFSET (28677 - DATA_OFFSET)
#define INS_OFFSET (29029 - DATA_OFFSET)

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

int load_data(ADLIB_DATA *aad, int len, int mus_offset, int ins_offset, int song_number);
int fillchip(ADLIB_DATA *aad);

#endif
