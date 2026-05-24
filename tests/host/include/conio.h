#pragma once

#include <stdint.h>
#include <stdio.h>

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

static inline void cputc(char c)
{
    putchar(c);
}

static inline void cputs(const char *s)
{
    fputs(s, stdout);
}
