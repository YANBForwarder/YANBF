#include "cmd.h"

#include "3ds/cbmd.h"
#include "3ds/cwav.h"
#include "3ds/data.h"
#include "3ds/lz11.h"
#include "3ds/smdh.h"
#include "../ext/stb_image.h"
#include "pc/stb_vorbis.h"
#include "pc/wav.h"

#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <codecvt>
#include <locale>
#include <map>
#include <vector>

typedef enum {
    RGB565,
    RGBA4444
} PixelFormat;

#ifdef _WIN32
    #include <stringapiset.h>

    // Based on stbi__fopen
    static FILE *utf8_fopen(char const *filename, char const *mode)
    {
        FILE *f;
        wchar_t wMode[64];
        wchar_t wFilename[1024];
        if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, filename, -1, wFilename, sizeof(wFilename)/sizeof(*wFilename)))
            return 0;

        if (0 == MultiByteToWideChar(65001 /* UTF8 */, 0, mode, -1, wMode, sizeof(wMode)/sizeof(*wMode)))
            return 0;

    #if defined(_MSC_VER) && _MSC_VER >= 1400
        if (0 != _wfopen_s(&f, wFilename, wMode))
            f = 0;
    #else
        f = _wfopen(wFilename, wMode);
    #endif

        return f;
    }
#else
    #define utf8_fopen fopen
#endif

static void utf8_to_utf16(u16* dst, const std::string& src, size_t maxLen) {
    if(maxLen == 0) {
        return;
    }

    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    std::u16string str16 = converter.from_bytes(src.data());

    size_t copyLen = str16.length() * sizeof(char16_t);
    if(copyLen > maxLen) {
        copyLen = maxLen;
    }

    memcpy(dst, str16.data(), copyLen);
}

static void* read_file(u32* size, const std::string& file) {
    FILE* fd = utf8_fopen(file.c_str(), "rb");
    if(fd == NULL) {
        perror("ERROR: Could not open file for reading");
        return NULL;
    }

    long tell = 0;
    if(fseek(fd, 0, SEEK_END) != 0 || (tell = ftell(fd)) < 0 || fseek(fd, 0, SEEK_SET) != 0) {
        fclose(fd);

        perror("ERROR: Failed to determine file size");
        return NULL;
    }

    size_t bufferSize = (size_t) tell;
    void* buffer = malloc(bufferSize);
    if(buffer == NULL) {
        fclose(fd);

        printf("ERROR: Could not allocate memory for file data.\n");
        return NULL;
    }

    size_t readRet = fread(buffer, 1, bufferSize, fd);

    fclose(fd);

    if(readRet != bufferSize) {
        free(buffer);

        perror("ERROR: Failed to read file");
        return NULL;
    }

    if(size != NULL) {
        *size = bufferSize;
    }

    return buffer;
}

static bool write_file(void* contents, u32 size, const std::string& file) {
    FILE* fd = utf8_fopen(file.c_str(), "wb");
    if(fd == NULL) {
        perror("ERROR: Could not open file for writing");
        return false;
    }

    size_t writeRet = fwrite(contents, 1, size, fd);

    fclose(fd);

    if(writeRet != size) {
        perror("ERROR: Failed to write file");
        return false;
    }

    return true;
}

static void* load_image(const std::string& file, u32 width, u32 height) {
    int imgWidth = 0;
    int imgHeight = 0;
    int imgDepth = 0;
    unsigned char* img = stbi_load(file.c_str(), &imgWidth, &imgHeight, &imgDepth, STBI_rgb_alpha);
    if(img == NULL) {
        printf("ERROR: Could not load image file: %s.\n", stbi_failure_reason());
        return NULL;
    }

    if(width == 0) {
        width = (u32) imgWidth;
    }

    if(height == 0) {
        height = (u32) imgHeight;
    }

    if((u32) imgWidth != width || (u32) imgHeight != height) {
        stbi_image_free(img);

        printf("ERROR: Image must be exactly %d x %d in size.\n", width, height);
        return NULL;
    }

    return img;
}

static void free_image(void* img) {
    stbi_image_free(img);
}

static void image_data_to_tiles(void* out, void* img, u32 width, u32 height, PixelFormat format) {
    for(u32 y = 0; y < height; y++) {
        for(u32 x = 0; x < width; x++) {
            u32 index = (((y >> 3) * (width >> 3) + (x >> 3)) << 6) + ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) | ((x & 4) << 2) | ((y & 4) << 3));

            u8* pixel = &((u8*) img)[(y * width + x) * sizeof(u32)];
            u16 color = 0;
            if(format == RGB565) {
                float alpha = pixel[3] / 255.0f;
                color = (u16) ((((u8) (pixel[0] * alpha) & ~0x7) << 8) | (((u8) (pixel[1] * alpha) & ~0x3) << 3) | ((u8) (pixel[2] * alpha) >> 3));
            } else if(format == RGBA4444) {
                color = (u16) (((pixel[0] & ~0xF) << 8) | ((pixel[1] & ~0xF) << 4)| (pixel[2] & ~0xF) | (pixel[3] >> 4));
            }

            ((u16*) out)[index] = color;
        }
    }
}

