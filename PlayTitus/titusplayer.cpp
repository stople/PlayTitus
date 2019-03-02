/*   
 * Copyright (C) 2010 Eirik Stople
 *
 * OpenTitus is  free software; you can redistribute  it and/or modify
 * it under the  terms of the GNU General  Public License as published
 * by the Free  Software Foundation; either version 3  of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of
 * MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU
 * General Public License for more details.
 */

#include <Arduino.h>
#include <OPL2.h>
#include "titusplayer.h"
#include "music_ttf.h"

extern OPL2 opl2;


const unsigned char opera[] = {0,0,1,2,3,4,5,8,9,0xA,0xB,0xC,0xD,0x10,0x11,0x12,0x13,0x14,0x15};
const unsigned char voxp[] = {1,2,3,7,8,9,13,17,15,18,14};
const unsigned int gamme[] = {343,363,385,408,432,458,485,514,544,577,611,647,0};

const int segment = 0x6AF0 - DATA_OFFSET;

void insmaker(unsigned char *insdata, int channel);
void all_vox_zero();

uint8_t getByte(uint16_t offset)
{
  return pgm_read_byte_near(music_ttf_dataonly_bin + offset);
}

void updatechip(int reg, int val)
{
  opl2.write(reg, val);
}

int fillchip(ADLIB_DATA *aad)
{
    int i;
    unsigned char bytedata;
    unsigned char oct;  //.xxx....
    unsigned char freq; //....xxxx
    unsigned int tmp1, tmp2;
    unsigned char tmpC;

    aad->skip_delay_counter--;
    if (aad->skip_delay_counter == 0) {
        aad->skip_delay_counter = aad->skip_delay;
        return (aad->cutsong); //Skip (for modifying tempo)
    }
    for (i = 0; i < ADLIB_DATA_COUNT; i++) {
        if (aad->pointer[i] == 0) continue;
        if (aad->delay_counter[i] > 1) {
            aad->delay_counter[i]--;
            continue;
        }

        do {
        bytedata = getByte(aad->pointer[i]);
        aad->pointer[i]++;
        oct = (bytedata >> 4) & 0x07;
        freq = bytedata & 0x0F;
        if (bytedata & 0x80) { //Escape)
            switch (oct) {
            case 0: //Change duration
                aad->duration[i] = freq;
                break;

            case 1: //Change volume
                aad->volume[i] = freq;
                tmpC = aad->instrument[i]->op[0][2];
                tmpC = (tmpC & 0x3F) - 63;

                tmp1 = (((256 - tmpC) << 4) & 0x0FF0) * (freq + 1);
                tmp1 = 63 - ((tmp1 >> 8) & 0xFF);
                tmp2 = voxp[aad->vox[i]];
                if (tmp2 <= 13)
                    tmp2 += 3;
                tmp2 = opera[tmp2];
                updatechip(0x40 + tmp2, tmp1);
                break;

            case 2: //Change tempo
                aad->tempo[i] = freq;
                break;

            case 3: //Change triple_duration
                aad->triple_duration[i] = freq;
                break;

            case 4: //Change lie
                aad->lie[i] = freq;
                break;

            case 5: //Change vox (channel)
                aad->vox[i] = freq;
                break;

            case 6: //Change instrument
                if (freq == 1) { //Not melodic
                    aad->instrument[i] = &(aad->instrument_data[aad->octave[i] + 15]); //(1st perc instrument is the 16th instrument)
                    aad->vox[i] = aad->instrument[i]->vox;
                    aad->perc_stat = aad->perc_stat | (0x10 >> ((signed int)aad->vox[i] - 6)); //set a bit in perc_stat
                } else {
                    if (freq > 1)
                        freq--;
                    aad->instrument[i] = &(aad->instrument_data[freq]);
                    if (((signed int)aad->vox[i] - 6) >= 0) {
                        tmp1 = (0x10 << ((signed int)aad->vox[i] - 6)) & 0xFF; //
                        aad->perc_stat = aad->perc_stat | tmp1;                //   clear a bit from perc_stat
                        tmp1 = ~(tmp1) & 0xFF;                                 //
                        aad->perc_stat = aad->perc_stat & tmp1;                //
                        updatechip(0xBD, aad->perc_stat); //update perc_stat
                    }
                }
                tmp2 = voxp[aad->vox[i]];
                if (aad->vox[i] <= 6)
                    tmp2 += 3;
                tmp2 = opera[tmp2]; //Adlib channel
                insmaker(aad->instrument[i]->op[0], tmp2);
                if (aad->vox[i] < 7) {
                    insmaker(aad->instrument[i]->op[1], tmp2 - 3);
                    updatechip(0xC0 + aad->vox[i], aad->instrument[i]->fb_alg);
                }
                break;

            case 7: //Extra functions
                switch (freq) {
                case 0: //Call a sub
                    aad->return_point[i] = aad->pointer[i] + 2;
                    tmp1 = ((unsigned int)getByte(aad->pointer[i] + 1) << 8) & 0xFF00;
                    tmp1 += (unsigned int)getByte(aad->pointer[i]) & 0xFF;
                    aad->pointer[i] = segment + tmp1;
                    break;

                case 1: //Update loop counter
                    aad->loop_counter[i] = getByte(aad->pointer[i]);
                    aad->pointer[i]++;
                    break;

                case 2: //Loop
                    if (aad->loop_counter[i] > 1) {
                        aad->loop_counter[i]--;
                        tmp1 = ((unsigned int)getByte(aad->pointer[i] + 1) << 8) & 0xFF00;
                        tmp1 += (unsigned int)getByte(aad->pointer[i]) & 0xFF;
                        aad->pointer[i] = segment + tmp1;
                    } else {
                        aad->pointer[i] += 2;
                    }
                    break;

                case 3: //Return from sub
                    aad->pointer[i] = aad->return_point[i];
                    break;

                case 4: //Jump
                    tmp1 = ((unsigned int)getByte(aad->pointer[i] + 1) << 8) & 0xFF00;
                    tmp1 += (unsigned int)getByte(aad->pointer[i]) & 0xFF;
                    aad->pointer[i] = segment + tmp1;
                    break;

                case 15: //Finish
                    aad->pointer[i] = 0;
                    aad->cutsong--;
                    break;

                }
                break;
            }
        }

        } while ((bytedata & 0x80) && aad->pointer[i]);
        if (aad->pointer[i] == 0) continue;

        aad->octave[i] = oct;
        aad->freq[i] = freq;

        //Play note
        if (gamme[aad->freq[i]] != 0) {
            if (aad->instrument[i]->vox == 0xFE) { //Play a frequence
                updatechip(0xA0 + aad->vox[i], (unsigned char)(gamme[aad->freq[i]] & 0xFF)); //Output lower 8 bits of frequence
                if (aad->lie_late[i] != 1) {
                    updatechip(0xB0 + aad->vox[i], 0); //Silence the channel
                }
                tmp1 = (aad->octave[i] + 2) & 0x07; //Octave (3 bits)
                tmp2 = (unsigned char)((gamme[aad->freq[i]] >> 8) & 0x03); //Frequency (higher 2 bits)
                updatechip(0xB0 + aad->vox[i], 0x20 + (tmp1 << 2) + tmp2); //Voices the channel, and output octave and last bits of frequency
                aad->lie_late[i] = aad->lie[i];

            } else { //Play a perc instrument
                if (aad->instrument[i] != &(aad->instrument_data[aad->octave[i] + 15])) { //New instrument

                    //Similar to escape, oct = 6 (change instrument)
                    aad->instrument[i] = &(aad->instrument_data[aad->octave[i] + 15]); //(1st perc instrument is the 16th instrument)
                    aad->vox[i] = aad->instrument[i]->vox;
                    aad->perc_stat = aad->perc_stat | (0x10 >> ((signed int)aad->vox[i] - 6)); //set a bit in perc_stat
                    tmp2 = voxp[aad->vox[i]];
                    if (aad->vox[i] <= 6)
                        tmp2 += 3;
                    tmp2 = opera[tmp2]; //Adlib channel
                    insmaker(aad->instrument[i]->op[0], tmp2);
                    if (aad->vox[i] < 7) {
                        insmaker(aad->instrument[i]->op[1], tmp2 - 3);
                        updatechip(0xC0 + aad->vox[i], aad->instrument[i]->fb_alg);
                    }

                    //Similar to escape, oct = 1 (change volume)
                    tmpC = aad->instrument[i]->op[0][2];
                    tmpC = (tmpC & 0x3F) - 63;
                    tmp1 = (((256 - tmpC) << 4) & 0x0FF0) * (aad->volume[i] + 1);
                    tmp1 = 63 - ((tmp1 >> 8) & 0xFF);
                    tmp2 = voxp[aad->vox[i]];
                    if (tmp2 <= 13)
                        tmp2 += 3;
                    tmp2 = opera[tmp2];
                    updatechip(0x40 + tmp2, tmp1);
                }
                tmpC = 0x10 >> ((signed int)aad->vox[i] - 6);
                updatechip(0xBD, aad->perc_stat & ~tmpC); //Output perc_stat with one bit removed
                if (aad->vox[i] == 6) {
                    updatechip(0xA6, 0x57); //
                    updatechip(0xB6, 0);    // Output the perc sound
                    updatechip(0xB6, 5);    //
                }
                updatechip(0xBD, aad->perc_stat); //Output perc_stat
            }
        } else {
            aad->lie_late[i] = aad->lie[i];
        }

        if (aad->duration[i] == 7)
            aad->delay_counter[i] = 0x40 >> aad->triple_duration[i];
        else
            aad->delay_counter[i] = 0x60 >> aad->duration[i];
    }
    return (aad->cutsong);
}

