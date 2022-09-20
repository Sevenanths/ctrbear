#pragma once
// ifndef for compilers that don't support #pragma once.
#ifndef UT_TD_SOUND
#define UT_TD_SOUND

#include <3ds.h>
#include <stdbool.h>
#include <tremor/ivorbisfile.h>

struct sound {
    const char* filename;
    OggVorbis_File vf;
    ndspWaveBuf waveBuf[2];
    float mix[12];
    long status;
    int section;
    int channel;
    bool block;
    unsigned long block_pos;
    Thread thread;
    LightEvent stopEvent;
};

enum channel {
    BGM = 0,
    SFX
};

void audio_init(void);
struct sound* sound_create(enum channel chan);
void audio_load_ogg(const char *audio, struct sound *sound);
void sound_stop(struct sound *sound);
void audio_stop(void);

#endif
