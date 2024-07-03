#include "util.h"
#include "hardware.h"

static struct FrameBuffer* FB = 0;

static unsigned bg_color = COLOR_BLACK;
static unsigned fg_color = COLOR_WHITE;
static unsigned fb_y = 0, fb_x = 0;

#define FILL_COLOR(px, color) {\
    (px)->r = (color) >> 16 & 0xFF;\
    (px)->g = (color) >> 8 & 0xFF;\
    (px)->b = (color) & 0xFF;\
}

#define GET_FONT_PX(fnt, x, y) ((fnt)[(y)] >> (FONT_WIDTH - 1 - (x)) & 1)

void init_frame_buffer(struct FrameBuffer* fb) {
    FB = fb;
    fb_x = 0;
    fb_y = 0;
    for (unsigned int w = 0; w < FB->width; w++) {
        for (unsigned int h = 0; h < FB->height; h++) {
            struct Pixel* px = &FB->base[h * FB->width + w];
            FILL_COLOR(px, bg_color);
        }
    }
}

void set_bg_color(unsigned int color, unsigned int *old) {
    if (old) *old = bg_color;
    bg_color = color;
}

void set_fg_color(unsigned int color, unsigned int *old) {
    if (old) *old = fg_color;
    fg_color = color;
}

static void shift_one_line() {
    for (unsigned h = 0; h < FB->height; h++) {
        unsigned copied_from = h + FONT_HEIGHT;
        if (copied_from >= FB->height) {
            // If a pixel to be copied is out of range, fill it with the background color.
            for (unsigned w = 0; w < FB->width; w++) {
                struct Pixel* px = &FB->base[h * FB->width + w];
                FILL_COLOR(px, bg_color);
            }
        }
        else {
            for (unsigned w = 0; w < FB->width; w++) {
                struct Pixel* original_px = &FB->base[copied_from * FB->width + w];
                struct Pixel* px = &FB->base[h * FB->width + w];
                px->r = original_px->r;
                px->g = original_px->g;
                px->b = original_px->b;
            }
        }
    }
    if (fb_y) fb_y--;
    else fb_y = 0;
}

static void putc(char c) {
    // To protect the system from reading invalid addresses.
    if (c < 0) return;

    if (c == '\n') {
        fb_x = 0;
        fb_y++;

        if ((fb_y + 1) * FONT_HEIGHT > FB->height)
            shift_one_line();
    }
    else {
        if ((fb_x + 1) * FONT_WIDTH > FB->width) {
            fb_x = 0;
            fb_y++;
        }
        // Maybe the font is TOOOOOOOOOOOO LARGE that it cannot be shown to the screen.
        if (FONT_HEIGHT >= FB->height) return;

        if ((fb_y + 1) * FONT_HEIGHT > FB->height)
            shift_one_line();

        // Since 0x00 <= c <= 0x7f, the cast is valid.
        unsigned char* fnt = font[(unsigned char)c];
        for (int x = 0; x < FONT_WIDTH; x++) {
            for (int y = 0; y < FONT_HEIGHT; y++) {
                struct Pixel* px = &FB->base[(fb_y * FONT_HEIGHT + y) * FB->width + (fb_x * FONT_WIDTH + x)];
                FILL_COLOR(px, fg_color * GET_FONT_PX(fnt, x, y));
            }
        }
        fb_x++;
    }
}

void puts_n(char* str) {
    while (*str) {
        putc(*str);
        str++;
    }
}

void puts(char* str) {
    puts_n(str);
    putc('\n');
}

void puth(unsigned long long value, unsigned char digits_len) {
    while (digits_len > 8) {
        putc('0');
        digits_len--;
    }
    while (digits_len) {
        unsigned char digit = value >> ((digits_len - 1) * 4) & 0xF;
        if (digit >= 0xA) putc('A' + (digit - 0xA));
        else putc('0' + digit);
        digits_len--;
    }
    putc('\n');
}

char strcmp_len(char* str1, char* str2, int len) {
    for (int idx = 0; idx < len; idx++) {
        if (*(str1 + idx) != *(str2 + idx)) return 1;
    }
    return 0;
}
