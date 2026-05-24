#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CH_NEWLINE '\n'
#define CH_SPACE   ' '

static inline uint16_t put_s(const char *str)
{
    fputs(str, stdout);
    return str ? (uint16_t)strlen(str) : 0;
}

static inline uint16_t put_c(const char c)
{
    return (uint16_t)putchar(c);
}

static inline void *mem_set(void *ptr, uint8_t value, size_t size)
{
    return memset(ptr, value, size);
}

static inline void *mem_cpy(void *dst, const void *src, size_t size)
{
    return memcpy(dst, src, size);
}
