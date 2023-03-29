#include "cwav.h"

#include <stdlib.h>
#include <string.h>

enum {
    CWAV_REF_DSP_ADPCM_INFO = 0x0300,
    CWAV_REF_IMA_ADPCM_INFO = 0x0301,
    CWAV_REF_SAMPLE_DATA = 0x1F00,
    CWAV_REF_INFO_BLOCK = 0x7000,
    CWAV_REF_DATA_BLOCK = 0x7001,
    CWAV_REF_CHANNEL_INFO = 0x7100
};

typedef struct {
    u16 typeId;
    u16 padding;
    u32 offset;
} CWAVReference;

typedef struct {
    CWAVReference ref;
    u32 size;
} CWAVSizedReference;

typedef struct {
    u32 count;
    CWAVReference contents[0]; // Relative to beginning of CWAVReferenceTable.
} CWAVReferenceTable;

#define CWAV_MAGIC "CWAV"

enum {
    CWAV_ENDIANNESS_LITTLE = 0xFEFF,
    CWAV_ENDIANNESS_BIG = 0xFFFE
};

#define CWAV_VERSION 0x02010000

typedef struct {
    char magic[4];
    u16 endianness;
    u16 headerSize;
    u32 version;
    u32 fileSize;
    u16 numBlocks;
    u16 reserved;
    CWAVSizedReference infoBlock; // Relative to start of file.
    CWAVSizedReference dataBlock; // Relative to start of file.
} CWAVHeader;

#define CWAV_BLOCK_MAGIC_INFO "INFO"
#define CWAV_BLOCK_MAGIC_DATA "DATA"

typedef struct {
    char magic[4];
    u32 size;
} CWAVBlockHeader;

enum {
    CWAV_ENCODING_PCM8 = 0,
    CWAV_ENCODING_PCM16 = 1,
    CWAV_ENCODING_DSP_ADPCM = 2,
    CWAV_ENCODING_IMA_ADPCM = 3
};

typedef struct {
    CWAVBlockHeader header;
    u8 encoding;
    bool loop;
    u16 padding;
    u32 sampleRate;
    u32 loopStartFrame;
    u32 loopEndFrame;
    u32 reserved;
    CWAVReferenceTable channelInfos;
} CWAVInfoBlockHeader;

typedef struct {
    CWAVReference samples; // Relative to CWAVDataBlock.data
    CWAVReference adpcmInfo; // Relative to beginning of CWAVChannelInfo.
    u32 reserved;
} CWAVChannelInfo;

typedef struct {
    u16 coefficients[16];
} CWAVDSPADPCMParam;

typedef struct {
    u8 predictorScale;
    u8 reserved;
    u16 previousSample;
    u16 secondPreviousSample;
} CWAVDSPADPCMContext;

typedef struct {
    CWAVDSPADPCMParam param;
    CWAVDSPADPCMContext context;
    CWAVDSPADPCMContext loopContext;
    u16 padding;
} CWAVDSPADPCMInfo;

typedef struct {
    u16 data;
    u8 tableIndex;
    u8 padding;
} CWAVIMAADPCMContext;

typedef struct {
    CWAVIMAADPCMContext context;
    CWAVIMAADPCMContext loopContext;
} CWAVIMAADPCMInfo;

typedef struct {
    CWAVBlockHeader header;
    u8 data[0];
} CWAVDataBlock;

