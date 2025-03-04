#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <i86.h>
#include <conio.h>

#include "keys.h"
#include "files.h"
#include "fli.h"

/*---------------------------------------------------
   Global Variables and Definitions
---------------------------------------------------*/

/* Video memory (mode 13h: 320x200, 64000 bytes) */
static unsigned char *vram = (unsigned char *)0xA0000;

/* pcx_buffer: used as LOAD_MEMORY buffer */
#pragma aux pcx_buffer "*";
extern char pcx_buffer[];
#define LOAD_MEMORY (pcx_buffer)
//unsigned char LOAD_MEMORY[0x10000];  /* 64KB buffer */

uint16_t FRAME_CNT = 0;

extern volatile uint32_t TimerTicks;
extern int GetKey(void);
extern int BrightAdjust;
extern void Gamma(unsigned char *pal, int gammano);

/*---------------------------------------------------
   Forward Declarations of Functions
---------------------------------------------------*/
static void handle_RGB(unsigned char *chunk_ptr);
static void handle_COPY(unsigned char *chunk_ptr);
static void handle_BLK(void);
static void handle_LC(unsigned char *chunk_ptr);
static void handle_RLE(unsigned char *chunk_ptr);
static void set_palette(uint8_t start, uint32_t count, unsigned char *chunk_ptr);

/* CHUNKS_RT table corresponding to chunk types 11,12,13,14,15,16 */
enum CHUNKS_RT {
  FLI_RGB = 11,
  FLI_LC,
  FLI_BLK,
  FLI_COPY1,
  FLI_RLE,
  FLI_COPY2
};

/* Current Palette (R, G, B triplets for 256 colors) */
static unsigned char palette[256 * 3];

/*---------------------------------------------------
   Graphics and Animation Procedures
---------------------------------------------------*/

/*
   fli_file_run
   EAX : file name pointer (passed as parameter)
   Function: Runs the FLI file animation.
---------------------------------------------------*/
int fli_file_run(const char *filename) {
    FRAME_CNT = 0;

    /* Open file */
    FILE *handle = open_file(filename, "rb");
    if (!handle) {
        return -1;
    }

    /* Read FLI header: 128 bytes */
    if (read_file(handle, LOAD_MEMORY, 128) != 128) {
        close_file(handle);
        return -1;
    }

    /* ptr at start of FLI header */
    unsigned char *ptr = LOAD_MEMORY;
    /* Check FLI header ID at offset 4 */
    uint16_t id = *(uint16_t *)(ptr + 4);
    if (id != 0xAF11) {
        close_file(handle);
        return -1;
    }

    /* Read number of frames from offset 6 */
    uint16_t total_frames = *(uint16_t *)(ptr + 6);
    uint16_t frames_remaining = total_frames;

    /* Reset _TimerTicks */
    TimerTicks = 0;

    while (frames_remaining) {
        //fprintf(stderr, "%d frames remaining\n", frames_remaining);
        /* Read frame header: 16 bytes */
        if (read_file(handle, LOAD_MEMORY, 16) != 16) {
            close_file(handle);
            return -1;
        }
        ptr = LOAD_MEMORY;
        uint16_t frame_id = *(uint16_t *)(ptr + 4);
        /* Check frame header ID */
        if (frame_id != 0xF1FA) {
            close_file(handle);
            return -1;
        }

        /* Read number of chunks in frame from offset 6 */
        uint16_t chunks = *(uint16_t *)(ptr + 6);
        if (chunks != 0) {
            uint16_t num_chunks = chunks;

            /* Total bytes in frame = total bytes field at ptr minus header (16 bytes) */
            uint32_t total_frame_bytes = *(uint32_t *)ptr;
            uint32_t frame_data_size = total_frame_bytes - 16;
            if (read_file(handle, LOAD_MEMORY, frame_data_size) != frame_data_size) {
                close_file(handle);
                return -1;
            }
            unsigned char *chunk_ptr = LOAD_MEMORY;

            while (num_chunks--) {
                //fprintf(stderr, "%d chunks remaining\n", num_chunks+1);
                /* Save current chunk pointer */
                unsigned char *current_chunk = chunk_ptr;

                /* Read chunk type from offset 4 of current chunk */
                uint16_t chunk_type = *(uint16_t *)(current_chunk + 4);

                /* Check valid chunk type */
                switch(chunk_type) {
                case FLI_RGB:
                    handle_RGB(current_chunk + 6);
                    break;

                case FLI_LC:
                    handle_LC(current_chunk + 6);
                    break;

                case FLI_BLK:
                    handle_BLK();
                    break;

                case FLI_COPY1:
                case FLI_COPY2:
                    handle_COPY(current_chunk + 6);
                    break;

                case FLI_RLE:
                    handle_RLE(current_chunk + 6);
                    break;

                default:
                    fprintf(stderr, "Unknown FLI chunk %d!\n", chunk_type);
                    close_file(handle);
                    return -1;

                    break;
                }

                /* Update chunk pointer: add chunk length read from current chunk */
                uint16_t chunk_length = *(uint16_t *)current_chunk;
                chunk_ptr += chunk_length;
            }
        }

        /* End of frame: Check for key hits */
        if(kbhit()) {
            int key = GetKey();
            if (key == ESC) {
                close_file(handle);
                return 1;
            }
            if(key == SPACE) {
                close_file(handle);
                return 0;
            }
        }

        /* Time check: wait */
        while (TimerTicks <= 3) {
            delay(10);
        }
        TimerTicks = 0;

        /* Increment frame counter */
        FRAME_CNT++;
        frames_remaining--;
    }

    /* End of Play: close file */
    close_file(handle);

    return 0;
}