static void* convert_to_cgfx(u32* size, const std::string& file) {
    u32 bufferSize = BANNER_CGFX_HEADER_LENGTH + (256 * 128 * sizeof(u16));
    void* buffer = malloc(bufferSize);
    if(buffer == NULL) {
        printf("ERROR: Could not allocate memory for CGFX data.\n");
        return NULL;
    }

    void* img = load_image(file, 256, 128);
    if(img == NULL) {
        free(buffer);

        return NULL;
    }

    memcpy(buffer, BANNER_CGFX_HEADER, BANNER_CGFX_HEADER_LENGTH);
    image_data_to_tiles((u16*) ((u8*) buffer + BANNER_CGFX_HEADER_LENGTH), img, 256, 128, RGBA4444);

    free_image(img);

    if(size != NULL) {
        *size = bufferSize;
    }

    return buffer;
}

static void* convert_to_cwav(u32* size, const std::string& file, bool loop, u32 loopStartFrame, u32 loopEndFrame) {
    FILE* fd = utf8_fopen(file.c_str(), "rb");
    if(fd == NULL) {
        perror("ERROR: Failed to open file for CWAV conversion");
        return NULL;
    }

    char magic[4];
    if(fread(magic, 1, sizeof(magic), fd) != sizeof(magic)) {
        fclose(fd);

        perror("ERROR: Failed to read audio file magic");
        return NULL;
    }

    rewind(fd);

    CWAV cwav;
    memset(&cwav, 0, sizeof(cwav));

    cwav.loop = loop;
    cwav.loopStartFrame = loopStartFrame;
    cwav.loopEndFrame = loopEndFrame;

    if(memcmp(magic, "RIFF", sizeof(magic)) == 0) {
        WAV* wav = wav_read(fd);
        if(wav != NULL) {
            cwav.channels = wav->format.numChannels;
            cwav.sampleRate = wav->format.sampleRate;
            cwav.bitsPerSample = wav->format.bitsPerSample;

            cwav.dataSize = wav->data.size;
            cwav.data = calloc(wav->data.size, sizeof(u8));
            if(cwav.data != NULL) {
                memcpy(cwav.data, wav->data.data, wav->data.size);
            } else {
                printf("ERROR: Could not allocate memory for CWAV sample data.\n");
            }

            wav_free(wav);
        }
    } else if(memcmp(magic, "OggS", sizeof(magic)) == 0) {
        int error = 0;
        stb_vorbis* vorbis = stb_vorbis_open_file(fd, false, &error, NULL);

        if(vorbis != NULL) {
            stb_vorbis_info info = stb_vorbis_get_info(vorbis);
            u32 sampleCount = stb_vorbis_stream_length_in_samples(vorbis) * info.channels;

            cwav.channels = (u32) info.channels;
            cwav.sampleRate = info.sample_rate;
            cwav.bitsPerSample = sizeof(u16) * 8;

            cwav.dataSize = sampleCount * sizeof(u16);
            cwav.data = calloc(sampleCount, sizeof(u16));
            if(cwav.data != NULL) {
                stb_vorbis_get_samples_short_interleaved(vorbis, info.channels, (short*) cwav.data, sampleCount);
            } else {
                printf("ERROR: Could not allocate memory for CWAV sample data.\n");
            }

            stb_vorbis_close(vorbis);
        } else {
            printf("ERROR: Failed to open vorbis file: %d.\n", error);
        }
    } else {
        printf("ERROR: Audio file magic '%c%c%c%c' unrecognized.\n", magic[0], magic[1], magic[2], magic[3]);
    }

    fclose(fd);

    if(cwav.data == NULL) {
        return NULL;
    }

    void* ret = cwav_build(size, cwav);

    free(cwav.data);
    return ret;
}

static int cmd_make_banner(const std::string* images, const std::string& audio, const std::string* cgfxFiles, const std::string& cwavFile, const std::string& output) {
    CBMD cbmd;
    memset(&cbmd, 0, sizeof(cbmd));

    bool error = false;

    for(u32 i = 0; i < CBMD_NUM_CGFXS && !error; i++) {
        if(!cgfxFiles[i].empty()) {
            error = (cbmd.cgfxs[i] = read_file(&cbmd.cgfxSizes[i], cgfxFiles[i])) == NULL;
        } else if(!images[i].empty()) {
            error = (cbmd.cgfxs[i] = convert_to_cgfx(&cbmd.cgfxSizes[i], images[i])) == NULL;
        }
    }

    if(!error) {
        if(!cwavFile.empty()) {
            error = (cbmd.cwav = read_file(&cbmd.cwavSize, cwavFile)) == NULL;
        } else {
            error = (cbmd.cwav = convert_to_cwav(&cbmd.cwavSize, audio, false, 0, 0)) == NULL;
        }
    }

    u32 bnrSize = 0;
    void* bnr = !error ? bnr_build(&bnrSize, cbmd) : NULL;

    for(u32 i = 0; i < CBMD_NUM_CGFXS && !error; i++) {
        if(cbmd.cgfxs[i] != NULL) {
            free(cbmd.cgfxs[i]);
        }
    }

    if(cbmd.cwav != NULL) {
        free(cbmd.cwav);
    }

    if(bnr == NULL || !write_file(bnr, bnrSize, output)) {
        return 1;
    }

    printf("Created banner \"%s\".\n", output.c_str());
    return 0;
}

