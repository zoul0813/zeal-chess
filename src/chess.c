#include <stdio.h>
#include <string.h>

#include <zos_video.h>

#include <conio.h>

#include "chess.h"

/* FIXME: Why not have a 64-byte board? */
uint8_t *board;

int fflush_stdout(void);

static uint16_t u16_abs(int16_t value)
{
    return value < 0 ? (uint16_t)-value : (uint16_t)value;
}

static uint8_t is_promotion_square(uint8_t piece, uint8_t to, uint8_t side)
{
    if ((piece & 7) != PAWN)
        return 0;

    uint8_t rank = to >> 4;
    uint8_t promotion_rank = (side == WHITE) ? 7 : 0;
    return rank == promotion_rank;
}

static uint8_t build_legal_move(uint8_t from, uint8_t to, uint8_t side, Move *move)
{
    if (!IS_ON_BOARD(from) || !IS_ON_BOARD(to))
        return 0;

    uint8_t piece = board[from];
    if (piece == EMPTY || !is_friendly(piece, side))
        return 0;

    if (!is_valid_move(from, to, side))
        return 0;

    uint8_t target = board[to];
    if ((target & 7) == KING && !is_friendly(target, side))
        return 0;

    Move candidate;
    candidate.from      = from;
    candidate.to        = to;
    candidate.piece     = piece;
    candidate.captured  = 0;
    candidate.promotion = is_promotion_square(piece, to, side) ? QUEEN : 0;

    make_move(&candidate);
    uint8_t legal = !is_in_check(side);
    undo_move(&candidate);

    if (!legal)
        return 0;

    if (move)
        *move = candidate;

    return 1;
}

void board_init(uint8_t *the_board)
{
    board = the_board;

    // Clear board
    memset(board, EMPTY, 128);

    // White pieces (bottom)
    board[INDEX(0, 0)] = WHITE | ROOK;
    board[INDEX(0, 1)] = WHITE | KNIGHT;
    board[INDEX(0, 2)] = WHITE | BISHOP;
    board[INDEX(0, 3)] = WHITE | QUEEN;
    board[INDEX(0, 4)] = WHITE | KING;
    board[INDEX(0, 5)] = WHITE | BISHOP;
    board[INDEX(0, 6)] = WHITE | KNIGHT;
    board[INDEX(0, 7)] = WHITE | ROOK;
    for (uint8_t i = 0; i < 8; i++) {
        board[INDEX(1, i)] = WHITE | PAWN;
    }

    // Black pieces (top)
    board[INDEX(7, 0)] = BLACK | ROOK;
    board[INDEX(7, 1)] = BLACK | KNIGHT;
    board[INDEX(7, 2)] = BLACK | BISHOP;
    board[INDEX(7, 3)] = BLACK | QUEEN;
    board[INDEX(7, 4)] = BLACK | KING;
    board[INDEX(7, 5)] = BLACK | BISHOP;
    board[INDEX(7, 6)] = BLACK | KNIGHT;
    board[INDEX(7, 7)] = BLACK | ROOK;
    for (uint8_t i = 0; i < 8; i++) {
        board[INDEX(6, i)] = BLACK | PAWN;
    }
}

char piece_char(uint8_t piece)
{
    switch (piece & 7) {
        case PAWN: return 'P';
        case KNIGHT: return 'N';
        case BISHOP: return 'B';
        case ROOK: return 'R';
        case QUEEN: return 'Q';
        case KING: return 'K';
        default: return ' ';
    }
}

void print_board(void)
{
    gotoxy(0,0);
    uint8_t color = COLOR_LIGHT;

    // print the horizontal grid label
    bgcolor(TEXT_COLOR_BLACK);
    textcolor(TEXT_COLOR_LIGHT_GRAY);
    puts("\n    abcdefgh");

    for (int16_t rank = 7; rank >= 0; rank--) {

        // print the vertical grid label
        bgcolor(TEXT_COLOR_BLACK);
        textcolor(TEXT_COLOR_LIGHT_GRAY);
        putchar(CH_SPACE); putchar(CH_SPACE); putchar(CH_SPACE);
        putchar(rank + 1 + 48);
        fflush_stdout();

        for (uint8_t file = 0; file < 8; file++) {
            uint8_t p = board[INDEX(rank, file)];
            char c          = piece_char(p);
            if (p & BLACK) {
                c += 32; // lowercase for black
                textcolor(TEXT_COLOR_BLACK);
            } else {
                textcolor(TEXT_COLOR_WHITE);
            }

            bgcolor(color);
            putchar(c);
            fflush_stdout();

            // alternate board cell colors
            if (color == COLOR_LIGHT) color = COLOR_DARK;
            else color = COLOR_LIGHT;
        }
        fflush_stdout();

        // print the vertical grid label
        bgcolor(TEXT_COLOR_BLACK);
        textcolor(TEXT_COLOR_LIGHT_GRAY);
        putchar(rank + 1 + 48);
        fflush_stdout();

        // next row, reset bgcolor since ZVB will clear the line to bgcolor
        bgcolor(TEXT_COLOR_BLACK);
        putchar(CH_NEWLINE);

        // alternate board cell colors for next row
        if (color == COLOR_LIGHT) color = COLOR_DARK;
        else color = COLOR_LIGHT;
    }

    // print the horizontal grid label
    bgcolor(TEXT_COLOR_BLACK);
    textcolor(TEXT_COLOR_LIGHT_GRAY);
    puts("    abcdefgh");
}


