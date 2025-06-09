#include <stdio.h>
#include <stdint.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zgdk.h>

#include "conio.h"
#include "input.h"
#include "chess.h"

int continue_loop = 0;
char input[16];

int main(void) {
    lowvideo();
    clrscr();
    init_board();
    print_board();

    while (1) {

        if (side_to_move == WHITE) {

            printf("\nWhite to move or 'quit':\n");
            clreol();
            gotoxy(0, 13);
            if (!fgets(input, sizeof(input), DEV_STDIN)) {
                printf("Input error or EOF\n");
                return 0; // stop loop
            }
            continue_loop = human_move_turn(input);
            side_to_move  = BLACK;
        } else {
            continue_loop = ai_move_turn();
            side_to_move  = WHITE;
        }

        print_board();

        if (is_in_check(side_to_move)) {
            if (!has_legal_moves(side_to_move)) {
                printf("%s is in checkmate. Game over!\n", side_to_move == WHITE ? "White" : "Black");
                break;
            } else {
                printf("%s is in check.\n", side_to_move == WHITE ? "White" : "Black");
            }
        } else if (!has_legal_moves(side_to_move)) {
            printf("Stalemate. Game over!\n");
            break;
        }


        if (!continue_loop)
            break;
    }

    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    print_board();

    return 0;
}