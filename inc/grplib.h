#ifndef __GRPLIB_H
#define __GRPLIB_H

#pragma aux PageOffset "*";
extern unsigned int PageOffset;  // active screen page

#pragma aux erase_flag "*";
extern char erase_flag;  // active screen page

#pragma aux SCROFF "*";
void SCROFF();

#pragma aux SCRON "*";
void SCRON();

#pragma aux VRAMCLS "*";
void VRAMCLS();

#pragma aux wait_vrt "*";
void wait_vrt();

#pragma aux mode320X240 "*";
void mode320X240();

#pragma aux xmode "*";
void xmode();

#pragma aux clean "*";
void clean();

#pragma aux erase "*";
void erase();

#pragma aux set_palette "*" parm [esi];
void set_palette(unsigned char *pPalette);

#pragma aux gamma_correction "*" parm [edx] [esi] [edi];
void gamma_correction(unsigned char *correctionTable, unsigned char *inPalette, unsigned char *outPalette);

void Gamma(unsigned char *pal, int gammano);

#pragma aux showp "*";
void showp();

#pragma aux show_p "*";
void show_p();

#pragma aux put_pixel "*";
void put_pixel(unsigned int X, unsigned int Y, unsigned char color);

#pragma aux load_put_PCX "*";
void load_put_PCX(const char *path);

#pragma aux load_put_PCXX "*";
void load_put_PCXX(const char *path);

#pragma aux put_pic "*";
void put_pic();

#pragma aux put_string "*";
void put_string(unsigned int X, unsigned int Y, const char *text, unsigned char color);

#pragma aux decoding_PCX "*";
void decoding_PCX(const char *path);

#endif  // __GRPLIB_H
