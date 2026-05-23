#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <zos_errors.h>
#include <zos_vfs.h>
#include <zos_sys.h>
#include <zos_video.h>
#include <zos_keyboard.h>

#include <conio.h>
#include <zgdk/input.h>

#include "main.h"
#include "chess.h"
#include "view.h"
#include "splash.h"

#define FSM_SELECTING   0
#define FSM_MOVING      1

static uint8_t continue_loop = 0;
static char input[16];
static uint8_t the_board[128];
static uint8_t s_cpy_board[128];
static uint8_t the_board_gfx[128];

gfx_context vctx;
gfx_sprite  SPRITES[GFX_SPRITES_COUNT];

/* Selected piece */
static uint8_t s_selected = 0;
/* Selected piece while moving */
static uint8_t s_cpy_selected = 0;
/* Current FSM state */
static uint8_t s_fsm_state = FSM_SELECTING;
static Move s_legal_moves[256];
static uint16_t s_legal_move_count = 0;
static uint16_t s_selected_move_index = 0;

uint16_t input1_prev = 0;
zos_err_t err;

__sfr __at(0x80) debug;


static void enter_move_mode(void)
{
    s_legal_move_count = generate_legal_moves_for_square(s_selected, WHITE, s_legal_moves, 256);
    if (s_legal_move_count == 0)
        return;

    memcpy(s_cpy_board, the_board, sizeof(the_board));
    s_fsm_state = FSM_MOVING;
    debug = s_selected;
    s_cpy_selected = s_selected;
    s_selected_move_index = 0;
}

static void preview_selected_move(void)
{
    Move *move = &s_legal_moves[s_selected_move_index];
    uint8_t side = move->piece & (WHITE | BLACK);
    uint8_t piece = move->promotion ? (side | move->promotion) : move->piece;

    memcpy(s_cpy_board, the_board, sizeof(the_board));
    s_cpy_board[move->from] = EMPTY;
    s_cpy_board[move->to] = piece;
    s_cpy_selected = move->to;
    debug = move->to;

    view_draw(s_cpy_board);
    view_select_piece(the_board_gfx[move->to]);
}

static void controller_handle_selection(uint16_t input1)
{
    uint8_t new_selected = 0xff;

    if (RIGHT1) {
        new_selected = find_legal_move_piece(s_selected, WHITE, CHESS_DIR_RIGHT);
    } else if (LEFT1) {
        new_selected = find_legal_move_piece(s_selected, WHITE, CHESS_DIR_LEFT);
    } else if (UP1) {
        new_selected = find_legal_move_piece(s_selected, WHITE, CHESS_DIR_UP);
    } else if (DOWN1) {
        new_selected = find_legal_move_piece(s_selected, WHITE, CHESS_DIR_DOWN);
    } else if (BUTTON1_B) {
        enter_move_mode();
        if (s_fsm_state == FSM_MOVING)
            preview_selected_move();
    }

    if (new_selected != 0xff) {
        view_deselect_piece(the_board_gfx[s_selected]);
        s_selected = new_selected;
        view_select_piece(the_board_gfx[s_selected]);
    }
}


static void controller_handle_move(uint16_t input1)
{
    uint8_t changed_move = 0;

    if (RIGHT1 || DOWN1) {
        s_selected_move_index = (s_selected_move_index + 1) % s_legal_move_count;
        changed_move = 1;
    } else if (LEFT1 || UP1) {
        if (s_selected_move_index == 0)
            s_selected_move_index = s_legal_move_count - 1;
        else
            s_selected_move_index--;
        changed_move = 1;
    } else if (BUTTON1_B) {
        /* Commit the move */
        Move move;
        memcpy(&move, &s_legal_moves[s_selected_move_index], sizeof(move));
        make_move(&move);
        view_draw(the_board);
        s_fsm_state = FSM_SELECTING;
        s_selected = find_legal_move_piece(move.to, WHITE, CHESS_DIR_RIGHT);
        if (s_selected != 0xff)
            view_select_piece(the_board_gfx[s_selected]);
    } else if (BUTTON1_A) {
        /* Cancel the move */
        s_fsm_state = FSM_SELECTING;
        /* Render the former board */
        view_draw(the_board);
        view_select_piece(the_board_gfx[s_selected]);
        return;
    }

    if (changed_move)
        preview_selected_move();
}


int main(void) {

    splash_init();
    splash_show();

    board_init(the_board);

    /* Initialize Input */
    err = input_init(true);

    /* Initialize the view */
    view_init(the_board_gfx);
    view_draw(the_board);

    s_selected = find_legal_move_piece(INDEX(0, 0), WHITE, CHESS_DIR_RIGHT);
    if (s_selected != 0xff)
        view_select_piece(the_board_gfx[s_selected]);

    while (1) {
        uint16_t input1 = input_get();
        if(input1 == input1_prev) continue;
        input1_prev = input1;

        if(SELECT1) goto exit_game; // TODO: prompt confirm?
        if(BUTTON1_Y) {
            ai_move_turn();
            view_draw(the_board);
        }

        switch (s_fsm_state) {
            case FSM_SELECTING:
                controller_handle_selection(input1);
                break;
            case FSM_MOVING:
                controller_handle_move(input1);
                break;
        }
    }

    /*
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
    */

exit_game:
    ioctl(DEV_STDOUT, CMD_RESET_SCREEN, NULL);
    print_board();

    return 0;
}
