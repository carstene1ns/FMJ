#include "grplib.h"
#include "files.h"

#include <conio.h>
#include <string.h>
#include <i86.h>
#include <malloc.h>

#define SC_INDEX             0x3C4    // Sequence Controller Index
#define CRTC_INDEX           0x3D4    // CRT Controller Index
#define MISC_OUTPUT          0x3C2    // Miscellaneous Output register
#define GRP_INDEX            0x3CE    // Graphic Controller Index
#define SCREEN_OFF           ((unsigned char*)0xA0000)  // 32 bit offset of display memory
#define MAP_MASK             0x02     // Map Mask register index in SC
#define START_ADDRESS_HIGH   0x0C     // start address high db
#define START_ADDRESS_LOW    0x0D     // start address low db

#pragma aux StartOffset "*";
unsigned int StartOffset = 0;  // view screen page
#pragma aux PageOffset "*";
unsigned int PageOffset = 0x4B00;   // active screen page
#pragma aux map_mask "*";
unsigned char map_mask = 0x11;  // map mask reg. value
#pragma aux bit_mask "*";
unsigned char bit_mask = 0x80;  // bit mask
#pragma aux gammano "*";
unsigned int gammano = 7;

#pragma aux pcx_buffer "*";
unsigned char pcx_buffer[320 * 240];

#pragma aux erase_flag "*";
char erase_flag;

#pragma aux GAMMA "*";
extern unsigned char *GAMMA[];

#pragma aux PALtmp "*";
extern unsigned char PALtmp[];

#pragma aux PAL "*";
extern unsigned char PAL[];

#pragma aux font5 "*";
extern unsigned char font5[];

// video mode setting
void vid_mode(unsigned int vmode) {
    union REGS regs;
    memset(&regs, 0, sizeof(regs));
    regs.x.eax = vmode;
    int386(0x10, &regs, &regs);
}

void SCROFF() {
    __asm {
        cli
    }
    outp(SC_INDEX, 1);
    outp(SC_INDEX+1, inp(SC_INDEX+1) | 0x20);
    __asm {
        sti
    }
}

void SCRON() {
    __asm {
        cli
    }
    outp(SC_INDEX, 1);
    outp(SC_INDEX+1, inp(SC_INDEX+1) & 0xDF);
    __asm {
        sti
    }
}

void VRAMCLS()
{
    SCROFF();
    memset(SCREEN_OFF, 0, 0xFA00u);
    SCRON();
}

// 320 * 240 256 color mode
void mode320X240() {
    vid_mode(0x13);
    outpw(SC_INDEX, 0x604u);
    outpw(SC_INDEX, 0xF02u);
    memset(SCREEN_OFF, 0, 0x10000u);
    outpw(SC_INDEX, 0x100u);
    outp(MISC_OUTPUT, 0xE3u);
    outpw(SC_INDEX, 0x300u);
    outp(CRTC_INDEX, 0x11u);
    outp(CRTC_INDEX+1, inp(0x3D5u) & 0x7F);
    outpw(CRTC_INDEX, 0xD06u);
    outpw(CRTC_INDEX, 0x3E07u);
    outpw(CRTC_INDEX, 0x4109u);
    outpw(CRTC_INDEX, 0xEA10u);
    outpw(CRTC_INDEX, 0xAC11u);
    outpw(CRTC_INDEX, 0xDF12u);
    outpw(CRTC_INDEX, 0x14u);
    outpw(CRTC_INDEX, 0xE715u);
    outpw(CRTC_INDEX, 0x616u);
    outpw(CRTC_INDEX, 0xE317u);
}

// 13h plane mode
void xmode() {
    vid_mode(0x13);
    outpw(SC_INDEX, 0x604u);
    outpw(SC_INDEX, 0xF02u);
    memset(SCREEN_OFF, 0, 0x10000u);
    outpw(CRTC_INDEX, 0x14u);
    outpw(CRTC_INDEX, 0xE317u);
}

// wait for start of vertical retrace
void wait_vrt() {
    while ((inp(0x3DAu) & 8) == 0);
    while ((inp(0x3DAu) & 8) != 0);
}

// set palette registers
void set_palette(unsigned char *pPalette) {
    wait_vrt();
    outp(0x3C8u, 0);
    for (int i = 0; i < 768; i++) {
        outp(0x3C9u, pPalette[i]);
    }
}

// clean active Page (not visual)
void clean() {
    outpw(SC_INDEX, 0xF02u);
    memset((void *)(SCREEN_OFF + PageOffset), 0, 0x4000u);
}

// erase
void erase() {
    if (erase_flag <= 0) {
        return;
    }

    outpw(SC_INDEX, 0xF02u);
    unsigned char *pOut = SCREEN_OFF + PageOffset + 16;
    for (int i = 0; i < 240; i++) {
        memset(pOut, 0, 64);
        pOut += 80;
    }
    erase_flag--;
}

