#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chess.h"

int evaluate_board(unsigned char side);

int fflush_stdout(void)
{
    return fflush(stdout);
}

static void require(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void clear_board(unsigned char test_board[128])
{
    memset(test_board, 0, 128);
    board = test_board;
}

static int move_exists(Move moves[], int count, unsigned char from, unsigned char to)
{
    for (int i = 0; i < count; i++) {
        if (moves[i].from == from && moves[i].to == to)
            return 1;
    }

    return 0;
}

static int count_side_pieces(unsigned char side)
{
    int count = 0;

    for (unsigned char sq = 0; sq < 128; sq++) {
        if (!IS_ON_BOARD(sq))
            continue;
        if (board[sq] != EMPTY && is_friendly(board[sq], side))
            count++;
    }

    return count;
}

static void test_board_init(void)
{
    unsigned char test_board[128];

    memset(test_board, 0xAA, sizeof(test_board));
    board_init(test_board);

    require(board == test_board, "board_init stores board pointer");
    require(board[INDEX(0, 4)] == (WHITE | KING), "white king starts on e1");
    require(board[INDEX(7, 4)] == (BLACK | KING), "black king starts on e8");
    require(board[INDEX(1, 0)] == (WHITE | PAWN), "white pawn starts on a2");
    require(board[INDEX(6, 7)] == (BLACK | PAWN), "black pawn starts on h7");
    require(count_side_pieces(WHITE) == 16, "white starts with 16 pieces");
    require(count_side_pieces(BLACK) == 16, "black starts with 16 pieces");
}

static void test_initial_white_moves(void)
{
    unsigned char test_board[128];
    Move moves[256];

    board_init(test_board);

    int count = generate_legal_moves(WHITE, moves, 256);
    require(count > 0, "initial white legal move count is positive");
}

static void test_material_evaluation(void)
{
    unsigned char test_board[128];

    board_init(test_board);

    require(evaluate_board(WHITE) == 0, "initial material is equal for white");
    require(evaluate_board(BLACK) == 0, "initial material is equal for black");

    board[INDEX(7, 3)] = EMPTY;

    require(evaluate_board(WHITE) == QUEEN_VALUE, "white leads by queen after black queen removed");
    require(evaluate_board(BLACK) == -QUEEN_VALUE, "black trails by queen after black queen removed");
}

static void test_pinned_piece_move_filtered(void)
{
    unsigned char test_board[128];
    Move moves[256];
    unsigned char pinned_rook = INDEX(1, 4);
    unsigned char sideways    = INDEX(1, 5);

    clear_board(test_board);
    board[INDEX(0, 4)] = WHITE | KING;
    board[pinned_rook] = WHITE | ROOK;
    board[INDEX(7, 4)] = BLACK | ROOK;
    board[INDEX(7, 0)] = BLACK | KING;

    require(is_valid_move(pinned_rook, sideways, WHITE), "pinned rook move is pseudo-legal");

    int count = generate_legal_moves(WHITE, moves, 256);
    require(!move_exists(moves, count, pinned_rook, sideways), "pinned rook move is filtered from legal moves");
    require(move_exists(moves, count, pinned_rook, INDEX(2, 4)), "pinned rook can move along pin line");
}

static void test_king_cannot_move_into_check(void)
{
    unsigned char test_board[128];
    Move moves[256];
    unsigned char king_from = INDEX(0, 4);
    unsigned char attacked  = INDEX(1, 4);

    clear_board(test_board);
    board[king_from]   = WHITE | KING;
    board[INDEX(7, 4)] = BLACK | ROOK;
    board[INDEX(7, 0)] = BLACK | KING;

    require(is_valid_move(king_from, attacked, WHITE), "king move into attacked square is pseudo-legal");

    int count = generate_legal_moves(WHITE, moves, 256);
    require(!move_exists(moves, count, king_from, attacked), "king move into check is filtered from legal moves");
}

int main(void)
{
    test_board_init();
    test_initial_white_moves();
    test_material_evaluation();
    test_pinned_piece_move_filtered();
    test_king_cannot_move_into_check();

    puts("host chess tests passed");
    return 0;
}
