#ifndef SMDH_H
#define SMDH_H

#include "../types.h"

#define SMDH_MAGIC "SMDH"

#define SMDH_NUM_LANGUAGE_SLOTS 16
#define SMDH_NUM_VALID_LANGUAGE_SLOTS 12

#define SMDH_NUM_RATING_SLOTS 16

#define SMDH_SMALL_ICON_SIZE 24
#define SMDH_LARGE_ICON_SIZE 48

typedef enum {
    SMDH_LANGUAGE_JAPANESE,
    SMDH_LANGUAGE_ENGLISH,
    SMDH_LANGUAGE_FRENCH,
    SMDH_LANGUAGE_GERMAN,
    SMDH_LANGUAGE_ITALIAN,
    SMDH_LANGUAGE_SPANISH,
    SMDH_LANGUAGE_SIMPLIFIED_CHINESE,
    SMDH_LANGUAGE_KOREAN,
    SMDH_LANGUAGE_DUTCH,
    SMDH_LANGUAGE_PORTUGUESE,
    SMDH_LANGUAGE_RUSSIAN,
    SMDH_LANGUAGE_TRADITIONAL_CHINESE
} SMDHLanguage;

typedef enum {
    SMDH_RATING_CERO = 0,
    SMDH_RATING_ESRB = 1,
    SMDH_RATING_USK = 3,
    SMDH_RATING_PEGI_GEN = 4,
    SMDH_RATING_PEGI_PTR = 6,
    SMDH_RATING_PEGI_BBFC = 7,
    SMDH_RATING_COB = 8,
    SMDH_RATING_GRB = 9,
    SMDH_RATING_CGSRR = 10
} SMDHRating;

typedef enum {
    SMDH_REGION_JAPAN = 0x01,
    SMDH_REGION_NORTH_AMERICA = 0x02,
    SMDH_REGION_EUROPE = 0x04,
    SMDH_REGION_AUSTRALIA = 0x08,
    SMDH_REGION_CHINA = 0x10,
    SMDH_REGION_KOREA = 0x20,
    SMDH_REGION_TAIWAN = 0x40,

    // Not a bitmask, but a value.
    SMDH_REGION_FREE = 0x7FFFFFFF
} SMDHRegion;

typedef enum {
    SMDH_FLAG_VISIBLE = 0x0001,
    SMDH_FLAG_AUTO_BOOT = 0x0002,
    SMDH_FLAG_ALLOW_3D = 0x0004,
    SMDH_FLAG_REQUIRE_EULA = 0x0008,
    SMDH_FLAG_AUTO_SAVE_ON_EXIT = 0x0010,
    SMDH_FLAG_USE_EXTENDED_BANNER = 0x0020,
    SMDH_FLAG_RATING_REQUIED = 0x0040,
    SMDH_FLAG_USE_SAVE_DATA = 0x0080,
    SMDH_FLAG_RECORD_USAGE = 0x0100,
    SMDH_FLAG_DISABLE_SAVE_BACKUPS = 0x0400,
    SMDH_FLAG_NEW_3DS = 0x1000
} SMDHFlag;

typedef struct {
    u16 shortTitle[0x40];
    u16 longTitle[0x80];
    u16 publisher[0x40];
} SMDHTitle;

typedef struct {
    u8 gameRatings[SMDH_NUM_RATING_SLOTS];
    u32 regionLock;
    u32 matchMakerId;
    u64 matchMakerBitId;
    u32 flags;
    u16 eulaVersion;
    u16 reserved1;
    u32 optimalBannerFrame;
    u32 streetpassId;
} SMDHSettings;

typedef struct {
    char magic[4];
    u16 version;
    u16 reserved0;
    SMDHTitle titles[SMDH_NUM_LANGUAGE_SLOTS];
    SMDHSettings settings;
    u64 reserved1;
    u16 smallIcon[SMDH_SMALL_ICON_SIZE * SMDH_SMALL_ICON_SIZE];
    u16 largeIcon[SMDH_LARGE_ICON_SIZE * SMDH_LARGE_ICON_SIZE];
} SMDH;

#endif