static int cmd_make_smdh(SMDH& smdh, const std::string& icon, const std::string& output) {
    u8* icon48Data = (u8*) load_image(icon.c_str(), SMDH_LARGE_ICON_SIZE, SMDH_LARGE_ICON_SIZE);
    if(icon48Data == NULL) {
        return 1;
    }

    u8 icon24Data[SMDH_SMALL_ICON_SIZE * SMDH_SMALL_ICON_SIZE * sizeof(u32)];

    u32 scale = SMDH_LARGE_ICON_SIZE / SMDH_SMALL_ICON_SIZE;
    u32 samples = scale * scale;
    for(u32 y = 0; y < SMDH_LARGE_ICON_SIZE; y += scale) {
        for(u32 x = 0; x < SMDH_LARGE_ICON_SIZE; x += scale) {
            u32 r = 0;
            u32 g = 0;
            u32 b = 0;
            u32 a = 0;

            for(u32 oy = 0; oy < scale; oy++) {
                for(u32 ox = 0; ox < scale; ox++) {
                    int i = ((y + oy) * SMDH_LARGE_ICON_SIZE + (x + ox)) * sizeof(u32);
                    r += icon48Data[i + 0];
                    g += icon48Data[i + 1];
                    b += icon48Data[i + 2];
                    a += icon48Data[i + 3];
                }
            }

            int i = ((y / scale) * SMDH_SMALL_ICON_SIZE + (x / scale)) * sizeof(u32);
            icon24Data[i + 0] = (u8) (r / samples);
            icon24Data[i + 1] = (u8) (g / samples);
            icon24Data[i + 2] = (u8) (b / samples);
            icon24Data[i + 3] = (u8) (a / samples);
        }
    }

    image_data_to_tiles(smdh.largeIcon, icon48Data, SMDH_LARGE_ICON_SIZE, SMDH_LARGE_ICON_SIZE, RGB565);
    image_data_to_tiles(smdh.smallIcon, icon24Data, SMDH_SMALL_ICON_SIZE, SMDH_SMALL_ICON_SIZE, RGB565);

    free_image(icon48Data);

    if(!write_file(&smdh, sizeof(SMDH), output)) {
        return 1;
    }

    printf("Created SMDH \"%s\".\n", output.c_str());
    return 0;
}

static int cmd_make_cwav(const std::string& input, const std::string& output, bool loop, u32 loopStartFrame, u32 loopEndFrame) {
    u32 cwavSize = 0;
    void* cwav = convert_to_cwav(&cwavSize, input, loop, loopStartFrame, loopEndFrame);
    if(cwav == NULL || !write_file(cwav, cwavSize, output)) {
        return 1;
    }

    printf("Created CWAV \"%s\".\n", output.c_str());
    return 0;
}

static int cmd_lz11(const std::string& input, const std::string& output) {
    u32 size = 0;
    void* data = read_file(&size, input);
    if(data == NULL) {
        return 1;
    }

    u32 compressedSize = 0;
    void* compressed = lz11_compress(&compressedSize, data, size);

    free(data);

    if(compressed == NULL || !write_file(compressed, compressedSize, output)) {
        return 1;
    }

    printf("Compressed to file \"%s\".\n", output.c_str());
    return 0;
}

static std::map<std::string, std::string> cmd_get_args(int argc, char* argv[]) {
    std::map<std::string, std::string> args;
    for(int i = 0; i < argc; i++) {
        if((strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0) && argc != i + 1) {
            args[argv[i]] = argv[i + 1];
            i++;
        }
    }

    return args;
}

static std::string cmd_find_arg(const std::map<std::string, std::string>& args, const std::string& shortOpt, const std::string& longOpt, const std::string& def) {
    std::string sopt = "-" + shortOpt;
    std::string lopt = "--" + longOpt;

    std::map<std::string, std::string>::const_iterator match = args.find(sopt);
    if(match != args.end()) {
        return (*match).second;
    }

    match = args.find(lopt);
    if(match != args.end()) {
        return (*match).second;
    }

    return def;
}

static std::vector<std::string> cmd_parse_list(const std::string& list) {
    std::vector<std::string> ret;
    std::string::size_type lastPos = 0;
    std::string::size_type pos = 0;
    while((pos = list.find(',', lastPos)) != std::string::npos) {
        ret.push_back(list.substr(lastPos, pos - lastPos));
        lastPos = pos + 1;
    }

    if(lastPos < list.size()) {
        ret.push_back(list.substr(lastPos));
    }

    return ret;
}

