#include <stdint.h>
#include <zos_sys.h>
#include <zos_vfs.h>
#include <zvb_gfx.h>
#include <zgdk/tilemap.h>

#include "assets.h"

zar_file_t asset_file;

/* 4 16-bit colors for the board palette */
const uint16_t s_board_palette[] = {
    /* 4 colors for the board */
    [0] = 0x4228,
    [1] = 0x4984,
    [2] = 0x7ae8,
    [3] = 0xde32,
#if MODERN_PALETTE
    /* Black palette */
    [16] = 0x0000,
    [17] = 0x31e7,
    [18] = 0x2104,
    [19] = 0x9452,
    [20] = 0x732e,
    [21] = 0x39a8,

    /* White palette */
    [32] = 0x0000,
    [33] = 0x31e7,
    [34] = 0x5aaa,
    [35] = 0xffff,
    [36] = 0x9492,
    [37] = 0xdefd,
#else
    /* Brown palette */
    [16] = 0x0000, // unused
    [17] = 0x632c, // Deep brown
    [18] = 0x842f, // Dark wood
    [19] = 0xd5b4, // Mid brown
    [20] = 0xcd72,
    [21] = 0xfed6, // Highlight

    /* Beige palette */
    [32] = 0x0000, // unused
    [33] = 0xB52D,
    [34] = 0xD675,
    [35] = 0xF718,
    [36] = 0xFF55,
    [37] = 0xFFF6,
#endif

    /* Highlighted palette */
    [48] = 0x0000, // unused
    [49] = 0xf230,
    [50] = 0xee00,
    [51] = 0xed10,
    [52] = 0xf113,
    [53] = 0xf332,

};

zos_err_t load_asset_file(const char *path)
{
    return zar_file_open(path, &asset_file);
}

zos_err_t _load_asset_entry(const char* path, zar_file_entry_t* entry)
{
    zos_err_t err = zar_file_entry_from_name(&asset_file, path, entry);
    return err;
}

zos_err_t _read_asset_chunk(zar_file_entry_t* entry, uint8_t* buffer, uint16_t* size)
{
    zos_err_t err = zar_file_read(&asset_file, entry, buffer, size);

    if (err == ERR_NO_MORE_ENTRIES && *size == 0) {
        return ERR_SUCCESS;
    }

    return err;
}

zos_err_t _load_zts_chunk(gfx_context* ctx, const char* path, gfx_tileset_options* options)
{
    zos_err_t err;
    uint16_t size;
    uint8_t buffer[1024];
    zar_file_entry_t entry;

    err = _load_asset_entry(path, &entry);
    if(err) return err;

    do {
        size = 1024;
        err  = _read_asset_chunk(&entry, buffer, &size);
        if (err != ERR_SUCCESS) return err;

        if (size > 0) {
            uint16_t loaded_size = size;
            err = gfx_tileset_load(ctx, buffer, size, options);
            if (err) return err;

            if (options->compression == TILESET_COMP_4BIT) {
                loaded_size <<= 1;
            } else if (options->compression == TILESET_COMP_2BIT) {
                loaded_size <<= (ctx->bpp == 8) ? 2 : 1;
            } else if (options->compression == TILESET_COMP_1BIT) {
                loaded_size <<= (ctx->bpp == 8) ? 3 : 2;
            }

            options->from_byte += loaded_size;
        }
    } while (size > 0);

    return ERR_SUCCESS;
}

zos_err_t _load_pieces_zts_chunk(gfx_context* ctx, const char* path, gfx_tileset_options* options)
{
    zos_err_t err;
    uint16_t size;
    uint8_t buffer[1024];
    zar_file_entry_t entry;

    err = _load_asset_entry(path, &entry);
    if(err) return err;

    do {
        size = 1024;
        err  = _read_asset_chunk(&entry, buffer, &size);
        if (err != ERR_SUCCESS) return err;

        if (size > 0) {
            for (uint16_t i = 0; i < size; i++) {
                uint8_t hi = buffer[i] >> 4;
                uint8_t lo = buffer[i] & 0x0F;

                if (hi == 4) hi = 0;
                else if (hi == 0) hi = 4;

                if (lo == 4) lo = 0;
                else if (lo == 0) lo = 4;

                buffer[i] = (hi << 4) | lo;
            }

            err = gfx_tileset_load(ctx, buffer, size, options);
            if (err) return err;

            options->from_byte += size;
        }
    } while (size > 0);

    return ERR_SUCCESS;
}

