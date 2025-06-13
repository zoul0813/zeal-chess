/** Intro Splash Screen */
#include <stdio.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <zvb_gfx.h>

#include <zgdk/tilemap.h>

#include "view.h"
#include "splash.h"

void splash_init(void) {
    zos_err_t err;

    err = gfx_initialize(ZVB_CTRL_VID_MODE_GFX_320_8BIT, &vctx);
    if(err != GFX_SUCCESS) {
        exit(1);
    }

    gfx_enable_screen(0);

    memset(SPRITES, 0, sizeof(SPRITES));
    gfx_sprite_render_array(&vctx, 0, SPRITES, GFX_SPRITES_COUNT);

    uint8_t buffer[1024];
    uint16_t size;
    zos_dev_t ztp = open("splash.ztp", O_RDONLY);
    if(ztp < 0) {
        exit(0xFF);
    }
    size = 512;
    err = read(ztp, buffer, &size);
    if(err != ERR_SUCCESS) {
        exit(0xFE);
    }
    err = gfx_palette_load(&vctx, buffer, size, 0);
    if(err != ERR_SUCCESS) {
        exit(0xFD);
    }


    zos_dev_t zts = open("splash.zts", O_RDONLY);
    if(zts < 0) {
        exit(0xFC);
    }

    gfx_tileset_options options = {
        .compression = TILESET_COMP_NONE,
        .from_byte = 0,
    };
    do {
        size = 1024;
        err = read(zts, buffer, &size);
        if(err != ERR_SUCCESS) {
            exit(0xFF);
        }
        if(size > 0) {
            err = gfx_tileset_load(&vctx, buffer, size, &options);
            if(err != ERR_SUCCESS) {
                exit(0xFB);
            }
            options.from_byte += size;
        }
    } while(size > 0);

    zos_dev_t ztm = open("splash.ztm", O_RDONLY);
    if(ztm < 0) {
        exit(0xFA);
    }
    size = (WIDTH * HEIGHT);
    err = read(ztm, buffer, &size);
    if(err != ERR_SUCCESS) {
        exit(0xF9);
    }
    if(size > 0) {
        uint8_t *tilemap = buffer;
        for(uint8_t y = 0; y < HEIGHT; y++) {
            gfx_tilemap_load(&vctx, tilemap, WIDTH, LAYER0, 0, y);
            tilemap += WIDTH;
        }
    }

    tilemap_fill(&vctx, LAYER1, 0xFF, 0, 0, 20, 15);

    gfx_enable_screen(1);
}

void splash_show(void) {
    // it's pretty, show it for a while ...
    msleep(2500);
    return;
}