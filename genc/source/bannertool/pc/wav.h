#ifndef WAV_H
#define WAV_H

#include "../types.h"

typedef struct {
    char chunkId[4];
    u32 chunkSize;
} WavChunkHeader;

typedef struct {
    u16 format;
    u16 numChannels;
    u32 sampleRate;
    u32 byteRate;
    u16 align;
    u16 bitsPerSample;
} WavFormatChunk;

typedef struct {
    u32 size;
    u8* data;
} WavDataChunk;

typedef struct {
    WavFormatChunk format;
    WavDataChunk data;
} WAV;

WAV* wav_read(FILE* fd);
void wav_free(WAV* wav);

#endif