#pragma once

#include "hardware.h"

#define INT32_MAX 0xFFFFFFFF
#define NULL ((void*)0)

#define FONT_WIDTH 8
#define FONT_HEIGHT 8

#define COLOR_BLACK 0x000000
#define COLOR_WHITE 0xFFFFFF
#define COLOR_RED 0xFF0000
#define COLOR_GREEN 0x00FF00
#define COLOR_BLUE 0x0000FF

extern unsigned char font[128][8];

void init_frame_buffer(struct FrameBuffer* fb);
void set_bg_color(unsigned color, unsigned* old);
void set_fg_color(unsigned color, unsigned* old);
void puts_n(char* str);
void puts(char* str);
void puth(unsigned long long value, unsigned char digits_len);
void puth_n(unsigned long long value, unsigned char digits_len);

char strcmp_len(char* str1, char* str2, int length);
void strcpy_n(char* dest, char* src, int length);

unsigned char io_read_b(short addr);
unsigned short io_read_w(short addr);
unsigned int io_read_d(short addr);

void io_write_b(short addr, unsigned char item);
void io_write_w(short addr, unsigned short item);
void io_write_d(short addr, unsigned int item);
