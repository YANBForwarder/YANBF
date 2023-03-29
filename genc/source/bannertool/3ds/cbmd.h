#ifndef CBMD_H
#define CBMD_H

#include "../types.h"

#define CBMD_NUM_CGFXS 14

typedef enum {
    CGFX_COMMON,
    CGFX_EUR_ENGLISH,
    CGFX_EUR_FRENCH,
    CGFX_EUR_GERMAN,
    CGFX_EUR_ITALIAN,
    CGFX_EUR_SPANISH,
    CGFX_EUR_DUTCH,
    CGFX_EUR_PORTUGESE,
    CGFX_EUR_RUSSIAN,
    CGFX_JPN_JAPANESE,
    CGFX_USA_ENGLISH,
    CGFX_USA_FRENCH,
    CGFX_USA_SPANISH,
    CGFX_USA_PORTUGESE
} CBMDCGFX;

typedef struct {
    void* cgfxs[CBMD_NUM_CGFXS];
    u32 cgfxSizes[CBMD_NUM_CGFXS];
    void* cwav;
    u32 cwavSize;
} CBMD;

void* cbmd_build(u32* size, CBMD cbmd);
void* bnr_build(u32* size, CBMD cbmd);

#endif