static void cmd_print_info(const std::string& command) {
    if(command.compare("makebanner") == 0) {
        printf("makebanner - Creates a .bnr file.\n");
        printf("  -i/--image: Optional if specified for a language or with a CGFX. PNG file to use as the banner's default image.\n");
        printf("    -eei/--eurenglishimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR English image.\n");
        printf("    -efi/--eurfrenchimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR French image.\n");
        printf("    -egi/--eurgermanimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR German image.\n");
        printf("    -eii/--euritalianimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR Italian image.\n");
        printf("    -esi/--eurspanishimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR Spanish image.\n");
        printf("    -edi/--eurdutchimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR Dutch image.\n");
        printf("    -epi/--eurportugueseimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR Portuguese image.\n");
        printf("    -eri/--eurrussianimage: Optional if default or CGFX specified. PNG file to use as the banner's EUR Russian image.\n");
        printf("    -jji/--jpnjapaneseimage: Optional if default or CGFX specified. PNG file to use as the banner's JPN Japanese image.\n");
        printf("    -uei/--usaenglishimage: Optional if default or CGFX specified. PNG file to use as the banner's USA English image.\n");
        printf("    -ufi/--usafrenchimage: Optional if default or CGFX specified. PNG file to use as the banner's USA French image.\n");
        printf("    -usi/--usaspanishimage: Optional if default or CGFX specified. PNG file to use as the banner's USA Spanish image.\n");
        printf("    -upi/--usaportugueseimage: Optional if default or CGFX specified. PNG file to use as the banner's USA Portuguese image.\n");
        printf("  -ci/--cgfximage: Optional if specified for a language or with a PNG. CGFX file to use as the banner's default image.\n");
        printf("    -eeci/--eurenglishcgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR English image.\n");
        printf("    -efci/--eurfrenchcgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR French image.\n");
        printf("    -egci/--eurgermancgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR German image.\n");
        printf("    -eici/--euritaliancgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR Italian image.\n");
        printf("    -esci/--eurspanishcgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR Spanish image.\n");
        printf("    -edci/--eurdutchcgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR Dutch image.\n");
        printf("    -epci/--eurportuguesecgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR Portuguese image.\n");
        printf("    -erci/--eurrussiancgfximage: Optional if default or PNG specified. CGFX file to use as the banner's EUR Russian image.\n");
        printf("    -jjci/--jpnjapanesecgfximage: Optional if default or PNG specified. CGFX file to use as the banner's JPN Japanese image.\n");
        printf("    -ueci/--usaenglishcgfximage: Optional if default or PNG specified. CGFX file to use as the banner's USA English image.\n");
        printf("    -ufci/--usafrenchcgfximage: Optional if default or PNG specified. CGFX file to use as the banner's USA French image.\n");
        printf("    -usci/--usaspanishcgfximage: Optional if default or PNG specified. CGFX file to use as the banner's USA Spanish image.\n");
        printf("    -upci/--usaportuguesecgfximage: Optional if default or PNG specified. CGFX file to use as the banner's USA Portuguese image.\n");
        printf("  -a/--audio: Optional if specified for a language or with a CWAV. WAV/OGG file to use as the banner's tune.\n");
        printf("  -ca/--cwavaudio: Optional if specified for a language or with a WAV/OGG. CWAV file to use as the banner's tune.\n");
        printf("  -o/--output: File to output the created banner to.\n");
    } else if(command.compare("makesmdh") == 0) {
        printf("makesmdh - Creates a .smdh/.icn file.\n");
        printf("  -s/--shorttitle: Optional if specified for a language. Default short title of the application.\n");
        printf("    -js/--japaneseshorttitle: Optional if default specified. Japanese short title of the application.\n");
        printf("    -es/--englishshorttitle: Optional if default specified. English short title of the application.\n");
        printf("    -fs/--frenchshorttitle: Optional if default specified. French short title of the application.\n");
        printf("    -gs/--germanshorttitle: Optional if default specified. German short title of the application.\n");
        printf("    -is/--italianshorttitle: Optional if default specified. Italian short title of the application.\n");
        printf("    -ss/--spanishshorttitle: Optional if default specified. Spanish short title of the application.\n");
        printf("    -scs/--simplifiedchineseshorttitle: Optional if default specified. Simplified Chinese short title of the application.\n");
        printf("    -ks/--koreanshorttitle: Optional if default specified. Korean short title of the application.\n");
        printf("    -ds/--dutchshorttitle: Optional if default specified. Dutch short title of the application.\n");
        printf("    -ps/--portugueseshorttitle: Optional if default specified. Portuguese short title of the application.\n");
        printf("    -rs/--russianshorttitle: Optional if default specified. Russian short title of the application.\n");
        printf("    -tcs/--traditionalchineseshorttitle: Optional if default specified. Traditional Chinese short title of the application.\n");
        printf("  -l/--longtitle: Optional if specified for a language. Default long title of the application.\n");
        printf("    -jl/--japaneselongtitle: Optional if default specified. Japanese long title of the application.\n");
        printf("    -el/--englishlongtitle: Optional if default specified. English long title of the application.\n");
        printf("    -fl/--frenchlongtitle: Optional if default specified. French long title of the application.\n");
        printf("    -gl/--germanlongtitle: Optional if default specified. German long title of the application.\n");
        printf("    -il/--italianlongtitle: Optional if default specified. Italian long title of the application.\n");
        printf("    -sl/--spanishlongtitle: Optional if default specified. Spanish long title of the application.\n");
        printf("    -scl/--simplifiedchineselongtitle: Optional if default specified. Simplified Chinese long title of the application.\n");
        printf("    -kl/--koreanlongtitle: Optional if default specified. Korean long title of the application.\n");
        printf("    -dl/--dutchlongtitle: Optional if default specified. Dutch long title of the application.\n");
        printf("    -pl/--portugueselongtitle: Optional if default specified. Portuguese long title of the application.\n");
        printf("    -rl/--russianlongtitle: Optional if default specified. Russian long title of the application.\n");
        printf("    -tcl/--traditionalchineselongtitle: Optional if default specified. Traditional Chinese long title of the application.\n");
        printf("  -p/--publisher: Optional if specified for a language. Default publisher of the application.\n");
        printf("    -jp/--japanesepublisher: Optional if default specified. Japanese publisher of the application.\n");
        printf("    -ep/--englishpublisher: Optional if default specified. English publisher of the application.\n");
        printf("    -fp/--frenchpublisher: Optional if default specified. French publisher of the application.\n");
        printf("    -gp/--germanpublisher: Optional if default specified. German publisher of the application.\n");
        printf("    -ip/--italianpublisher: Optional if default specified. Italian publisher of the application.\n");
        printf("    -sp/--spanishpublisher: Optional if default specified. Spanish publisher of the application.\n");
        printf("    -scp/--simplifiedchinesepublisher: Optional if default specified. Simplified Chinese publisher of the application.\n");
        printf("    -kp/--koreanpublisher: Optional if default specified. Korean publisher of the application.\n");
        printf("    -dp/--dutchpublisher: Optional if default specified. Dutch publisher of the application.\n");
        printf("    -pp/--portuguesepublisher: Optional if default specified. Portuguese publisher of the application.\n");
        printf("    -rp/--russianpublisher: Optional if default specified. Russian publisher of the application.\n");
        printf("    -tcp/--traditionalchinesepublisher: Optional if default specified. Traditional Chinese publisher of the application.\n");
        printf("  -i/--icon: PNG file to use as an icon.\n");
        printf("  -o/--output: File to output the created SMDH/ICN to.\n");
        printf("  -r/--regions: Optional. Comma separated list of regions to lock the SMDH to.\n");
        printf("     Valid regions: regionfree, japan, northamerica, europe, australia, china, korea, taiwan.\n");
        printf("  -f/--flags: Optional. Flags to apply to the SMDH file.\n");
        printf("     Valid flags: visible, autoboot, allow3d, requireeula, autosave, extendedbanner, ratingrequired, savedata, recordusage, nosavebackups, new3ds.\n");
        printf("  -mmid/--matchmakerid: Optional. Match maker ID of the SMDH.\n");
        printf("  -mmbid/--matchmakerbitid: Optional. Match maker BIT ID of the SMDH.\n");
        printf("  -ev/--eulaversion: Optional. Version of the EULA required to be accepted before launching.\n");
        printf("  -obf/--optimalbannerframe: Optional. Optimal frame of the accompanying banner.\n");
        printf("  -spid/--streetpassid: Optional. Streetpass ID of the SMDH.\n");
        printf("  -cer/--cero: Optional. CERO rating number (0-255).\n");
        printf("  -er/--esrb: Optional. ESRB rating number (0-255).\n");
        printf("  -ur/--usk: Optional. USK rating number (0-255).\n");
        printf("  -pgr/--pegigen: Optional. PEGI GEN rating number (0-255).\n");
        printf("  -ppr/--pegiptr: Optional. PEGI PTR rating number (0-255).\n");
        printf("  -pbr/--pegibbfc: Optional. PEGI BBFC rating number (0-255).\n");
        printf("  -cr/--cob: Optional. COB rating number (0-255).\n");
        printf("  -gr/--grb: Optional. GR rating number (0-255).\n");
        printf("  -cgr/--cgsrr: Optional. CGSRR rating number (0-255).\n");
    } else if(command.compare("makecwav") == 0) {
        printf("makecwav - Creates a CWAV file from a WAV.\n");
        printf("  -i/--input: WAV file to convert.\n");
        printf("  -o/--output: File to output the created CWAV to.\n");
        printf("  -l/--loop: Optional. Whether or not the audio should loop (false/true).\n");
        printf("  -s/--loopstartframe: Optional. Sample frame to return to when looping.\n");
        printf("  -f/--loopendframe: Optional. Sample frame to loop at.\n");
    } else if(command.compare("lz11") == 0) {
        printf("lz11 - Compresses a file with LZ11.\n");
        printf("  -i/--input: File to compress.\n");
        printf("  -o/--output: File to output the compressed data to.\n");
    }
}

