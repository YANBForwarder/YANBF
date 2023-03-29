#include "wav.h"

#include <sstream>
#include <string>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    FILE* fd;
    WavChunkHeader currChunk;
    long currChunkDataPos;
} WavContext;

static bool wav_next_chunk(WavContext* context) {
    if(fseek(context->fd, context->currChunkDataPos + (memcmp(context->currChunk.chunkId, "RIFF", 4) == 0 ? 4 : context->currChunk.chunkSize), SEEK_SET) != 0) {
        return false;
    }

    if(fread(&context->currChunk, 1, sizeof(WavChunkHeader), context->fd) != sizeof(WavChunkHeader)) {
        return false;
    }

    context->currChunkDataPos = ftell(context->fd);
    return true;
}

static bool wav_read_chunk_data(WavContext* context, void* data, size_t maxSize) {
    if(fseek(context->fd, context->currChunkDataPos, SEEK_SET) != 0) {
        return false;
    }

    size_t size = context->currChunk.chunkSize < maxSize ? context->currChunk.chunkSize : maxSize;
    if(fread(data, 1, size, context->fd) != size) {
        return false;
    }

    return true;
}

WAV* wav_read(FILE* fd) {
    WAV* wav = (WAV*) calloc(1, sizeof(WAV));
    if(wav == NULL) {
        printf("ERROR: Could not allocate memory for WAV data.\n");
        return NULL;
    }

    std::string error = "";
    bool riff = false;
    bool fmt = false;
    bool data = false;

    WavContext context;
    memset(&context, 0, sizeof(context));
    context.fd = fd;

    while(error.empty() && (!riff || !fmt || !data) && wav_next_chunk(&context)) {
        if(memcmp(context.currChunk.chunkId, "RIFF", 4) == 0) {
            riff = true;

            char format[4];
            if(wav_read_chunk_data(&context, format, sizeof(format))) {
                if(memcmp(format, "WAVE", sizeof(format)) != 0) {
                    error = "ERROR: RIFF file is not of WAVE format";
                }
            } else {
                error = "ERROR: Failed to read RIFF chunk data";
            }
        } else if(memcmp(context.currChunk.chunkId, "fmt ", 4) == 0) {
            fmt = true;

            if(!wav_read_chunk_data(&context, &wav->format, sizeof(WavFormatChunk))) {
                error = "ERROR: Failed to read fmt chunk data";
            }
        } else if(memcmp(context.currChunk.chunkId, "data", 4) == 0) {
            data = true;

            wav->data.size = context.currChunk.chunkSize;
            wav->data.data = (u8*) malloc(wav->data.size);
            if(wav->data.data != NULL) {
                if(!wav_read_chunk_data(&context, wav->data.data, wav->data.size)) {
                    error = "ERROR: Failed to read data chunk data";
                }
            } else {
                error = "ERROR: Could not allocate memory for WAV samples";
            }
        }
    }

    if(error.empty() && (!riff || !fmt || !data)) {
        std::stringstream stream;
        stream << "ERROR: Missing one or more WAV chunks: ";

        if(!riff) {
            stream << "RIFF";
        }

        if(!fmt) {
            if(!riff) {
                stream << ", ";
            }

            stream << "fmt";
        }

        if(!data) {
            if(!riff || !fmt) {
                stream << ", ";
            }

            stream << "data";
        }

        error = stream.str();
    }

    if(!error.empty()) {
        wav_free(wav);

        if(errno != 0) {
            perror(error.c_str());
        } else {
            printf("%s.\n", error.c_str());
        }

        return NULL;
    }

    return wav;
}

void wav_free(WAV* wav) {
    if(wav != NULL) {
        if(wav->data.data != NULL) {
            free(wav->data.data);
            wav->data.data = NULL;
        }

        free(wav);
    }
}
