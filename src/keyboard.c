#include <i86.h>
#include <dos.h>
#include <string.h>
#include <conio.h>

#include "keyboard.h"

void (__interrupt __far *prev_int_09)() = NULL;
void __interrupt __far newint09();
unsigned char keyboard[128] = { 0 };
unsigned char key_edge[128] = { 0 };
unsigned char key_string[4] = { 0 };
unsigned char key_hit = 0;
unsigned char extended_key = 0;

// save orignal key int. handler & set new handler
void keyint_on() {
    prev_int_09 = _dos_getvect(0x09);
    _dos_setvect(0x09, newint09);
}

// restore original key-int. vector
void keyint_off() {
   if (prev_int_09 == NULL) { return; }
   _dos_setvect(0x09, prev_int_09);
}

void clear_key_buffer() {
    memset(keyboard, 0, sizeof(keyboard));
    memset(key_edge, 0, sizeof(key_edge));
    memset(key_string, 0, sizeof(key_string));
    key_hit = 0;
}

void __interrupt __far newint09() {
    _enable();
    unsigned char scan = inp(0x60);
    if (scan > 127) {
        if (scan == 224) {
            extended_key = 1;
        } else {
            key_hit = 0;
            unsigned char key = scan & 0x7F;
            if ((extended_key != 1) || (extended_key = 0, key != 42) && (key != 54)) {
                keyboard[key] = 0;
            }
        }
    } else {
        unsigned char key = scan & 0x7F;
        if ((extended_key != 1) || (extended_key = 0, key != 42) && (key != 54)) {
            key_hit = key;
            if (keyboard[key] == 0) {
                key_edge[key] = 1;
            }
            keyboard[key] = 1;
        }
    }
    outp(0x20u, 0x20u);
}
