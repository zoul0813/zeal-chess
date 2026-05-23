#pragma once

#include <stdint.h>

#define CH_SPACE ' '
#define CH_NEWLINE '\n'

static inline void gotoxy(uint8_t x, uint8_t y)
{
    (void)x;
    (void)y;
}

static inline void bgcolor(uint8_t color)
{
    (void)color;
}

static inline void textcolor(uint8_t color)
{
    (void)color;
}
