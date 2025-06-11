#pragma once

#include <stdint.h>

void view_init(void);
void view_clear_pieces(void);
uint8_t view_place_piece(uint8_t x, uint8_t y, uint8_t type, uint8_t color);
void view_render_pieces(void);
void view_select_piece(uint8_t index);
void view_deselect_piece(uint8_t index);