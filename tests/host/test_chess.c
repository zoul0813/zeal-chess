#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "chess.h"

int16_t evaluate_board(uint8_t side);

int fflush_stdout(void)
{
    return fflush(stdout);
}

static void require(uint8_t condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        exit(1);
    }
}

static void clear_board(uint8_t test_board[128])
{
    memset(test_board, 0, 128);
    board = test_board;
}

static uint8_t move_exists(Move moves[], uint16_t count, uint8_t from, uint8_t to)
{
    for (uint16_t i = 0; i < count; i++) {
        if (moves[i].from == from && moves[i].to == to)
            return 1;
    }

    return 0;
}

static uint16_t count_side_pieces(uint8_t side)
{
    uint16_t count = 0;

    for (uint8_t sq = 0; sq < 128; sq++) {
        if (!IS_ON_BOARD(sq))
            continue;
        if (board[sq] != EMPTY && is_friendly(board[sq], side))
            count++;
    }

    return count;
}

static void test_board_init(void)
{
    uint8_t test_board[128];

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
    uint8_t test_board[128];
    Move moves[256];

    board_init(test_board);

    uint16_t count = generate_legal_moves(WHITE, moves, 256);
    require(count > 0, "initial white legal move count is positive");
}

static void test_material_evaluation(void)
{
    uint8_t test_board[128];

    board_init(test_board);

    require(evaluate_board(WHITE) == 0, "initial material is equal for white");
    require(evaluate_board(BLACK) == 0, "initial material is equal for black");

    board[INDEX(7, 3)] = EMPTY;

    require(evaluate_board(WHITE) == QUEEN_VALUE, "white leads by queen after black queen removed");
    require(evaluate_board(BLACK) == -QUEEN_VALUE, "black trails by queen after black queen removed");
}

static void test_pinned_piece_move_filtered(void)
{
    uint8_t test_board[128];
    Move moves[256];
    uint8_t pinned_rook = INDEX(1, 4);
    uint8_t sideways    = INDEX(1, 5);

    clear_board(test_board);
    board[INDEX(0, 4)] = WHITE | KING;
    board[pinned_rook] = WHITE | ROOK;
    board[INDEX(7, 4)] = BLACK | ROOK;
    board[INDEX(7, 0)] = BLACK | KING;

    require(is_valid_move(pinned_rook, sideways, WHITE), "pinned rook move is pseudo-legal");

    uint16_t count = generate_legal_moves(WHITE, moves, 256);
    require(!move_exists(moves, count, pinned_rook, sideways), "pinned rook move is filtered from legal moves");
    require(move_exists(moves, count, pinned_rook, INDEX(2, 4)), "pinned rook can move along pin line");
}

static void test_king_cannot_move_into_check(void)
{
    uint8_t test_board[128];
    Move moves[256];
    uint8_t king_from = INDEX(0, 4);
    uint8_t attacked  = INDEX(1, 4);

    clear_board(test_board);
    board[king_from]   = WHITE | KING;
    board[INDEX(7, 4)] = BLACK | ROOK;
    board[INDEX(7, 0)] = BLACK | KING;

    require(is_valid_move(king_from, attacked, WHITE), "king move into attacked square is pseudo-legal");

    uint16_t count = generate_legal_moves(WHITE, moves, 256);
    require(!move_exists(moves, count, king_from, attacked), "king move into check is filtered from legal moves");
}

static void test_king_capture_rejected(void)
{
    uint8_t test_board[128];
    Move moves[256];
    uint8_t queen_from = INDEX(6, 4);
    uint8_t king_to    = INDEX(7, 4);

    clear_board(test_board);
    board[INDEX(0, 4)] = WHITE | KING;
    board[queen_from]  = WHITE | QUEEN;
    board[king_to]     = BLACK | KING;

    require(is_valid_move(queen_from, king_to, WHITE), "king capture is pseudo-legal for attack detection");

    uint16_t count = generate_legal_moves(WHITE, moves, 256);
    require(!move_exists(moves, count, queen_from, king_to), "enemy king capture is filtered from legal moves");
}

static void test_game_status_checkmate(void)
{
    uint8_t test_board[128];

    clear_board(test_board);
    board[INDEX(0, 0)] = WHITE | KING;
    board[INDEX(1, 1)] = BLACK | QUEEN;
    board[INDEX(2, 2)] = BLACK | KING;

    require(is_in_check(WHITE), "checkmate fixture has white in check");
    require(!has_legal_moves(WHITE), "checkmate fixture has no legal white moves");
    require(game_status(WHITE) == GAME_STATUS_CHECKMATE, "game_status reports checkmate");
}