uint8_t make_black_ai_reply(Move *move)
{
    GameStatus status = game_status(BLACK);
    if (status == GAME_STATUS_CHECKMATE || status == GAME_STATUS_STALEMATE) {
        return 0; // stop loop
    }

    Move ai_move;
    if (!pick_best_move(BLACK, &ai_move)) {
        return 0; // stop loop
    }

    make_move(&ai_move);
    if (move)
        *move = ai_move;

    return 1;
}

uint8_t is_friendly(uint8_t piece, uint8_t side)
{
    return (piece & (WHITE | BLACK)) == side;
}

uint8_t is_valid_move(uint8_t from, uint8_t to, uint8_t side)
{
    uint8_t piece = board[from] & 7;

    switch (piece) {
        case PAWN: return is_valid_pawn_move(from, to, side);
        case KNIGHT: return is_valid_knight_move(from, to, side);
        case BISHOP: return is_valid_bishop_move(from, to, side);
        case ROOK: return is_valid_rook_move(from, to, side);
        case QUEEN: return is_valid_queen_move(from, to, side);
        case KING: return is_valid_king_move(from, to, side);
        default: return 0;
    }
}


uint8_t is_valid_pawn_move(uint8_t from, uint8_t to, uint8_t side)
{
    uint8_t target = board[to];

    int16_t from_rank = from >> 4;
    int16_t from_file = from & 0xF;
    int16_t to_rank   = to >> 4;
    int16_t to_file   = to & 0xF;

    int16_t dir        = (side == WHITE) ? 1 : -1;
    int16_t start_rank = (side == WHITE) ? 1 : 6;

    // Normal forward move
    if (to_file == from_file && target == EMPTY) {
        if (to_rank == from_rank + dir) {
            return 1; // single step
        }
        // double step from starting position
        if (from_rank == start_rank && to_rank == from_rank + 2 * dir) {
            // check if square in front is empty
            uint8_t between = INDEX(from_rank + dir, from_file);
            if (board[between] == EMPTY)
                return 1;
        }
        return 0;
    }

    // Capture diagonally
    if ((to_file == from_file + 1 || to_file == from_file - 1) && to_rank == from_rank + dir) {
        if (target != EMPTY && (target & (WHITE | BLACK)) != side) {
            return 1;
        }
        // TODO: en passant capture (optional)
    }

    return 0;
}

uint8_t is_valid_knight_move(uint8_t from, uint8_t to, uint8_t side)
{
    uint8_t target    = board[to];
    int16_t from_rank = from >> 4;
    int16_t from_file = from & 0xF;
    int16_t to_rank   = to >> 4;
    int16_t to_file   = to & 0xF;

    int16_t dr = to_rank - from_rank;
    int16_t df = to_file - from_file;

    // Can't capture own piece
    if (target != EMPTY && (target & (WHITE | BLACK)) == side)
        return 0;

    // Knight moves in L-shape
    if ((u16_abs(dr) == 2 && u16_abs(df) == 1) || (u16_abs(dr) == 1 && u16_abs(df) == 2)) {
        return 1;
    }

    return 0;
}

uint8_t is_valid_bishop_move(uint8_t from, uint8_t to, uint8_t side)
{
    uint8_t target    = board[to];
    int16_t from_rank = from >> 4;
    int16_t from_file = from & 0xF;
    int16_t to_rank   = to >> 4;
    int16_t to_file   = to & 0xF;

    int16_t dr = to_rank - from_rank;
    int16_t df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side)
        return 0;

    if (u16_abs(dr) == u16_abs(df) && dr != 0) {
        int16_t step_r = (dr > 0) ? 1 : -1;
        int16_t step_f = (df > 0) ? 1 : -1;
        int16_t r      = from_rank + step_r;
        int16_t f      = from_file + step_f;
        while (r != to_rank && f != to_file) {
            if (board[INDEX(r, f)] != EMPTY)
                return 0;
            r += step_r;
            f += step_f;
        }
        return 1;
    }
    return 0;
}

