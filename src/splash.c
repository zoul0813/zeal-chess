/** Intro Splash Screen */
#include <stdio.h>
#include <string.h>
#include <zos_sys.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <zos_video.h>
#include <zvb_gfx.h>

#include <zgdk/tilemap.h>

#include "assets.h"
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

    err = load_splash(&vctx);
    if(err) exit(1);

    err = load_splash_tilemap(&vctx);
    if(err) exit(1);

    // clear layer1 to prevent artifacts
    tilemap_fill(&vctx, LAYER1, 0xFF, 0, 0, 20, 15);

    gfx_enable_screen(1);
}

void splash_show(void) {
    // it's pretty, show it for a while ...
    msleep(2500);
    return;
}