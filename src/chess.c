#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <zos_errors.h>
#include <zos_video.h>
#include <zos_time.h>

#include "chess.h"
#include "view.h"
#include "conio.h"

/* FIXME: Why not have a 64-byte board? */
unsigned char board[128]; // 0x88 board, 16x8
static uint8_t gfx_board[128]; // 0x88 board, 16x8
unsigned char side_to_move = WHITE;

static void draw_pieces(void)
{
    /* Browse the board diagonally, from back to front */
    static const uint8_t indexes[] = {
        INDEX(7,7),
        INDEX(7,6), INDEX(6,7),
        INDEX(7,5), INDEX(6,6), INDEX(5,7),
        INDEX(7,4), INDEX(6,5), INDEX(5,6), INDEX(4,7),
        INDEX(7,3), INDEX(6,4), INDEX(5,5), INDEX(4,6), INDEX(3,7),
        INDEX(7,2), INDEX(6,3), INDEX(5,4), INDEX(4,5), INDEX(3,6), INDEX(2,7),
        INDEX(7,1), INDEX(6,2), INDEX(5,3), INDEX(4,4), INDEX(3,5), INDEX(2,6), INDEX(1,7),
        INDEX(7,0), INDEX(6,1), INDEX(5,2), INDEX(4,3), INDEX(3,4), INDEX(2,5), INDEX(1,6), INDEX(0,7),
        INDEX(6,0), INDEX(5,1), INDEX(4,2), INDEX(3,3), INDEX(2,4), INDEX(1,5), INDEX(0,6),
        INDEX(5,0), INDEX(4,1), INDEX(3,2), INDEX(2,3), INDEX(1,4), INDEX(0,5),
        INDEX(4,0), INDEX(3,1), INDEX(2,2), INDEX(1,3), INDEX(0,4),
        INDEX(3,0), INDEX(2,1), INDEX(1,2), INDEX(0,3),
        INDEX(2,0), INDEX(1,1), INDEX(0,2),
        INDEX(1,0), INDEX(0,1),
        INDEX(0,0)
    };

    view_clear_pieces();

    for (uint8_t i = 0; i < sizeof(indexes); i++) {
        const uint8_t pos = indexes[i];
        const uint8_t piece = board[pos];
        gfx_board[pos] = view_place_piece(GET_X(pos), GET_Y(pos), piece & 0x7, piece >> 3);
    }

    view_render_pieces();
}