uint8_t is_valid_rook_move(uint8_t from, uint8_t to, uint8_t side)
{
    uint8_t target    = board[to];
    int16_t from_rank = from >> 4;
    int16_t from_file = from & 0xF;
    int16_t to_rank   = to >> 4;
    int16_t to_file   = to & 0xF;

    int16_t dr = to_rank - from_rank;
    int16_t df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side)
        return 0;

    if ((dr == 0 && df != 0) || (dr != 0 && df == 0)) {
        int16_t step_r = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
        int16_t step_f = (df == 0) ? 0 : (df > 0 ? 1 : -1);
        int16_t r      = from_rank + step_r;
        int16_t f      = from_file + step_f;
        while (r != to_rank || f != to_file) {
            if (board[INDEX(r, f)] != EMPTY)
                return 0;
            r += step_r;
            f += step_f;
        }
        return 1;
    }
    return 0;
}

uint8_t is_valid_queen_move(uint8_t from, uint8_t to, uint8_t side)
{
    // Queen combines rook and bishop moves
    return is_valid_rook_move(from, to, side) || is_valid_bishop_move(from, to, side);
}

uint8_t is_valid_king_move(uint8_t from, uint8_t to, uint8_t side)
{
    uint8_t target    = board[to];
    int16_t from_rank = from >> 4;
    int16_t from_file = from & 0xF;
    int16_t to_rank   = to >> 4;
    int16_t to_file   = to & 0xF;

    int16_t dr = to_rank - from_rank;
    int16_t df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side)
        return 0;

    if (u16_abs(dr) <= 1 && u16_abs(df) <= 1) {
        return 1;
    }

    // TODO: castling rules (optional)

    return 0;
}

uint16_t generate_legal_moves(uint8_t side, Move moves[], uint16_t max_moves)
{
    uint16_t count = 0;
    for (uint8_t sq = 0; sq < 128; sq++) {
        if ((sq & 0x88) != 0)
            continue; // skip invalid squares
        uint8_t piece = board[sq];
        if (piece == EMPTY)
            continue;
        if ((piece & (WHITE | BLACK)) != side)
            continue;

        for (uint8_t to = 0; to < 128; to++) {
            if ((to & 0x88) != 0)
                continue;

            Move move;
            if (build_legal_move(sq, to, side, &move)) {
                if (count >= max_moves)
                    return count;
                moves[count] = move;
                count++;
            }
        }
    }
    return count;
}

uint8_t try_make_legal_move(uint8_t from, uint8_t to, uint8_t side, Move *move)
{
    Move legal_move;

    if (!build_legal_move(from, to, side, &legal_move))
        return 0;

    make_move(&legal_move);
    if (move)
        *move = legal_move;

    return 1;
}

uint16_t generate_legal_moves_for_square(uint8_t from, uint8_t side, Move moves[], uint16_t max_moves)
{
    uint16_t count = 0;

    for (uint8_t to = 0; to < 128; to++) {
        if (!IS_ON_BOARD(to))
            continue;

        Move move;
        if (build_legal_move(from, to, side, &move)) {
            if (count >= max_moves)
                return count;
            moves[count] = move;
            count++;
        }
    }

    return count;
}

uint8_t find_legal_move_piece(uint8_t selected, uint8_t side, uint8_t dir)
{
    uint8_t row = IS_ON_BOARD(selected) ? (selected & 0x70) : 0;
    uint8_t col = IS_ON_BOARD(selected) ? (selected & 0x07) : 0;
    Move moves[1];

    for (uint8_t i = 0; i < 64; i++) {
        if      (dir == CHESS_DIR_LEFT)  col = (col - 1) & 0x7;
        else if (dir == CHESS_DIR_RIGHT) col = (col + 1) & 0x7;
        else if (dir == CHESS_DIR_DOWN)  row = (row - 0x10) & 0x70;
        else if (dir == CHESS_DIR_UP)    row = (row + 0x10) & 0x70;

        uint8_t coord = row | col;
        uint8_t piece = board[coord];
        if (piece != EMPTY && is_friendly(piece, side) &&
            generate_legal_moves_for_square(coord, side, moves, 1) > 0) {
            return coord;
        }
    }

    return 0xff;
}