zos_err_t _load_ztm(gfx_context* ctx, const char* path) {
    zos_err_t err;
    uint16_t size;
    uint8_t buffer[WIDTH * HEIGHT];
    zar_file_entry_t entry;

    err = _load_asset_entry(path, &entry);
    if(err) return err;

    size = (WIDTH * HEIGHT);
    err  = _read_asset_chunk(&entry, buffer, &size);
    if (err) return err;

    if (size > 0) {
        uint8_t* tilemap = buffer;
        for (uint8_t y = 0; y < HEIGHT; y++) {
            err = gfx_tilemap_load(ctx, tilemap, WIDTH, LAYER0, 0, y);
            if (err) return err;

            tilemap += WIDTH;
        }
    }

    return ERR_SUCCESS;
}

zos_err_t _load_ztp_at(gfx_context* ctx, const char* path, uint8_t from) {
    zos_err_t err;
    uint16_t size;
    uint8_t buffer[512];
    zar_file_entry_t entry;

    err = _load_asset_entry(path, &entry);
    if(err) return err;

    size = 512; // 256 color
    err  = _read_asset_chunk(&entry, buffer, &size);
    if (err) return err;

    err = gfx_palette_load(ctx, buffer, size, from);
    return err;
}

zos_err_t _load_ztp(gfx_context* ctx, const char* path) {
    return _load_ztp_at(ctx, path, 0);
}

zos_err_t load_palette(gfx_context* ctx)
{
    gfx_error err;
    err = gfx_palette_load(ctx, (uint8_t*) s_board_palette, sizeof(s_board_palette), 0);
    return err;
}

zos_err_t load_board_tileset(gfx_context* ctx)
{
    gfx_error err;
    /* The tileset is "compressed" in 2-bit mode, so 2 bits represent a pixel */
    gfx_tileset_options options = {
        .compression = TILESET_COMP_2BIT,
    };
    err = _load_zts_chunk(ctx, "board.zts", &options);
    return err;
}

zos_err_t load_board_tilemap(gfx_context* ctx) {
    zos_err_t err;

    err = _load_ztm(ctx, "board.ztm");

    return err;
}

zos_err_t load_pieces_tileset(gfx_context* ctx)
{
    gfx_error err;

    gfx_tileset_options options   = {
        .compression = TILESET_COMP_NONE,
        // Start at tile 16 (16 * 128);
        .from_byte = TILE_PIECES_START << 7
    };
    err = _load_pieces_zts_chunk(ctx, "pieces.zts", &options);

    return err;
}

zos_err_t load_font(gfx_context* ctx)
{
    gfx_error err;

    err = _load_ztp_at(ctx, "font.ztp", FONT_PALETTE_START);
    if(err) return err;

    gfx_tileset_options options = {
        .compression = TILESET_COMP_NONE,
        .from_byte   = TILE_FONT_START << 7,
        .pal_offset  = FONT_PALETTE_START,
        .opacity     = 1,
    };
    err = _load_zts_chunk(ctx, "font.zts", &options);

    return err;
}

/** Splash Screen */
zos_err_t load_splash(gfx_context* ctx)
{
    gfx_error err;

    err = _load_ztp(ctx, "splash.ztp");
    if(err) return err;

    gfx_tileset_options options = {
        .compression = TILESET_COMP_NONE,
        .from_byte   = 0,
    };
    err = _load_zts_chunk(ctx, "splash.zts", &options);

    return err;
}

zos_err_t load_splash_tilemap(gfx_context* ctx)
{
    gfx_error err;

    err = _load_ztm(ctx, "splash.ztm");
    if(err) return err;

    err = gfx_tileset_add_color_tile(ctx, EMPTY_TILE, 0);
    return err;
}