void init_board(void)
{
    // Clear board
    for (int i = 0; i < 128; i++) {
        board[i] = EMPTY;
    }

    // White pieces (bottom)
    board[INDEX(0, 0)] = WHITE | ROOK;
    board[INDEX(0, 1)] = WHITE | KNIGHT;
    board[INDEX(0, 2)] = WHITE | BISHOP;
    board[INDEX(0, 3)] = WHITE | QUEEN;
    board[INDEX(0, 4)] = WHITE | KING;
    board[INDEX(0, 5)] = WHITE | BISHOP;
    board[INDEX(0, 6)] = WHITE | KNIGHT;
    board[INDEX(0, 7)] = WHITE | ROOK;
    for (int i = 0; i < 8; i++) {
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
    for (int i = 0; i < 8; i++) {
        board[INDEX(6, i)] = BLACK | PAWN;
    }

    /* Initialize the view */
    view_init();
    draw_pieces();

    /* Highlight one piece */
    for (uint8_t i = 0; i <= 7; i++) {
        view_select_piece(gfx_board[i]);
        msleep(500);
        view_deselect_piece(gfx_board[i]);
    }
}

char piece_char(unsigned char piece)
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

    for (int rank = 7; rank >= 0; rank--) {

        // print the vertical grid label
        bgcolor(TEXT_COLOR_BLACK);
        textcolor(TEXT_COLOR_LIGHT_GRAY);
        putchar(CH_SPACE); putchar(CH_SPACE); putchar(CH_SPACE);
        putchar(rank + 1 + 48);
        fflush_stdout();

        for (int file = 0; file < 8; file++) {
            unsigned char p = board[INDEX(rank, file)];
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


unsigned char parse_file(char c)
{
    if (c >= 'a' && c <= 'h')
        return c - 'a';
    return 0xFF;
}

unsigned char parse_rank(char c)
{
    if (c >= '1' && c <= '8')
        return c - '1';
    return 0xFF;
}

unsigned char parse_square(const char* s)
{
    unsigned char file = parse_file(s[0]);
    unsigned char rank = parse_rank(s[1]);
    if (file > 7 || rank > 7)
        return 0xFF;
    return INDEX(rank, file);
}

// Handles human (White) move input and execution
int human_move_turn(char* input)
{
    // Remove trailing newline if present
    int len = strlen(input);
    if (len > 0 && input[len - 1] == '\n') {
        input[len - 1] = '\0';
        len--;
    }

    if (strcmp(input, "quit") == 0) {
        printf("Exiting move loop.\n");
        return 0; // stop loop
    }

    if (len < 4) {
        printf("Invalid input. Please enter moves like 'e2e4'.\n");
        return 1; // continue loop
    }

    unsigned char from = parse_square(&input[0]);
    unsigned char to   = parse_square(&input[2]);

    if (!IS_ON_BOARD(from) || !IS_ON_BOARD(to)) {
        printf("Invalid square.\n");
        return 1;
    }

    unsigned char piece = board[from];

    if (piece == EMPTY) {
        printf("No piece at source square.\n");
        return 1;
    }

    if (!is_friendly(piece, side_to_move)) {
        printf("It's White's turn. You can't move that piece.\n");
        return 1;
    }

    if (!is_valid_move(from, to, side_to_move)) {
        printf("Invalid move for that piece.\n");
        return 1;
    }

    // Make the player's move
    board[to]   = piece;
    board[from] = EMPTY;

    // Pawn promotion check
    unsigned char side = piece & (WHITE | BLACK);
    int rank           = to >> 4;

    if ((piece & 7) == PAWN) {
        if ((side == WHITE && rank == 7) || (side == BLACK && rank == 0)) {
            board[to] = side | QUEEN; // Promote to queen
            printf("Pawn promoted to Queen!\n");
        }
    }

    return 1;
}

// Handles AI (Black) move generation and execution
int ai_move_turn(void)
{
    gotoxy(0, 15);
    puts("Black is thinking...");

    Move ai_move;
    pick_best_move(BLACK, &ai_move);

    if (ai_move.from == 0 && ai_move.to == 0) {
        printf("AI has no valid moves. Game over.\n");
        return 0; // stop loop
    }

    unsigned char piece = board[ai_move.from];

    // Make the AI move
    board[ai_move.to]   = piece;
    board[ai_move.from] = EMPTY;

    // Pawn promotion check for AI
    unsigned char side = piece & (WHITE | BLACK);
    int rank           = ai_move.to >> 4;

    if ((piece & 7) == PAWN) {
        if ((side == WHITE && rank == 7) || (side == BLACK && rank == 0)) {
            board[ai_move.to] = side | QUEEN;
            printf("AI pawn promoted to Queen!\n");
        }
    }

    printf("Black moved from %c%d to %c%d\n", 'a' + (ai_move.from & 7), 1 + (ai_move.from >> 4), 'a' + (ai_move.to & 7),
           1 + (ai_move.to >> 4));

    return 1;
}

int is_friendly(unsigned char piece, unsigned char side)
{
    return (piece & (WHITE | BLACK)) == side;
}

int is_valid_move(unsigned char from, unsigned char to, unsigned char side)
{
    unsigned char piece = board[from] & 7;

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


int is_valid_pawn_move(unsigned char from, unsigned char to, unsigned char side)
{
    unsigned char piece  = board[from];
    unsigned char target = board[to];

    int from_rank = from >> 4;
    int from_file = from & 0xF;
    int to_rank   = to >> 4;
    int to_file   = to & 0xF;

    int dir        = (side == WHITE) ? 1 : -1;
    int start_rank = (side == WHITE) ? 1 : 6;

    // Normal forward move
    if (to_file == from_file && target == EMPTY) {
        if (to_rank == from_rank + dir) {
            return 1; // single step
        }
        // double step from starting position
        if (from_rank == start_rank && to_rank == from_rank + 2 * dir) {
            // check if square in front is empty
            unsigned char between = INDEX(from_rank + dir, from_file);
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

int is_valid_knight_move(unsigned char from, unsigned char to, unsigned char side)
{
    unsigned char target = board[to];
    int from_rank        = from >> 4;
    int from_file        = from & 0xF;
    int to_rank          = to >> 4;
    int to_file          = to & 0xF;

    int dr = to_rank - from_rank;
    int df = to_file - from_file;

    // Can't capture own piece
    if (target != EMPTY && (target & (WHITE | BLACK)) == side)
        return 0;

    // Knight moves in L-shape
    if ((abs(dr) == 2 && abs(df) == 1) || (abs(dr) == 1 && abs(df) == 2)) {
        return 1;
    }

    return 0;
}

int is_valid_bishop_move(unsigned char from, unsigned char to, unsigned char side)
{
    unsigned char target = board[to];
    int from_rank        = from >> 4;
    int from_file        = from & 0xF;
    int to_rank          = to >> 4;
    int to_file          = to & 0xF;

    int dr = to_rank - from_rank;
    int df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side)
        return 0;

    if (abs(dr) == abs(df) && dr != 0) {
        int step_r = (dr > 0) ? 1 : -1;
        int step_f = (df > 0) ? 1 : -1;
        int r      = from_rank + step_r;
        int f      = from_file + step_f;
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

int is_valid_rook_move(unsigned char from, unsigned char to, unsigned char side)
{
    unsigned char target = board[to];
    int from_rank        = from >> 4;
    int from_file        = from & 0xF;
    int to_rank          = to >> 4;
    int to_file          = to & 0xF;

    int dr = to_rank - from_rank;
    int df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side)
        return 0;

    if ((dr == 0 && df != 0) || (dr != 0 && df == 0)) {
        int step_r = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
        int step_f = (df == 0) ? 0 : (df > 0 ? 1 : -1);
        int r      = from_rank + step_r;
        int f      = from_file + step_f;
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

int is_valid_queen_move(unsigned char from, unsigned char to, unsigned char side)
{
    // Queen combines rook and bishop moves
    return is_valid_rook_move(from, to, side) || is_valid_bishop_move(from, to, side);
}

int is_valid_king_move(unsigned char from, unsigned char to, unsigned char side)
{
    unsigned char target = board[to];
    int from_rank        = from >> 4;
    int from_file        = from & 0xF;
    int to_rank          = to >> 4;
    int to_file          = to & 0xF;

    int dr = to_rank - from_rank;
    int df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side)
        return 0;

    if (abs(dr) <= 1 && abs(df) <= 1) {
        return 1;
    }

    // TODO: castling rules (optional)

    return 0;
}

int generate_legal_moves(unsigned char side, Move moves[], int max_moves)
{
    int count = 0;
    for (unsigned char sq = 0; sq < 128; sq++) {
        if ((sq & 0x88) != 0)
            continue; // skip invalid squares
        unsigned char piece = board[sq];
        if (piece == EMPTY)
            continue;
        if ((piece & (WHITE | BLACK)) != side)
            continue;

        for (unsigned char to = 0; to < 128; to++) {
            if ((to & 0x88) != 0)
                continue;

            if (is_valid_move(sq, to, side)) {
                // Add promotion moves if pawn reaches last rank
                if ((piece & 7) == PAWN) {
                    int rank           = to >> 4;
                    int promotion_rank = (side == WHITE) ? 7 : 0;
                    if (rank == promotion_rank) {
                        // Generate moves for all promotion pieces
                        unsigned char promo_pieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
                        for (int i = 0; i < 4; i++) {
                            if (count >= max_moves)
                                return count;
                            moves[count].from      = sq;
                            moves[count].to        = to;
                            moves[count].promotion = promo_pieces[i];
                            count++;
                        }
                        continue;
                    }
                }

                if (count >= max_moves)
                    return count;
                moves[count].from      = sq;
                moves[count].to        = to;
                moves[count].promotion = 0;
                count++;
            }
        }
    }
    return count;
}

void make_move(Move* m)
{
    unsigned char piece = board[m->from];
    m->captured         = board[m->to]; // save captured piece for undo

    // Move piece to destination
    if (m->promotion) {
        // Replace pawn with promoted piece (keep side bits)
        unsigned char side = piece & (WHITE | BLACK);
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
    unsigned char piece = board[m->to];
    unsigned char side  = piece & (WHITE | BLACK);

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

int piece_value(unsigned char piece)
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

int evaluate_board(unsigned char side)
{
    int score = 0;

    for (unsigned char sq = 0; sq < 128; sq++) {
        if (sq & 0x88)
            continue;
        unsigned char piece = board[sq];
        if (piece == EMPTY)
            continue;

        int value = piece_value(piece);
        if ((piece & (WHITE | BLACK)) == side)
            score += value;
        else
            score -= value;
    }
    return score;
}

int file_char_to_index(char c)
{
    if (c >= 'a' && c <= 'h')
        return c - 'a';
    if (c >= 'A' && c <= 'H')
        return c - 'A';
    return -1;
}

int rank_char_to_index(char c)
{
    if (c >= '1' && c <= '8')
        return c - '1';
    return -1;
}

Move moves[256];
void pick_best_move(unsigned char side, Move* move)
{
    memset(moves, 0, sizeof(moves));
    Move best_move = {0, 0, 0, 0};
    int move_count = generate_legal_moves(side, moves, 256);

    if (move_count == 0) {
        // No legal moves, zero out move struct
        move->from     = 0;
        move->to       = 0;
        move->piece    = 0;
        move->captured = 0;
        return;
    }

    int best_score = -32767; // minimum safe value

    for (int i = 0; i < move_count; i++) {
        make_move(&moves[i]);
        int score = evaluate_board(side);
        undo_move(&moves[i]);

        if (score > best_score) {
            best_score = score;
            best_move  = moves[i];
        }
    }

    // Copy the best move found back to the pointer provided
    *move = best_move;
}


bool is_in_check(unsigned char side)
{
    unsigned char enemy       = (side == WHITE) ? BLACK : WHITE;
    unsigned char king_square = 0xFF;

    // Find the king's square
    for (int i = 0; i < 128; i++) {
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
    for (int i = 0; i < 128; i++) {
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

bool has_legal_moves(unsigned char side)
{
    Move moves[256];
    int count = generate_legal_moves(side, moves, 256);

    for (int i = 0; i < count; i++) {
        make_move(&moves[i]);
        bool legal = !is_in_check(side);
        undo_move(&moves[i]);
        if (legal)
            return true;
    }

    return false;
}