void make_move(Move* m)
{
    uint8_t piece = board[m->from];
    m->captured         = board[m->to]; // save captured piece for undo

    // Move piece to destination
    if (m->promotion) {
        // Replace pawn with promoted piece (keep side bits)
        uint8_t side = piece & (WHITE | BLACK);
        board[m->to]       = m->promotion | side;
    } else {
        board[m->to] = piece;
    }

    // Clear source square
    board[m->from] = EMPTY;

    // TODO: Update castling rights, en passant, halfmove clock if needed
}

void undo_move(Move* m)
{
    uint8_t piece = board[m->to];
    uint8_t side  = piece & (WHITE | BLACK);

    // Restore original piece at source (pawn if promoted)
    if (m->promotion) {
        // Undo promotion: put pawn back
        board[m->from] = PAWN | side;
    } else {
        board[m->from] = piece;
    }

    // Restore captured piece at destination
    board[m->to] = m->captured;
}

uint16_t piece_value(uint8_t piece)
{
    switch (piece & 7) {
        case PAWN: return PAWN_VALUE;
        case KNIGHT: return KNIGHT_VALUE;
        case BISHOP: return BISHOP_VALUE;
        case ROOK: return ROOK_VALUE;
        case QUEEN: return QUEEN_VALUE;
        case KING: return KING_VALUE;
        default: return 0;
    }
}

int16_t evaluate_board(uint8_t side)
{
    int16_t score = 0;

    for (uint8_t sq = 0; sq < 128; sq++) {
        if (sq & 0x88)
            continue;
        uint8_t piece = board[sq];
        if (piece == EMPTY)
            continue;

        uint16_t value = piece_value(piece);
        if ((piece & (WHITE | BLACK)) == side)
            score += value;
        else
            score -= value;
    }
    return score;
}

static Move ai_moves[256];
static Move ai_reply_moves[256];
uint8_t pick_best_move(uint8_t side, Move* move)
{
    Move best_move = {0, 0, 0, 0, 0};
    uint8_t enemy = (side == WHITE) ? BLACK : WHITE;

    memset(ai_moves, 0, sizeof(ai_moves));
    uint16_t move_count = generate_legal_moves(side, ai_moves, 256);

    if (move_count == 0) {
        if (move)
            *move = best_move;
        return 0;
    }

    int16_t best_score = -32767; // minimum safe value

    for (uint16_t i = 0; i < move_count; i++) {
        make_move(&ai_moves[i]);

        uint16_t reply_count = generate_legal_moves(enemy, ai_reply_moves, 256);
        int16_t score;

        if (reply_count == 0) {
            score = evaluate_board(side);
        } else {
            score = 32767;
            for (uint16_t j = 0; j < reply_count; j++) {
                make_move(&ai_reply_moves[j]);
                int16_t reply_score = evaluate_board(side);
                undo_move(&ai_reply_moves[j]);

                if (reply_score < score)
                    score = reply_score;
            }
        }

        undo_move(&ai_moves[i]);

        if (score > best_score) {
            best_score = score;
            best_move  = ai_moves[i];
        }
    }

    // Copy the best move found back to the pointer provided
    if (move)
        *move = best_move;

    return 1;
}


bool is_in_check(uint8_t side)
{
    uint8_t enemy       = (side == WHITE) ? BLACK : WHITE;
    uint8_t king_square = 0xFF;

    // Find the king's square
    for (uint8_t i = 0; i < 128; i++) {
        if (!IS_ON_BOARD(i))
            continue;
        if ((board[i] & 7) == KING && is_friendly(board[i], side)) {
            king_square = i;
            break;
        }
    }

    if (king_square == 0xFF)
        return true; // king not found — assume check

    // See if any enemy piece can move to the king
    for (uint8_t i = 0; i < 128; i++) {
        if (!IS_ON_BOARD(i))
            continue;
        if (board[i] != EMPTY && is_friendly(board[i], enemy)) {
            if (is_valid_move(i, king_square, enemy)) {
                return true;
            }
        }
    }

    return false;
}

bool has_legal_moves(uint8_t side)
{
    Move moves[256];
    uint16_t count = generate_legal_moves(side, moves, 256);
    return count > 0;
}

GameStatus game_status(uint8_t side)
{
    bool in_check = is_in_check(side);

    if (has_legal_moves(side)) {
        return in_check ? GAME_STATUS_CHECK : GAME_STATUS_NORMAL;
    }

    return in_check ? GAME_STATUS_CHECKMATE : GAME_STATUS_STALEMATE;
}
