#ifndef CWAV_H
#define CWAV_H

#include "../types.h"

typedef struct {
    u32 channels;
    u32 sampleRate;
    u32 bitsPerSample;

    bool loop;
    u32 loopStartFrame;
    u32 loopEndFrame;

    u32 dataSize;
    void* data;
} CWAV;

void* cwav_build(u32* size, CWAV wav);

#endif