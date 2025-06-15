#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <zvb_gfx.h>
#include <zos_sys.h>

#include "assets.h"
#include "view.h"
#include "chess.h"

#define GFX_WIDTH           20
#define GFX_HEIGHT          15
#define TILE_SIZE           16



static uint8_t     s_sprite_idx;
static uint8_t*    s_gfx_board; // 0x88 board, 16x8

void view_init(uint8_t *the_board)
{
    s_gfx_board = the_board;
    uint8_t empty[GFX_WIDTH];

    if (gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_4BIT, &vctx)) {
        exit(1);
    }

    gfx_enable_screen(0);

    if(load_palette(&vctx)) {
        exit(1);
    }

    if(load_board_tileset(&vctx)) {
        exit(1);
    }

    if(load_pieces_tileset(&vctx)) {
        exit(1);
    }

    /* Load the tilemap (layer0) */
    const uint8_t *tilemap = s_board_tilemap;
    memset(empty, 0, sizeof(empty));
    for (uint8_t i = 0; i < GFX_HEIGHT; i++) {
        gfx_tilemap_load(&vctx, tilemap, GFX_WIDTH, 0, 0, i);
        gfx_tilemap_load(&vctx, empty,   GFX_WIDTH, 1, 0, i);
        tilemap += GFX_WIDTH;
    }

    gfx_enable_screen(1);
}

void view_draw(const uint8_t* board)
{
    /* Browse the board diagonally, from back to front */
    static const uint8_t indexes[] = {
        INDEX(7,7),
        INDEX(7,6), INDEX(6,7),
        INDEX(7,5), INDEX(6,6), INDEX(5,7),
        INDEX(7,4), INDEX(6,5), INDEX(5,6), INDEX(4,7),
        INDEX(7,3), INDEX(6,4), INDEX(5,5), INDEX(4,6), INDEX(3,7),
        INDEX(7,2), INDEX(6,3), INDEX(5,4), INDEX(4,5), INDEX(3,6), INDEX(2,7),
        INDEX(7,1), INDEX(6,2), INDEX(5,3), INDEX(4,4), INDEX(3,5), INDEX(2,6), INDEX(1,7),
        INDEX(7,0), INDEX(6,1), INDEX(5,2), INDEX(4,3), INDEX(3,4), INDEX(2,5), INDEX(1,6), INDEX(0,7),
        INDEX(6,0), INDEX(5,1), INDEX(4,2), INDEX(3,3), INDEX(2,4), INDEX(1,5), INDEX(0,6),
        INDEX(5,0), INDEX(4,1), INDEX(3,2), INDEX(2,3), INDEX(1,4), INDEX(0,5),
        INDEX(4,0), INDEX(3,1), INDEX(2,2), INDEX(1,3), INDEX(0,4),
        INDEX(3,0), INDEX(2,1), INDEX(1,2), INDEX(0,3),
        INDEX(2,0), INDEX(1,1), INDEX(0,2),
        INDEX(1,0), INDEX(0,1),
        INDEX(0,0)
    };

    view_clear_pieces();

    for (uint8_t i = 0; i < sizeof(indexes); i++) {
        const uint8_t pos = indexes[i];
        const uint8_t piece = board[pos];
        s_gfx_board[pos] = view_place_piece(GET_X(pos), GET_Y(pos), piece & 0x7, piece >> 3);
    }

    view_render_pieces();
}

void view_clear_pieces(void)
{
    for (uint8_t i = 0; i < GFX_SPRITES_COUNT; i++) {
        SPRITES[i].x = 0;
    }
    s_sprite_idx = 0;
}

uint8_t view_place_piece(uint8_t x, uint8_t y, uint8_t type, uint8_t color)
{

    if (type == EMPTY) {
        return 0xff;
    }

    const uint8_t y_flipped = 7 - y;
    const uint8_t palette = (PIECES_PALETTE + ((color & 1) ? 1 : 0)) << 4;
    const uint16_t iso_x = 160 + (y - x) * 16;
    const uint16_t iso_y = 172 - (y + x) * 8;

    const uint8_t start_tile = TILE_PIECES_START + ((type - 1) * TILE_PER_SPRITE);

    /* Top left part of the piece */
    const uint8_t spr_index = s_sprite_idx;
    SPRITES[s_sprite_idx].x = iso_x;
    SPRITES[s_sprite_idx].y = iso_y;
    SPRITES[s_sprite_idx].tile  = start_tile;
    SPRITES[s_sprite_idx].flags = palette;
    s_sprite_idx++;
    /* Top right part of the piece */
    SPRITES[s_sprite_idx].x = iso_x + 16;
    SPRITES[s_sprite_idx].y = iso_y;
    SPRITES[s_sprite_idx].tile  = start_tile + 1;
    SPRITES[s_sprite_idx].flags = palette;
    s_sprite_idx++;
    /* Bottom left part of the piece */
    SPRITES[s_sprite_idx].x = iso_x;
    SPRITES[s_sprite_idx].y = iso_y + 16;
    SPRITES[s_sprite_idx].tile  = start_tile + 2;
    SPRITES[s_sprite_idx].flags = palette;
    s_sprite_idx++;
    /* Bottom right part of the piece */
    SPRITES[s_sprite_idx].x = iso_x + 16;
    SPRITES[s_sprite_idx].y = iso_y + 16;
    SPRITES[s_sprite_idx].tile  = start_tile + 3;
    SPRITES[s_sprite_idx].flags = palette;
    s_sprite_idx++;

    return spr_index;
}

void view_render_pieces(void)
{
    gfx_sprite_render_array(&vctx, 0, SPRITES, GFX_SPRITES_COUNT);
}

void view_select_piece(uint8_t index)
{
    if (index == 0xff) {
        return;
    }

    gfx_sprite_set_flags(&vctx, index+0, HIGHLG_PALETTE << 4);
    gfx_sprite_set_flags(&vctx, index+1, HIGHLG_PALETTE << 4);
    gfx_sprite_set_flags(&vctx, index+2, HIGHLG_PALETTE << 4);
    gfx_sprite_set_flags(&vctx, index+3, HIGHLG_PALETTE << 4);
}

void view_deselect_piece(uint8_t index)
{
    if (index == 0xff) {
        return;
    }

    gfx_sprite_render_array(&vctx, index, &SPRITES[index], 4);
}