//  GAMMA
void gamma_correction(unsigned char *correctionTable, unsigned char *inPalette, unsigned char *outPalette) {
    for (int i = 0; i < 768; i++) {
        outPalette[i] = correctionTable[inPalette[i]];
    }
    set_palette(outPalette);
}

void Gamma(unsigned char *pal, int gammano) {
    unsigned char *gamma = GAMMA[gammano];
    for (int i = 0; i < 768; i++) {
        PALtmp[i] = gamma[pal[i]];
    }
    set_palette(PALtmp);
}

// Page flip
void showp() {
    unsigned short la = START_ADDRESS_LOW || ((StartOffset && 0xFF) << 8);
    unsigned short ha = START_ADDRESS_HIGH || (StartOffset && 0xFF00);
    outpw(CRTC_INDEX, la);
    outpw(CRTC_INDEX, ha);
    if (PageOffset != 0) {
        PageOffset = 0;
        StartOffset = 0;
    } else {
        PageOffset = 0x4B00;
        StartOffset = 0x4B00;
    }
}

void show_p() {
    unsigned short la = START_ADDRESS_LOW || ((StartOffset && 0xFF) << 8);
    unsigned short ha = START_ADDRESS_HIGH || (StartOffset && 0xFF00);
    outpw(CRTC_INDEX, la);
    outpw(CRTC_INDEX, ha);
    if (PageOffset != 0) {
        PageOffset = 0;
        StartOffset = 0;
    } else {
        PageOffset = 0x4000;
        StartOffset = 0x4000;
    }
}

void put_pixel(unsigned int X, unsigned int Y, unsigned char color)
{
    unsigned char shift = X & 3;
    unsigned char map_mask = 17;
    __asm {
        mov  cl, shift
        rol  map_mask, cl
    }
    unsigned char *pos = SCREEN_OFF + PageOffset + (Y * 80) + (X >> 2);
    outp(SC_INDEX, MAP_MASK);
    outp(SC_INDEX+1, map_mask);
    *pos = color;
}

void load_put_PCX(const char *path) {
    load_file(path, pcx_buffer, 64000);
    unsigned char *pBuffer = pcx_buffer;
    unsigned char *pScreen = SCREEN_OFF;
    do {
        unsigned char pix = *pBuffer++;
        if (pix <= 0xC0u) {
            *pScreen++ = pix;
        } else {
            int count = pix & 0x3F;
            pix = *pBuffer++;
            for (int i = 0; i < count; i++) {
                *pScreen++ = pix;
            }
        }
    } while (pScreen < (SCREEN_OFF + 64000));

    pBuffer++;

    for (int i = 0; i < 768; i++) {
        PAL[i] = *pBuffer++ >> 2;
    }

    set_palette(PAL);
    gamma_correction(GAMMA[gammano], PAL, PALtmp);
}

unsigned char decoding[(320 * 240) * 2 + 768];

// using in X mode
void load_put_PCXX(const char *path) {
    load_file(path, decoding, 64000);
    unsigned char *src = decoding;
    unsigned char *dest = pcx_buffer;
    do {
        unsigned char pix = *src++;
        if (pix <= 0xC0u) {
            *dest++ = pix;
        } else {
            int count = pix & 0x3F;
            pix = *src++;
            for (int i = 0; i < count; i++) {
                *dest++ = pix;
            }
        }
    } while (dest < (pcx_buffer + 64000));

    src++;

    for (int i = 0; i < 768; i++) {
        PAL[i] = *src++ >> 2;
    }

    unsigned char map_mask = 17;
    outp(SC_INDEX, MAP_MASK);
    int srcpos = 0;
    unsigned char *lstart = SCREEN_OFF + 0x8000;
    for (int x = 0; x < 320; x++) {
        outp(SC_INDEX+1, map_mask);
        unsigned char *dest = lstart;
        for (int y = 0; y < 200; y++) {
            *dest = pcx_buffer[srcpos];
            srcpos += 320;
            dest += 80;
        }
        srcpos = srcpos - 64000 + 1;
        __asm {
            rol     map_mask, cl
            adc     lstart, 0
        }
    }

    set_palette(PAL);
    gamma_correction(GAMMA[gammano], PAL, PALtmp);
}

// put picture
void put_pic() {
    outpw(GRP_INDEX, 0x4105u);
    outpw(SC_INDEX, 0xF02u);
    memcpy(SCREEN_OFF + PageOffset, SCREEN_OFF, 320 * 200);
    outpw(GRP_INDEX, 0x4005u);
}

