#pragma once

#include <stdint.h>
#include <zvb_gfx.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define WIDTH         20
#define HEIGHT        15

extern gfx_context vctx;
extern gfx_sprite  SPRITES[GFX_SPRITES_COUNT];

void view_init(uint8_t *the_board);
void view_draw(const uint8_t* board);
void view_clear_pieces(void);
uint8_t view_place_piece(uint8_t x, uint8_t y, uint8_t type, uint8_t color);
void view_render_pieces(void);
void view_select_piece(uint8_t index);
void view_deselect_piece(uint8_t index);