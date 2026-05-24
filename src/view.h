#pragma once

#include <stdint.h>
#include <zvb_gfx.h>

#include "chess.h"

extern gfx_context vctx;
extern gfx_sprite  SPRITES[GFX_SPRITES_COUNT];

void view_init(uint8_t *the_board);
void view_draw(const uint8_t* board);
void view_clear_pieces(void);
uint8_t view_place_piece(uint8_t x, uint8_t y, uint8_t type, uint8_t color);
void view_render_pieces(void);
void view_select_piece(uint8_t index);
void view_deselect_piece(uint8_t index);
const char *view_status_text(uint8_t side, GameStatus status, uint8_t thinking);
void view_draw_text(uint8_t x, uint8_t y, const char *text);
void view_clear_status_rows(void);
void view_show_status(uint8_t side, GameStatus status);
void view_show_thinking(void);