static void cmd_print_commands() {
    printf("Available commands:\n");
    cmd_print_info("makebanner");
    cmd_print_info("makesmdh");
    cmd_print_info("makecwav");
    cmd_print_info("lz11");
}

static void cmd_print_usage(const std::string& executedFrom) {
    printf("bannertool v%d.%d.%d\n", VERSION_MAJOR, VERSION_MINOR, VERSION_MICRO);
    printf("Usage: %s <command> <args>\n", executedFrom.c_str());
    cmd_print_commands();
}

static void cmd_missing_args(const std::string& command) {
    printf("Missing arguments for command \"%s\".\n", command.c_str());
    cmd_print_info(command);
}

static void cmd_invalid_arg(const std::string& argument, const std::string& command) {
    printf("Invalid value for argument \"%s\" in command \"%s\".\n", argument.c_str(), command.c_str());
    cmd_print_info(command);
}

static void cmd_invalid_command(const std::string& command) {
    printf("Invalid command \"%s\".\n", command.c_str());
    cmd_print_commands();
}

int cmd_process_command(int argc, char* argv[]) {
    if(argc < 2) {
        cmd_print_usage(argv[0]);
        return -1;
    }

    char* command = argv[1];
    std::map<std::string, std::string> args = cmd_get_args(argc, argv);
    if(strcmp(command, "makebanner") == 0) {
        const std::string audio = cmd_find_arg(args, "a", "audio", "");
        const std::string cwavFile = cmd_find_arg(args, "ca", "cwavaudio", "");
        const std::string output = cmd_find_arg(args, "o", "output", "");
        if((audio.empty() && cwavFile.empty()) || output.empty()) {
            cmd_missing_args(command);
            return -1;
        }

        static const char* imageShortArgs[CBMD_NUM_CGFXS] = {"i", "eei", "efi", "egi", "eii", "esi", "edi", "epi", "eri", "jji", "uei", "ufi", "usi", "upi"};
        static const char* imageLongArgs[CBMD_NUM_CGFXS] = {"image", "eurenglishimage", "eurfrenchimage", "eurgermanimage", "euritalianimage", "eurspanishimage", "eurdutchimage", "eurportugueseimage", "eurrussianimage", "jpnjapaneseimage", "usaenglishimage", "usafrenchimage", "usaspanishimage", "usaportugueseimage"};
        static const char* cgfxImageShortArgs[CBMD_NUM_CGFXS] = {"ci", "eeci", "efci", "egci", "eici", "esci", "edci", "epci", "erci", "jjci", "ueci", "ufci", "usci", "upci"};
        static const char* cgfxImageLongArgs[CBMD_NUM_CGFXS] = {"cgfximage", "eurenglishcgfximage", "eurfrenchcgfximage", "eurgermancgfximage", "euritaliancgfximage", "eurspanishcgfximage", "eurdutchcgfximage", "eurportuguesecgfximage", "eurrussiancgfximage", "jpnjapanesecgfximage", "usaenglishcgfximage", "usafrenchcgfximage", "usaspanishcgfximage", "usaportuguesecgfximage"};

        std::string images[CBMD_NUM_CGFXS] = {""};
        std::string cgfxFiles[CBMD_NUM_CGFXS] = {""};

        bool found = false;

        for(u32 i = 0; i < CBMD_NUM_CGFXS; i++) {
            images[i] = cmd_find_arg(args, imageShortArgs[i], imageLongArgs[i], "");
            cgfxFiles[i] = cmd_find_arg(args, cgfxImageShortArgs[i], cgfxImageLongArgs[i], "");

            found = found || !images[i].empty() || !cgfxFiles[i].empty();
        }

        if(!found) {
            cmd_missing_args(command);
            return -1;
        }

        return cmd_make_banner(images, audio, cgfxFiles, cwavFile, output);
    } else if(strcmp(command, "makesmdh") == 0) {
        const std::string icon = cmd_find_arg(args, "i", "icon", "");
        const std::string output = cmd_find_arg(args, "o", "output", "");
        if(icon.empty() || output.empty()) {
            cmd_missing_args(command);
            return -1;
        }

        SMDH smdh;
        memset(&smdh, 0, sizeof(smdh));

        memcpy(smdh.magic, SMDH_MAGIC, sizeof(smdh.magic));

        static const char* shortTitleShortArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"js", "es", "fs", "gs", "is", "ss", "scs", "ks", "ds", "ps", "rs", "tcs"};
        static const char* shortTitleLongArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"japaneseshorttitle", "englishshorttitle", "frenchshorttitle", "germanshorttitle", "italianshorttitle", "spanishshorttitle", "simplifiedchineseshorttitle", "koreanshorttitle", "dutchshorttitle", "portugueseshorttitle", "russianshorttitle", "traditionalchineseshorttitle"};
        static const char* longTitleShortArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"jl", "el", "fl", "gl", "il", "sl", "scl", "kl", "dl", "pl", "rl", "tcl"};
        static const char* longTitleLongArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"japaneselongtitle", "englishlongtitle", "frenchlongtitle", "germanlongtitle", "italianlongtitle", "spanishlongtitle", "simplifiedchineselongtitle", "koreanlongtitle", "dutchlongtitle", "portugueselongtitle", "russianlongtitle", "traditionalchineselongtitle"};
        static const char* publisherShortArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"jp", "ep", "fp", "gp", "ip", "sp", "scp", "kp", "dp", "pp", "rp", "tcp"};
        static const char* publisherLongArgs[SMDH_NUM_VALID_LANGUAGE_SLOTS] = {"japanesepublisher", "englishpublisher", "frenchpublisher", "germanpublisher", "italianpublisher", "spanishpublisher", "simplifiedchinesepublisher", "koreanpublisher", "dutchpublisher", "portuguesepublisher", "russianpublisher", "traditionalchinesepublisher"};

        const std::string shortTitle = cmd_find_arg(args, "s", "shorttitle", "");
        const std::string longTitle = cmd_find_arg(args, "l", "longtitle", "");
        const std::string publisher = cmd_find_arg(args, "p", "publisher", "");

        bool shortTitleFound = false;
        bool longTitleFound = false;
        bool publisherFound = false;

        for(u32 i = 0; i < SMDH_NUM_LANGUAGE_SLOTS; i++) {
            std::string currShortTitle = shortTitle;
            std::string currLongTitle = longTitle;
            std::string currPublisher = publisher;

            if(i < SMDH_NUM_VALID_LANGUAGE_SLOTS) {
                currShortTitle = cmd_find_arg(args, shortTitleShortArgs[i], shortTitleLongArgs[i], shortTitle);
                currLongTitle = cmd_find_arg(args, longTitleShortArgs[i], longTitleLongArgs[i], longTitle);
                currPublisher = cmd_find_arg(args, publisherShortArgs[i], publisherLongArgs[i], publisher);
            }

            shortTitleFound = shortTitleFound || !currShortTitle.empty();
            longTitleFound = longTitleFound || !currLongTitle.empty();
            publisherFound = publisherFound || !currPublisher.empty();

            utf8_to_utf16(smdh.titles[i].shortTitle, currShortTitle, sizeof(smdh.titles[i].shortTitle) / 2);
            utf8_to_utf16(smdh.titles[i].longTitle, currLongTitle, sizeof(smdh.titles[i].longTitle) / 2);
            utf8_to_utf16(smdh.titles[i].publisher, currPublisher, sizeof(smdh.titles[i].publisher) / 2);
        }

        if(!shortTitleFound || !longTitleFound || !publisherFound) {
            cmd_missing_args(command);
            return -1;
        }

        std::vector<std::string> regions = cmd_parse_list(cmd_find_arg(args, "r", "regions", "regionfree"));
        for(std::vector<std::string>::iterator it = regions.begin(); it != regions.end(); it++) {
            const std::string region = *it;
            if(region.compare("regionfree") == 0) {
                smdh.settings.regionLock = SMDH_REGION_FREE;
                break;
            } else if(region.compare("japan") == 0) {
                smdh.settings.regionLock |= SMDH_REGION_JAPAN;
            } else if(region.compare("northamerica") == 0) {
                smdh.settings.regionLock |= SMDH_REGION_NORTH_AMERICA;
            } else if(region.compare("europe") == 0) {
                smdh.settings.regionLock |= SMDH_REGION_EUROPE;
            } else if(region.compare("australia") == 0) {
                smdh.settings.regionLock |= SMDH_REGION_AUSTRALIA;
            } else if(region.compare("china") == 0) {
                smdh.settings.regionLock |= SMDH_REGION_CHINA;
            } else if(region.compare("korea") == 0) {
                smdh.settings.regionLock |= SMDH_REGION_KOREA;
            } else if(region.compare("taiwan") == 0) {
                smdh.settings.regionLock |= SMDH_REGION_TAIWAN;
            } else {
                cmd_invalid_arg("regions", command);
                return -1;
            }
        }

        std::vector<std::string> flags = cmd_parse_list(cmd_find_arg(args, "f", "flags", "visible,allow3d,recordusage"));
        for(std::vector<std::string>::iterator it = flags.begin(); it != flags.end(); it++) {
            const std::string flag = *it;
            if(flag.compare("visible") == 0) {
                smdh.settings.flags |= SMDH_FLAG_VISIBLE;
            } else if(flag.compare("autoboot") == 0) {
                smdh.settings.flags |= SMDH_FLAG_AUTO_BOOT;
            } else if(flag.compare("allow3d") == 0) {
                smdh.settings.flags |= SMDH_FLAG_ALLOW_3D;
            } else if(flag.compare("requireeula") == 0) {
                smdh.settings.flags |= SMDH_FLAG_REQUIRE_EULA;
            } else if(flag.compare("autosave") == 0) {
                smdh.settings.flags |= SMDH_FLAG_AUTO_SAVE_ON_EXIT;
            } else if(flag.compare("extendedbanner") == 0) {
                smdh.settings.flags |= SMDH_FLAG_USE_EXTENDED_BANNER;
            } else if(flag.compare("ratingrequired") == 0) {
                smdh.settings.flags |= SMDH_FLAG_RATING_REQUIED;
            } else if(flag.compare("savedata") == 0) {
                smdh.settings.flags |= SMDH_FLAG_USE_SAVE_DATA;
            } else if(flag.compare("recordusage") == 0) {
                smdh.settings.flags |= SMDH_FLAG_RECORD_USAGE;
            } else if(flag.compare("nosavebackups") == 0) {
                smdh.settings.flags |= SMDH_FLAG_DISABLE_SAVE_BACKUPS;
            } else if(flag.compare("new3ds") == 0) {
                smdh.settings.flags |= SMDH_FLAG_NEW_3DS;
            } else {
                cmd_invalid_arg("flags", command);
                return -1;
            }
        }

        smdh.settings.matchMakerId = (u32) atoi(cmd_find_arg(args, "mmid", "matchmakerid", "0").c_str());
        smdh.settings.matchMakerBitId = (u64) atoll(cmd_find_arg(args, "mmbid", "matchmakerbitid", "0").c_str());

        smdh.settings.eulaVersion = (u16) atoi(cmd_find_arg(args, "ev", "eulaversion", "0").c_str());
        smdh.settings.optimalBannerFrame = (u32) atoi(cmd_find_arg(args, "obf", "optimalbannerframe", "0").c_str());
        smdh.settings.streetpassId = (u32) atoi(cmd_find_arg(args, "spid", "streetpassid", "0").c_str());

        smdh.settings.gameRatings[SMDH_RATING_CERO] = (u8) atoi(cmd_find_arg(args, "cer", "cero", "0").c_str());
        smdh.settings.gameRatings[SMDH_RATING_ESRB] = (u8) atoi(cmd_find_arg(args, "er", "esrb", "0").c_str());
        smdh.settings.gameRatings[SMDH_RATING_USK] = (u8) atoi(cmd_find_arg(args, "ur", "usk", "0").c_str());
        smdh.settings.gameRatings[SMDH_RATING_PEGI_GEN] = (u8) atoi(cmd_find_arg(args, "pgr", "pegigen", "0").c_str());
        smdh.settings.gameRatings[SMDH_RATING_PEGI_PTR] = (u8) atoi(cmd_find_arg(args, "ppr", "pegiptr", "0").c_str());
        smdh.settings.gameRatings[SMDH_RATING_PEGI_BBFC] = (u8) atoi(cmd_find_arg(args, "pbr", "pegibbfc", "0").c_str());
        smdh.settings.gameRatings[SMDH_RATING_COB] = (u8) atoi(cmd_find_arg(args, "cor", "cob", "0").c_str());
        smdh.settings.gameRatings[SMDH_RATING_GRB] = (u8) atoi(cmd_find_arg(args, "gr", "grb", "0").c_str());
        smdh.settings.gameRatings[SMDH_RATING_CGSRR] = (u8) atoi(cmd_find_arg(args, "cgr", "cgsrr", "0").c_str());

        return cmd_make_smdh(smdh, icon, output);
    } else if(strcmp(command, "makecwav") == 0) {
        const std::string input = cmd_find_arg(args, "i", "input", "");
        const std::string output = cmd_find_arg(args, "o", "output", "");
        std::string loop = cmd_find_arg(args, "l", "loop", "false");
        u32 loopStartFrame = (u32) atoi(cmd_find_arg(args, "s", "loopstartframe", "0").c_str());
        u32 loopEndFrame = (u32) atoi(cmd_find_arg(args, "e", "loopendframe", "0").c_str());
        if(input.empty() || output.empty()) {
            cmd_missing_args(command);
            return -1;
        }

        std::transform(loop.begin(), loop.end(), loop.begin(), (int (*)(int)) std::tolower);
        if(loop != "false" && loop != "true") {
            cmd_invalid_arg("loop", command);
            return -1;
        }

        return cmd_make_cwav(input, output, loop == "true", loopStartFrame, loopEndFrame);
    } else if(strcmp(command, "lz11") == 0) {
        const std::string input = cmd_find_arg(args, "i", "input", "");
        const std::string output = cmd_find_arg(args, "o", "output", "");
        if(input.empty() || output.empty()) {
            cmd_missing_args(command);
            return -1;
        }

        return cmd_lz11(input, output);
    } else {
        cmd_invalid_command(command);
        return -1;
    }
}
