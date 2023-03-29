#include "cbmd.h"

#include "lz11.h"

#include <stdlib.h>
#include <string.h>

#define CBMD_MAGIC "CBMD"

typedef struct {
    char magic[4];
    u32 zero;
    u32 cgfxOffsets[CBMD_NUM_CGFXS];
    u8 padding[0x44];
    u32 cwavOffset;
} CBMDHeader;

static void* cbmd_build_data(u32* size, CBMD cbmd, bool bnr) {
    CBMDHeader header;
    memset(&header, 0, sizeof(header));

    memcpy(header.magic, CBMD_MAGIC, sizeof(header.magic));

    u32 outputSize = sizeof(CBMDHeader);

    void* compressedCGFXs[14] = {NULL};
    u32 compressedCGFXSizes[14] = {0};
    for(u32 i = 0; i < CBMD_NUM_CGFXS; i++) {
        if(cbmd.cgfxs[i] != NULL) {
            header.cgfxOffsets[i] = outputSize;

            compressedCGFXs[i] = lz11_compress(&compressedCGFXSizes[i], cbmd.cgfxs[i], cbmd.cgfxSizes[i]);
            outputSize += compressedCGFXSizes[i];
        }
    }

    if(bnr) {
        outputSize = (outputSize + 0xF) & ~0xF;
    }

    if(cbmd.cwav != NULL) {
        header.cwavOffset = outputSize;

        outputSize += cbmd.cwavSize;
    }

    void* output = calloc(outputSize, sizeof(u8));
    if(output == NULL) {
        for(u32 i = 0; i < CBMD_NUM_CGFXS; i++) {
            if(cbmd.cgfxs[i] != NULL) {
                free(compressedCGFXs[i]);
            }
        }

        printf("ERROR: Could not allocate memory for CBMD data.\n");
        return NULL;
    }

    memcpy(output, &header, sizeof(header));

    for(u32 i = 0; i < CBMD_NUM_CGFXS; i++) {
        if(compressedCGFXs[i] != NULL) {
            memcpy(&((u8*) output)[header.cgfxOffsets[i]], compressedCGFXs[i], compressedCGFXSizes[i]);
            free(compressedCGFXs[i]);
        }
    }

    if(cbmd.cwav != NULL) {
        memcpy(&((u8*) output)[header.cwavOffset], cbmd.cwav, cbmd.cwavSize);
    }

    if(size != NULL) {
        *size = outputSize;
    }

    return output;
}

void* cbmd_build(u32* size, CBMD cbmd) {
    return cbmd_build_data(size, cbmd, false);
}

void* bnr_build(u32* size, CBMD cbmd) {
    return cbmd_build_data(size, cbmd, true);
}