static void test_game_status_stalemate(void)
{
    uint8_t test_board[128];

    clear_board(test_board);
    board[INDEX(0, 0)] = WHITE | KING;
    board[INDEX(1, 2)] = BLACK | QUEEN;
    board[INDEX(2, 2)] = BLACK | KING;

    require(!is_in_check(WHITE), "stalemate fixture does not have white in check");
    require(!has_legal_moves(WHITE), "stalemate fixture has no legal white moves");
    require(game_status(WHITE) == GAME_STATUS_STALEMATE, "game_status reports stalemate");
}

static void test_game_status_normal_and_check(void)
{
    uint8_t test_board[128];

    board_init(test_board);
    require(game_status(WHITE) == GAME_STATUS_NORMAL, "initial position reports normal status");

    clear_board(test_board);
    board[INDEX(0, 4)] = WHITE | KING;
    board[INDEX(7, 4)] = BLACK | ROOK;
    board[INDEX(7, 0)] = BLACK | KING;

    require(game_status(WHITE) == GAME_STATUS_CHECK, "checked side with legal moves reports check status");
}

static void test_try_make_legal_move_allows_capture(void)
{
    uint8_t test_board[128];
    Move move;

    clear_board(test_board);
    board[INDEX(0, 4)] = WHITE | KING;
    board[INDEX(7, 4)] = BLACK | KING;
    board[INDEX(3, 3)] = WHITE | BISHOP;
    board[INDEX(5, 5)] = BLACK | KNIGHT;

    require(try_make_legal_move(INDEX(3, 3), INDEX(5, 5), WHITE, &move), "legal capture succeeds through move API");
    require(move.from == INDEX(3, 3), "capture move records source");
    require(move.to == INDEX(5, 5), "capture move records destination");
    require(move.piece == (WHITE | BISHOP), "capture move records moving piece");
    require(move.captured == (BLACK | KNIGHT), "capture move records captured piece");
    require(board[INDEX(3, 3)] == EMPTY, "capture clears source square");
    require(board[INDEX(5, 5)] == (WHITE | BISHOP), "capture places moving piece on target");
}

static void test_try_make_legal_move_rejects_illegal_move(void)
{
    uint8_t test_board[128];
    Move move;

    clear_board(test_board);
    board[INDEX(0, 4)] = WHITE | KING;
    board[INDEX(7, 4)] = BLACK | KING;
    board[INDEX(1, 0)] = WHITE | PAWN;

    memset(&move, 0xAA, sizeof(move));
    require(!try_make_legal_move(INDEX(1, 0), INDEX(4, 0), WHITE, &move), "illegal move is rejected through move API");
    require(board[INDEX(1, 0)] == (WHITE | PAWN), "rejected move leaves source unchanged");
    require(board[INDEX(4, 0)] == EMPTY, "rejected move leaves target unchanged");
}

static void test_try_make_legal_move_promotes_to_queen(void)
{
    uint8_t test_board[128];
    Move move;

    clear_board(test_board);
    board[INDEX(0, 4)] = WHITE | KING;
    board[INDEX(7, 4)] = BLACK | KING;
    board[INDEX(6, 0)] = WHITE | PAWN;

    require(try_make_legal_move(INDEX(6, 0), INDEX(7, 0), WHITE, &move), "promotion move succeeds through move API");
    require(move.promotion == QUEEN, "promotion records queen");
    require(board[INDEX(6, 0)] == EMPTY, "promotion clears source square");
    require(board[INDEX(7, 0)] == (WHITE | QUEEN), "promotion creates white queen");
}

static void test_generate_legal_moves_for_square(void)
{
    uint8_t test_board[128];
    Move moves[16];
    uint8_t bishop = INDEX(3, 3);

    clear_board(test_board);
    board[INDEX(0, 4)] = WHITE | KING;
    board[INDEX(7, 4)] = BLACK | KING;
    board[bishop]      = WHITE | BISHOP;
    board[INDEX(5, 5)] = BLACK | KNIGHT;

    uint16_t count = generate_legal_moves_for_square(bishop, WHITE, moves, 16);
    require(count > 0, "square legal move API returns moves for selected piece");
    require(move_exists(moves, count, bishop, INDEX(5, 5)), "square legal move API includes legal capture");

    count = generate_legal_moves_for_square(INDEX(5, 5), WHITE, moves, 16);
    require(count == 0, "square legal move API rejects enemy piece for side");
}

int main(void)
{
    test_board_init();
    test_initial_white_moves();
    test_material_evaluation();
    test_pinned_piece_move_filtered();
    test_king_cannot_move_into_check();
    test_king_capture_rejected();
    test_game_status_checkmate();
    test_game_status_stalemate();
    test_game_status_normal_and_check();
    test_try_make_legal_move_allows_capture();
    test_try_make_legal_move_rejects_illegal_move();
    test_try_make_legal_move_promotes_to_queen();
    test_generate_legal_moves_for_square();

    puts("host chess tests passed");
    return 0;
}
