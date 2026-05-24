#pragma once

#include <stdint.h>

#define GFX_SPRITES_COUNT 64
#define LAYER0 0
#define LAYER1 1
#define ZVB_CTRL_VID_MODE_GFX_320_4BIT 0
#define SPRITE_OPTION_32PX 0x01
#define TILESET_COMP_NONE 0
#define TILESET_COMP_1BIT 1
#define TILESET_COMP_4BIT 2
#define TILESET_COMP_2BIT 3

typedef uint8_t gfx_error;

typedef struct {
    uint8_t unused;
} gfx_context;

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t tile;
    uint8_t flags;
    uint8_t options;
} gfx_sprite;

typedef struct {
    uint8_t compression;
    uint16_t from_byte;
    uint8_t pal_offset;
    uint8_t opacity;
} gfx_tileset_options;

gfx_error gfx_initialize(uint8_t mode, gfx_context* out);
void gfx_enable_screen(uint8_t ena);
gfx_error gfx_palette_load(gfx_context* ctx, void* palette, uint16_t size, uint8_t from);
gfx_error gfx_tileset_load(gfx_context* ctx, void* tileset, uint16_t size, const gfx_tileset_options* options);
gfx_error gfx_tileset_add_color_tile(gfx_context* ctx, uint16_t index, uint8_t color);
gfx_error gfx_tilemap_load(gfx_context* ctx, void* tiles, uint8_t size, uint8_t layer, uint8_t x, uint8_t y);
gfx_error gfx_sprite_render_array(gfx_context* ctx, uint8_t index, gfx_sprite *sprites, uint8_t count);
gfx_error gfx_sprite_set_flags(gfx_context* ctx, uint8_t index, uint8_t flags);
