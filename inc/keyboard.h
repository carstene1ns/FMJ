#ifndef __KEYBOARD_H
#define __KEYBOARD_H

#pragma aux keyboard "*";
extern unsigned char keyboard[128];
#pragma aux key_edge "*";
extern unsigned char key_edge[128];
#pragma aux key_string "*";
extern unsigned char key_string[4];
#pragma aux key_hit "*";
extern unsigned char key_hit;
#pragma aux extended_key "*";
extern unsigned char extended_key;

#pragma aux keyint_on "*";
extern void keyint_on();

#pragma aux keyint_off "*";
extern void keyint_off();

#pragma aux clear_key_buffer "*";
extern void clear_key_buffer();

#endif  // __KEYBOARD_H
