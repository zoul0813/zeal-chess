#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zos_keyboard.h>

#include "conio.h"
#include "input.h"
#include "chess.h"
#include "view.h"

#define DIR_LEFT    0
#define DIR_RIGHT   1
#define DIR_DOWN    2
#define DIR_UP      3

#define FSM_SELECTING   0
#define FSM_MOVING      1

static int continue_loop = 0;
static char input[16];
static uint8_t the_board[128];
static uint8_t s_cpy_board[128];
static uint8_t the_board_gfx[128];

/* Selected piece */
static uint8_t s_selected = 0;
/* Selected piece while moving */
static uint8_t s_cpy_selected = 0;
/* Current FSM state */
static uint8_t s_fsm_state = FSM_SELECTING;

__sfr __at(0x80) debug;


static void enter_move_mode(void)
{
    memcpy(s_cpy_board, the_board, sizeof(the_board));
    s_fsm_state = FSM_MOVING;
    debug = s_selected;
    s_cpy_selected = s_selected;
}


static uint8_t find_cell(uint8_t* board, uint8_t selected, uint8_t dir, uint8_t is_empty)
{
    uint8_t row = selected & 0x70;
    uint8_t col = selected & 0x07;

    for (uint8_t i = 0; i < 15; i++) {
        if      (dir == DIR_LEFT)  col = (col - 1) & 0x7;
        else if (dir == DIR_RIGHT) col = (col + 1) & 0x7;
        else if (dir == DIR_DOWN)  row = (row - 0x10) & 0x70;
        else if (dir == DIR_UP)    row = (row + 0x10) & 0x70;

        const uint8_t coord = row | col;
        if ( (!is_empty && (board[coord] & WHITE) != 0) ||
             ( is_empty && (board[coord] & 7) == EMPTY) )
        {
            return coord;
        }
    }

    return 0xff;
}

static void controller_handle_selection(uint8_t key)
{
    uint8_t new_selected = 0xff;

    if (key == KB_RIGHT_ARROW) {
        new_selected = find_cell(the_board, s_selected, DIR_RIGHT, 0);
    } else if (key == KB_LEFT_ARROW) {
        new_selected = find_cell(the_board, s_selected, DIR_LEFT, 0);
    } else if (key == KB_UP_ARROW) {
        new_selected = find_cell(the_board, s_selected, DIR_UP, 0);
    } else if (key == KB_DOWN_ARROW) {
        new_selected = find_cell(the_board, s_selected, DIR_DOWN, 0);
    } else if (key == KB_KEY_ENTER) {
        enter_move_mode();
    }

    if (new_selected != 0xff) {
        view_deselect_piece(the_board_gfx[s_selected]);
        s_selected = new_selected;
        view_select_piece(the_board_gfx[s_selected]);
    }
}


static void controller_handle_move(uint8_t key)
{
    uint8_t new_selected = 0xff;

    /* TODO: Generate all the possible positions for the selected piece? */
    if (key == KB_RIGHT_ARROW) {
        new_selected = find_cell(s_cpy_board, s_cpy_selected, DIR_RIGHT, 1);
    } else if (key == KB_LEFT_ARROW) {
        new_selected = find_cell(s_cpy_board, s_cpy_selected, DIR_LEFT, 1);
    } else if (key == KB_UP_ARROW) {
        new_selected = find_cell(s_cpy_board, s_cpy_selected, DIR_UP, 1);
    } else if (key == KB_DOWN_ARROW) {
        new_selected = find_cell(s_cpy_board, s_cpy_selected, DIR_DOWN, 1);
    } else if (key == KB_KEY_BACKSPACE) {
        /* Cancel the move */
        s_fsm_state = FSM_SELECTING;
        /* Render the former board */
        view_draw(the_board);
        view_select_piece(the_board_gfx[s_selected]);
        return;
    }

    if (new_selected != 0xff) {
        debug = new_selected;
        s_cpy_board[new_selected] = s_cpy_board[s_cpy_selected];
        s_cpy_board[s_cpy_selected] = 0;
        s_cpy_selected = new_selected;
        view_draw(s_cpy_board);
        view_select_piece(the_board_gfx[new_selected]);
    }
}


int main(void) {
    board_init(the_board);

    /* Set the keyboard to RAW */
    zos_err_t err = ioctl(DEV_STDIN, KB_CMD_SET_MODE, (void*) (KB_READ_NON_BLOCK | KB_MODE_RAW));
    if (err) {
        return 2;
    }

    /* Initialize the view */
    view_init(the_board_gfx);
    view_draw(the_board);

    view_select_piece(the_board_gfx[s_selected]);

    while (1) {
        int size = 3;
        uint8_t* kbdata = input;
        zos_err_t ret = read(DEV_STDIN, kbdata, &size);
        if (ret == ERR_SUCCESS && size != 0) {
            if (*kbdata == KB_RELEASED) {
                kbdata += 2;
            }

            switch (s_fsm_state) {
                case FSM_SELECTING:
                    controller_handle_selection(*kbdata);
                    break;
                case FSM_MOVING:
                    controller_handle_move(*kbdata);
                    break;
            }
        }
    }

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