void insmaker(unsigned char *insdata, int channel)
{
    updatechip(0x60 + channel, insdata[0]); //Attack Rate / Decay Rate
    updatechip(0x80 + channel, insdata[1]); //Sustain Level / Release Rate
    updatechip(0x40 + channel, insdata[2]); //Key scaling level / Operator output level
    updatechip(0x20 + channel, insdata[3]); //Amp Mod / Vibrato / EG type / Key Scaling / Multiple
    updatechip(0xE0 + channel, insdata[4]); //Wave type
}

int load_data(ADLIB_DATA *aad, int len, int mus_offset, int ins_offset, int song_number)
{
    int i; //Index
    int j; //Offset to the current offset
    int k;
    unsigned int tmp1; //Source offset
    unsigned int tmp2; //Next offset

    aad->perc_stat = 0x20;

    //Load instruments
    j = ins_offset;

    for (i = 0; i < song_number; i++) {
        do {
            j += 2;
            tmp1 = ((unsigned int)getByte(j) & 0xFF) + (((unsigned int)getByte(j + 1) << 8) & 0xFF00);
        } while (tmp1 != 0xFFFF);
        j += 2;
    }
    all_vox_zero();
    tmp2 = ((unsigned int)getByte(j) & 0xFF) + (((unsigned int)getByte(j + 1) << 8) & 0xFF00);

    for (i = 0; i < ADLIB_INSTRUMENT_COUNT + 1; i++)
        aad->instrument_data[i].vox = 0xFF; //Init; instrument not in use

    for (i = 0; (i < ADLIB_INSTRUMENT_COUNT + 1) && ((j + 2) < len); i++) {
        tmp1 = tmp2;
        tmp2 = ((unsigned int)getByte(j + 2) & 0xFF) + (((unsigned int)getByte(j + 3) << 8) & 0xFF00);
        j += 2;

        if (tmp2 == 0xFFFF) //Terminate for loop
            break;

        if (tmp1 == 0) //Instrument not in use
            continue;

        if (i > 14) //Perc instrument (15-18) have an extra byte, melodic (0-14) have not
            aad->instrument_data[i].vox = getByte((tmp1++) + segment);
        else
            aad->instrument_data[i].vox = 0xFE;

        for (k = 0; k < 5; k++)
            aad->instrument_data[i].op[0][k] = getByte((tmp1++) + segment);

        for (k = 0; k < 5; k++)
            aad->instrument_data[i].op[1][k] = getByte((tmp1++) + segment);

        aad->instrument_data[i].fb_alg = getByte(tmp1 + segment);

    }

    //Set skip delay
    aad->skip_delay = tmp1;
    aad->skip_delay_counter = tmp1;

    //Load music
    j = mus_offset;

    for (i = 0; i < song_number; i++) {
        do {
            j += 2;
            tmp1 = ((unsigned int)getByte(j) & 0xFF) + (((unsigned int)getByte(j + 1) << 8) & 0xFF00);
        } while (tmp1 != 0xFFFF);
        j += 2;
    }

    aad->cutsong = -1;
    for (i = 0; (i < ADLIB_DATA_COUNT + 1) && (j < len); i++) {
        tmp1 = ((unsigned int)getByte(j) & 0xFF) + (((unsigned int)getByte(j + 1) << 8) & 0xFF00);
        aad->cutsong++;
        if (tmp1 == 0xFFFF) //Terminate for loop
            break;

        aad->duration[i] = 0;
        aad->volume[i] = 0;
        aad->tempo[i] = 0;
        aad->triple_duration[i] = 0;
        aad->lie[i] = 0;
        aad->vox[i] = (unsigned char)i;
        aad->instrument[i] = NULL;
        //aad->instrument[i] = &(aad->instrument_data[0]);
        aad->delay_counter[i] = 0;
        aad->freq[i] = 0;
        aad->octave[i] = 0;
        aad->return_point[i] = 0;
        aad->loop_counter[i] = 0;
        aad->pointer[i] = segment + tmp1;
        aad->lie_late[i] = 0;
        j += 2;
    }
}

void all_vox_zero()
{
    int i;
    for (i = 0xB0; i < 0xB9; i++)
        updatechip(i, 0); //Clear voice, octave and upper bits of frequence
    for (i = 0xA0; i < 0xB9; i++)
        updatechip(i, 0); //Clear lower byte of frequence

    updatechip(0x08, 0x00);
    updatechip(0xBD, 0x00);
    updatechip(0x40, 0x3F);
    updatechip(0x41, 0x3F);
    updatechip(0x42, 0x3F);
    updatechip(0x43, 0x3F);
    updatechip(0x44, 0x3F);
    updatechip(0x45, 0x3F);
    updatechip(0x48, 0x3F);
    updatechip(0x49, 0x3F);
    updatechip(0x4A, 0x3F);
    updatechip(0x4B, 0x3F);
    updatechip(0x4C, 0x3F);
    updatechip(0x4D, 0x3F);
    updatechip(0x50, 0x3F);
    updatechip(0x51, 0x3F);
    updatechip(0x52, 0x3F);
    updatechip(0x53, 0x3F);
    updatechip(0x54, 0x3F);
    updatechip(0x55, 0x3F);
}