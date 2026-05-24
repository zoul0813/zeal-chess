#include <stdint.h>
#include <zos_errors.h>
#include <zvb_gfx.h>
#include <zar.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240
#define WIDTH         20
#define HEIGHT        15

#define TILE_PER_SPRITE     4
#define TILE_PIECES_START   16
#define TILE_FONT_START     (TILE_PIECES_START + 24)
#define PIECES_PALETTE      1
#define FONT_PALETTE_START  64
#define MODERN_PALETTE      1
#define HIGHLG_PALETTE      3

#define EMPTY_TILE          0xFF

extern uint8_t _pieces_tileset_start;
extern uint8_t _pieces_tileset_end;

extern const uint16_t s_board_palette[];
extern const uint8_t s_board_tilemap[];
extern const uint8_t s_board_tileset[];
extern zar_file_t asset_file;


zos_err_t load_asset_file(const char *path);

zos_err_t load_palette(gfx_context* ctx);

zos_err_t load_board_tileset(gfx_context* ctx);
zos_err_t load_board_tilemap(gfx_context* ctx);

zos_err_t load_pieces_tileset(gfx_context* ctx);
zos_err_t load_font(gfx_context* ctx);

zos_err_t load_splash(gfx_context* ctx);
zos_err_t load_splash_tilemap(gfx_context* ctx);