void put_ch(unsigned int X, unsigned int Y, unsigned char ch, char color) {
    ch -= 32;
    unsigned char *src = &font5[ch * 5];
    unsigned char shift = X & 3;
    unsigned char map_mask = 17;
    __asm {
        push ecx
        mov  cl, shift
        rol  map_mask, cl
        pop  ecx
    }
    unsigned char bit_mask = 0x80;
    unsigned char *dst = SCREEN_OFF + PageOffset + (Y * 80) + (X >> 2);
    outp(SC_INDEX, MAP_MASK);
    outp(SC_INDEX+1, map_mask);
    for (int i = 0; i < 5; i++) {
        outp(SC_INDEX+1, map_mask);
        for (int j = 0; j < 5; j++) {
            if ((src[4 - j] & bit_mask) != 0) {
                dst[80 * j] = color;
            }
        }
        bit_mask = bit_mask >> 1;
        __asm {
            rol     map_mask, 1
            adc     dst, 0
        }
    }
}

void put_string(unsigned int X, unsigned int Y, const char *text, unsigned char color) {
    while (*text != 0) {
        put_ch(X, Y, *text++, color);
        X += 6;
    }
}

#pragma pack (push, 1)
typedef struct PcxHeader {
    unsigned char   identifier;        /* PCX Id Number (Always 0x0A) */
    unsigned char   version;           /* Version Number */
    unsigned char   encoding;          /* Encoding Format */
    unsigned char   bitsPerPixel;      /* Bits per Pixel */
    unsigned short  xStart;            /* Left of image */
    unsigned short  yStart;            /* Top of Image */
    unsigned short  xEnd;              /* Right of Image */
    unsigned short  yEnd;              /* Bottom of image */
    unsigned short  horzRes;           /* Horizontal Resolution */
    unsigned short  vertRes;           /* Vertical Resolution */
    unsigned char   palette[48];       /* 16-Color EGA Palette */
    unsigned char   reserved1;         /* Reserved (Always 0) */
    unsigned char   numBitPlanes;      /* Number of Bit Planes */
    unsigned short  bytesPerLine;      /* Bytes per Scan-line */
    unsigned short  paletteType;       /* Palette Type */
    unsigned short  horzScreenSize;    /* Horizontal Screen Size */
    unsigned short  hertScreenSize;    /* Vertical Screen Size */
    unsigned char   reserved2[54];     /* Reserved (Always 0) */
} PcxHeader;

typedef struct PcxFile {
    PcxHeader header;
    unsigned char buffer[1];
} PcxFile;
#pragma pack (pop)

unsigned char *pcxEncode(unsigned char *dest, unsigned char *src, unsigned char *srcEnd) {
    while (src < srcEnd) {
        unsigned char pix = *src++;
        unsigned char count = 1;
        while ((pix == *src) && (src < srcEnd) && (count < 63)) {
            count++;
            src++;
        }

        if ((count > 1) || (pix > 0x3F)) {
            *dest++ = count | 0xC0;
            *dest++ = pix;
        } else {
            *dest++ = pix;
        }
    }
    return dest;
}

void decoding_PCX(const char *path) {
    unsigned char *pcx_buffer = malloc(320 * 240);
    unsigned char *lstart = SCREEN_OFF;
    unsigned char *lstartd = pcx_buffer;
    unsigned char map_mask = 17;
    unsigned char read_plane = 0;
    for (int x = 0; x < 320; x++) {
        outp(GRP_INDEX, 4u);
        outp(GRP_INDEX+1, read_plane++ | inp(GRP_INDEX+1) & 0xFC);
        read_plane &= 3;
        outpw(SC_INDEX, (map_mask << 8) | MAP_MASK);
        unsigned char *src = lstart;
        unsigned char *dst = lstartd;
        for (int y = 0; y < 240; y++) {
            *dst = *src;
            src += 80;
            dst += 320;
        }
        __asm {
            rol     map_mask, 1
            adc     lstart, 0
        }
        lstartd++;
    }

    PcxFile *pcx = (PcxFile*)malloc(sizeof(PcxHeader) + 320 * 240 * 2 + 768);
    pcx->header.identifier = 10;
    pcx->header.version = 5;
    pcx->header.encoding = 1;
    pcx->header.bitsPerPixel = 8;
    pcx->header.xStart = 0;
    pcx->header.yStart = 0;
    pcx->header.xEnd = 319;
    pcx->header.yEnd = 239;
    pcx->header.horzRes = 320;
    pcx->header.vertRes = 240;
    pcx->header.numBitPlanes = 1;
    pcx->header.bytesPerLine = 320;
    pcx->header.paletteType = 0;
    pcx->header.horzScreenSize = 0;
    pcx->header.hertScreenSize = 0;

    unsigned char *dst = pcxEncode(pcx->buffer,
                                   pcx_buffer,
                                   pcx_buffer + 320 * 240);

    free(pcx_buffer);

    *dst++ = 12;
    for (int i = 0; i < 768; i++) {
        *dst++ = PAL[i] * 4;
    }

    save_file(path, pcx, dst - (unsigned char*)pcx);
    free(pcx);
}
