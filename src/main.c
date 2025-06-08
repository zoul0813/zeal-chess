#include <stdio.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zgdk.h>

#include "chess.h"

int main(void) {
    init_board();
    print_board();
    move_input_loop();

    return 0;
}