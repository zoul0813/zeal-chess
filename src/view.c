#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <zvb_gfx.h>
#include <zos_sys.h>

#include <zgdk.h>
#include <zgdk/ascii.h>
#include <zgdk/tilemap.h>
#include <zgdk/utils/print.h>

#include "assets.h"
#include "view.h"
#include "chess.h"

static uint8_t     s_sprite_idx;
static uint8_t*    s_gfx_board; // 0x88 board, 16x8

void view_init(uint8_t *the_board)
{
    s_gfx_board = the_board;

    gfx_enable_screen(0);

    if (gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_4BIT, &vctx)) {
        exit(1);
    }

    if(load_palette(&vctx)) {
        exit(1);
    }

    if(load_board_tileset(&vctx)) {
        exit(1);
    }

    if(load_pieces_tileset(&vctx)) {
        exit(1);
    }

    if(load_font(&vctx)) {
        exit(1);
    }
    ascii_map('A', 26, TILE_FONT_START);
    ascii_map('0', 10, TILE_FONT_START + 26);
    ascii_map(' ', 1, EMPTY_TILE);

    if(gfx_tileset_add_color_tile(&vctx, EMPTY_TILE, 0)) {
        exit(1);
    }


    tilemap_fill(&vctx, LAYER1, 0, 0, 0, WIDTH, HEIGHT);

    if(load_board_tilemap(&vctx)) {
        exit(1);
    }

    view_clear_status_rows();
    gfx_enable_screen(1);
}

const char *view_status_text(uint8_t side, GameStatus status, uint8_t thinking)
{
    if (thinking) {
        return "BLACK THINKING";
    }

    switch (status) {
        case GAME_STATUS_CHECK:
            return side == WHITE ? "WHITE CHECK" : "BLACK CHECK";
        case GAME_STATUS_CHECKMATE:
            return "CHECKMATE";
        case GAME_STATUS_STALEMATE:
            return "STALEMATE";
        case GAME_STATUS_NORMAL:
        default:
            return side == WHITE ? "WHITE TO MOVE" : "BLACK TO MOVE";
    }
}

void view_draw_text(uint8_t x, uint8_t y, const char *text)
{
    uint8_t len = 0;

    if (!text) {
        return;
    }

    while (text[len] && len < WIDTH - x) {
        len++;
    }

    nprint_string_layer(&vctx, text, len, LAYER0, x, y);
}

void view_clear_status_rows(void)
{
    tilemap_fill(&vctx, LAYER0, EMPTY_TILE, 0, 0, WIDTH, 2);
    tilemap_fill(&vctx, LAYER0, EMPTY_TILE, 0, HEIGHT - 2, WIDTH, 2);
}

void view_show_status(uint8_t side, GameStatus status)
{
    view_clear_status_rows();
    view_draw_text(1, 0, view_status_text(side, status, 0));
    view_draw_text(1, HEIGHT - 1, "B SELECT");
}

void view_show_thinking(void)
{
    view_clear_status_rows();
    view_draw_text(1, 0, view_status_text(BLACK, GAME_STATUS_NORMAL, 1));
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

    const uint8_t palette = (PIECES_PALETTE + ((color & 1) ? 1 : 0)) << 4;
    const uint16_t iso_x = 160 + (y - x) * 16;
    const uint16_t iso_y = 172 - (y + x) * 8;

    const uint8_t start_tile = TILE_PIECES_START + ((type - 1) * TILE_PER_SPRITE);

    /* Left part of the piece */
    const uint8_t spr_index = s_sprite_idx;
    SPRITES[s_sprite_idx].x = iso_x;
    SPRITES[s_sprite_idx].y = iso_y;
    SPRITES[s_sprite_idx].tile  = start_tile;
    SPRITES[s_sprite_idx].flags = palette;
    SPRITES[s_sprite_idx].options = SPRITE_OPTION_32PX;
    s_sprite_idx++;
    /* Right part of the piece */
    SPRITES[s_sprite_idx].x = iso_x + 16;
    SPRITES[s_sprite_idx].y = iso_y;
    SPRITES[s_sprite_idx].tile  = start_tile + 2;
    SPRITES[s_sprite_idx].flags = palette;
    SPRITES[s_sprite_idx].options = SPRITE_OPTION_32PX;
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
}

void view_deselect_piece(uint8_t index)
{
    if (index == 0xff) {
        return;
    }

    gfx_sprite_render_array(&vctx, index, &SPRITES[index], 2);
}