void* cwav_build(u32* size, CWAV cwav) {
    u32 headerSize = (sizeof(CWAVHeader) + 0x1F) & ~0x1F;
    u32 infoSize = ((sizeof(CWAVInfoBlockHeader) + (cwav.channels * (sizeof(CWAVReference) + sizeof(CWAVChannelInfo)))) + 0x1F) & ~0x1F;
    u32 dataSize = ((sizeof(CWAVDataBlock) + 0x1F) & ~0x1F) + cwav.dataSize;

    u32 outputSize = headerSize + infoSize + dataSize;

    void* output = calloc(outputSize, sizeof(u8));
    if(output == NULL) {
        printf("ERROR: Could not allocate memory for CWAV data.\n");
        return NULL;
    }

    CWAVHeader* header = (CWAVHeader*) &((u8*) output)[0];
    memcpy(header->magic, CWAV_MAGIC, sizeof(header->magic));
    header->endianness = CWAV_ENDIANNESS_LITTLE;
    header->headerSize = (u16) headerSize;
    header->version = CWAV_VERSION;
    header->fileSize = outputSize;
    header->numBlocks = 2;
    header->infoBlock.ref.typeId = CWAV_REF_INFO_BLOCK;
    header->infoBlock.ref.offset = headerSize;
    header->infoBlock.size = infoSize;
    header->dataBlock.ref.typeId = CWAV_REF_DATA_BLOCK;
    header->dataBlock.ref.offset = headerSize + infoSize;
    header->dataBlock.size = dataSize;

    CWAVInfoBlockHeader* infoBlockHeader = (CWAVInfoBlockHeader*) &((u8*) output)[headerSize];
    memcpy(infoBlockHeader->header.magic, CWAV_BLOCK_MAGIC_INFO, sizeof(infoBlockHeader->header.magic));
    infoBlockHeader->header.size = infoSize;
    infoBlockHeader->encoding = cwav.bitsPerSample == 16 ? CWAV_ENCODING_PCM16 : CWAV_ENCODING_PCM8;
    infoBlockHeader->loop = cwav.loop;
    infoBlockHeader->sampleRate = cwav.sampleRate;
    infoBlockHeader->loopStartFrame = cwav.loopStartFrame;
    infoBlockHeader->loopEndFrame = cwav.loopEndFrame != 0 ? cwav.loopEndFrame : cwav.dataSize / cwav.channels / (cwav.bitsPerSample / 8);
    infoBlockHeader->channelInfos.count = cwav.channels;
    for(u32 c = 0; c < cwav.channels; c++) {
        infoBlockHeader->channelInfos.contents[c].typeId = CWAV_REF_CHANNEL_INFO;
        infoBlockHeader->channelInfos.contents[c].offset = sizeof(CWAVReferenceTable) + (cwav.channels * sizeof(CWAVReference)) + (c * sizeof(CWAVChannelInfo));

        CWAVChannelInfo* info = (CWAVChannelInfo*) &((u8*) output)[headerSize + sizeof(CWAVInfoBlockHeader) + (cwav.channels * sizeof(CWAVReference)) + (c * sizeof(CWAVChannelInfo))];
        info->samples.typeId = CWAV_REF_SAMPLE_DATA;
        info->samples.offset = (((sizeof(CWAVDataBlock) + 0x1F) & ~0x1F) - sizeof(CWAVDataBlock)) + (c * (cwav.dataSize / cwav.channels));
        info->adpcmInfo.typeId = 0;
        info->adpcmInfo.offset = 0xFFFFFFFF;
    }

    CWAVDataBlock* dataBlock = (CWAVDataBlock*) &((u8*) output)[headerSize + infoSize];
    memcpy(dataBlock->header.magic, CWAV_BLOCK_MAGIC_DATA, sizeof(dataBlock->header.magic));
    dataBlock->header.size = dataSize;

    for(u32 i = 0; i < cwav.dataSize; i += cwav.channels * (cwav.bitsPerSample / 8)) {
        for(u32 c = 0; c < cwav.channels; c++) {
            CWAVChannelInfo* info = (CWAVChannelInfo*) &((u8*) output)[headerSize + sizeof(CWAVInfoBlockHeader) + (cwav.channels * sizeof(CWAVReference)) + (c * sizeof(CWAVChannelInfo))];

            memcpy(&dataBlock->data[info->samples.offset + (i / cwav.channels)], &((u8*) cwav.data)[i + (c * (cwav.bitsPerSample / 8))], cwav.bitsPerSample / 8);
        }
    }

    if(size != NULL) {
        *size = outputSize;
    }

    return output;
}