/*
   handle_RGB:
   Processes an FLI_RGB chunk.
   Calls handle_BLK then processes packets to update the palette.
---------------------------------------------------*/
static void handle_RGB(unsigned char *chunk_ptr) {
    /* Call handle_BLK to clear video memory */
    handle_BLK();

    /* Read NUMBER OF PACKETS (2 bytes) */
    uint16_t num_packets = *(uint16_t *)chunk_ptr;
    chunk_ptr += 2;
    uint8_t start = 0;

    while (num_packets--) {
        //fprintf(stderr, "[rgb]%d packets remaining\n", num_packets+1);
        /* Add number of colors to skip from current packet */
        start += *chunk_ptr;
        /* Get number of colors changed */
        uint8_t colors = *(chunk_ptr + 1);
        chunk_ptr += 2;
        uint32_t count;
        if (colors != 0) {
            count = colors;
        } else {
            count = 256;
        }
        set_palette(start, count, chunk_ptr);
        /* Advance palette data pointer (3 bytes per color) */
        chunk_ptr += count * 3;
        /* Skip used colors */
        start += colors;
    }

    /* apply whole gamma corrected palette */
    Gamma(palette, BrightAdjust);
}

/*
   handle_COPY:
   Copies 64000 bytes (video image) from the chunk to VRAM.
---------------------------------------------------*/
static void handle_COPY(unsigned char *chunk_ptr) {
    memcpy(vram, (void *)chunk_ptr, 64000);
}

/*
   handle_BLK:
   Clears the video memory (VRAM) by setting it to zero.
---------------------------------------------------*/
static void handle_BLK() {
    memset(vram, 0, 64000);
}

/*
   handle_LC:
   Processes an FLI_LC chunk (line compression).
   This routine interprets a custom compression format and renders lines
   into the video memory (vram).
---------------------------------------------------*/
static void handle_LC(unsigned char *chunk_ptr) {
    /* read line offset from chunk_ptr and advance by 2 */
    uint16_t lineofs = *(uint16_t *)chunk_ptr;
    chunk_ptr += 2;
    /* Multiply by 320 (line width) and add to destination base */
    uint32_t dest_offset = lineofs * 320;
    /* read number of lines */
    uint16_t lines = *(uint16_t *)chunk_ptr;
    chunk_ptr += 2;
    uint32_t base_ofs = dest_offset;

    while (lines--) {
        //fprintf(stderr, "[lc]%d lines remaining\n", lines+1);
        /* Save current destination offset for this line */
        uint32_t line_dest_offset = base_ofs;
        /* Read packet count for this line */
        uint8_t packets = *chunk_ptr++;
        if (packets == 0) {
            /* End of line if packet count is zero */
            base_ofs += 320;
            continue;
        }
        while (packets--) {
            //fprintf(stderr, "[lc]%d packets remaining\n", packets+1);
            /* read a skip value and add to destination pointer */
            uint8_t skip = *chunk_ptr++;
            line_dest_offset += skip;
            /* read a count value */
            int8_t count = (int8_t)*chunk_ptr++;
            uint8_t n;
            if (count >= 0) {
                /* Positive count: copy count bytes from chunk_ptr to destination */
                n = (uint8_t)count;
                memcpy(&vram[line_dest_offset], chunk_ptr, n);
                chunk_ptr += n;
            } else {
                /* Negative count: fill count (-count) bytes with a single value */
                n = (uint8_t)(-count);
                uint8_t value = *chunk_ptr++;
                memset(&vram[line_dest_offset], value, n);
            }
            line_dest_offset += n;
        }
        base_ofs += 320;  /* Move to the next line */
    }
}

/*
   handle_RLE:
   Processes an FLI_RLE chunk (run-length encoding).
   This routine decodes RLE-compressed data into the video memory.
---------------------------------------------------*/
static void handle_RLE(unsigned char *chunk_ptr) {
    /* Set destination pointer to video memory */
    unsigned char *dest_base = vram;
    /* Number of lines */
    int lines = 200;
    uint32_t dest_offset = 0;

    while (lines--) {
        //fprintf(stderr, "[rle]%d lines remaining\n", lines+1);
        unsigned char *line_dest = dest_base + dest_offset;
        /* Read packet count for this line */
        uint8_t packets = *chunk_ptr++;
        while (packets--) {
            //fprintf(stderr, "[rle]%d packets remaining\n", packets+1);
            int8_t code = (int8_t)*chunk_ptr++;
            if (code < 0) {
                /* Negative: copy -code bytes from source to destination */
                uint8_t count = (uint8_t)(-code);
                memcpy(line_dest, chunk_ptr, count);
                chunk_ptr += count;
                line_dest += count;
            } else {
                /* Non-negative: fill next code bytes with the following byte */
                uint8_t count = (uint8_t)code;
                uint8_t value = *chunk_ptr++;
                memset(line_dest, value, count);
                line_dest += count;
            }
        }
        dest_offset += 320;
    }
}

/*
   set_palette:
   Sets color palette.
   Parameters:
      start: starting palette index
      count: number of palette entries to update (each entry has 3 bytes)
      chunk_ptr: pointer to palette data.
---------------------------------------------------*/
static void set_palette(uint8_t start, uint32_t count, unsigned char *chunk_ptr) {
    int pal_index = start;
    /* Multiply count by 3 because each palette entry has 3 bytes */
    uint32_t total_bytes = count * 3;

    for (uint32_t i = 0; i < total_bytes; i++) {
        unsigned char data = chunk_ptr[i];

        palette[pal_index * 3 + (i % 3)] = data;
        if ((i % 3) == 2) {
            pal_index++;
        }
    }
}
