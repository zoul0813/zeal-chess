#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "chess.h"
#include "input.h"

unsigned char board[128]; // 0x88 board, 16x8
unsigned char side_to_move         = WHITE;
const signed char knight_offsets[] = {-33, -31, -18, -14, 14, 18, 31, 33};
const signed char king_offsets[]   = {-17, -16, -15, -1, 1, 15, 16, 17};

int evaluate(void)
{
    int score = 0;
    for (int i = 0; i < 128; i++) {
        if (IS_ON_BOARD(i)) {
            switch (board[i] & 7) {
                case PAWN: score += (board[i] & WHITE) ? 10 : -10; break;
                case KNIGHT: score += (board[i] & WHITE) ? 30 : -30; break;
                case BISHOP: score += (board[i] & WHITE) ? 30 : -30; break;
                case ROOK: score += (board[i] & WHITE) ? 50 : -50; break;
                case QUEEN: score += (board[i] & WHITE) ? 90 : -90; break;
                case KING: score += (board[i] & WHITE) ? 900 : -900; break;
            }
        }
    }
    return score;
}

int negamax(int depth)
{
    if (depth == 0)
        return evaluate();
    int max = -9999;
    // For each legal move:
    //   make_move();
    //   int score = -negamax(depth - 1);
    //   undo_move();
    //   if (score > max) max = score;
    return max;
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
        default: return '.';
    }
}

void print_board(void)
{
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            unsigned char p = board[INDEX(rank, file)];
            char c          = piece_char(p);
            if (p & BLACK)
                c += 32; // lowercase for black
            putchar(c);
        }
        putchar('\n');
    }
    printf("  abcdefgh\n");
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
int human_move_turn(void) {
    char input[16];

    printf("\nWhite to move. Enter move (e.g. e2e4) or 'quit':\n");
    if (!fgets(input, sizeof(input), DEV_STDIN)) {
        printf("Input error or EOF\n");
        return 0; // stop loop
    }

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
    unsigned char to = parse_square(&input[2]);

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
    board[to] = piece;
    board[from] = EMPTY;

    // Pawn promotion check
    unsigned char side = piece & (WHITE | BLACK);
    int rank = to >> 4;

    if ((piece & 7) == PAWN) {
        if ((side == WHITE && rank == 7) || (side == BLACK && rank == 0)) {
            board[to] = side | QUEEN;  // Promote to queen
            printf("Pawn promoted to Queen!\n");
        }
    }

    print_board();

    // Switch side
    side_to_move = BLACK;
    return 1;
}

// Handles AI (Black) move generation and execution
int ai_move_turn(void) {
    printf("\nBlack (AI) thinking...\n");

    Move ai_move;
    pick_best_move(BLACK, &ai_move);

    if (ai_move.from == 0 && ai_move.to == 0) {
        printf("AI has no valid moves. Game over.\n");
        return 0; // stop loop
    }

    unsigned char piece = board[ai_move.from];

    // Make the AI move
    board[ai_move.to] = piece;
    board[ai_move.from] = EMPTY;

    // Pawn promotion check for AI
    unsigned char side = piece & (WHITE | BLACK);
    int rank = ai_move.to >> 4;

    if ((piece & 7) == PAWN) {
        if ((side == WHITE && rank == 7) || (side == BLACK && rank == 0)) {
            board[ai_move.to] = side | QUEEN;
            printf("AI pawn promoted to Queen!\n");
        }
    }

    printf("AI moves from %c%d to %c%d\n",
           'a' + (ai_move.from & 7), 1 + (ai_move.from >> 4),
           'a' + (ai_move.to & 7), 1 + (ai_move.to >> 4));

    print_board();

    // Switch side
    side_to_move = WHITE;
    return 1;
}

// Main game loop
void move_input_loop(void) {
    while (1) {
        int continue_loop = 0;

        if (side_to_move == WHITE) {
            continue_loop = human_move_turn();
        } else {
            continue_loop = ai_move_turn();
        }

        if (!continue_loop) break;
    }
}



int is_friendly(unsigned char piece, unsigned char side)
{
    return (piece & (WHITE | BLACK)) == side;
}

int is_valid_move(unsigned char from, unsigned char to, unsigned char side) {
    unsigned char piece = board[from] & 7;

    switch (piece) {
        case PAWN:   return is_valid_pawn_move(from, to, side);
        case KNIGHT: return is_valid_knight_move(from, to, side);
        case BISHOP: return is_valid_bishop_move(from, to, side);
        case ROOK:   return is_valid_rook_move(from, to, side);
        case QUEEN:  return is_valid_queen_move(from, to, side);
        case KING:   return is_valid_king_move(from, to, side);
        default:     return 0;
    }
}


int is_valid_pawn_move(unsigned char from, unsigned char to, unsigned char side) {
    unsigned char piece = board[from];
    unsigned char target = board[to];

    int from_rank = from >> 4;
    int from_file = from & 0xF;
    int to_rank = to >> 4;
    int to_file = to & 0xF;

    int dir = (side == WHITE) ? 1 : -1;
    int start_rank = (side == WHITE) ? 1 : 6;

    // Normal forward move
    if (to_file == from_file && target == EMPTY) {
        if (to_rank == from_rank + dir) {
            return 1;  // single step
        }
        // double step from starting position
        if (from_rank == start_rank && to_rank == from_rank + 2 * dir) {
            // check if square in front is empty
            unsigned char between = INDEX(from_rank + dir, from_file);
            if (board[between] == EMPTY) return 1;
        }
        return 0;
    }

    // Capture diagonally
    if ((to_file == from_file + 1 || to_file == from_file -1) && to_rank == from_rank + dir) {
        if (target != EMPTY && (target & (WHITE | BLACK)) != side) {
            return 1;
        }
        // TODO: en passant capture (optional)
    }

    return 0;
}

int is_valid_knight_move(unsigned char from, unsigned char to, unsigned char side) {
    unsigned char target = board[to];
    int from_rank = from >> 4;
    int from_file = from & 0xF;
    int to_rank = to >> 4;
    int to_file = to & 0xF;

    int dr = to_rank - from_rank;
    int df = to_file - from_file;

    // Can't capture own piece
    if (target != EMPTY && (target & (WHITE | BLACK)) == side) return 0;

    // Knight moves in L-shape
    if ((abs(dr) == 2 && abs(df) == 1) || (abs(dr) == 1 && abs(df) == 2)) {
        return 1;
    }

    return 0;
}

int is_valid_bishop_move(unsigned char from, unsigned char to, unsigned char side) {
    unsigned char target = board[to];
    int from_rank = from >> 4;
    int from_file = from & 0xF;
    int to_rank = to >> 4;
    int to_file = to & 0xF;

    int dr = to_rank - from_rank;
    int df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side) return 0;

    if (abs(dr) == abs(df) && dr != 0) {
        int step_r = (dr > 0) ? 1 : -1;
        int step_f = (df > 0) ? 1 : -1;
        int r = from_rank + step_r;
        int f = from_file + step_f;
        while (r != to_rank && f != to_file) {
            if (board[INDEX(r, f)] != EMPTY) return 0;
            r += step_r;
            f += step_f;
        }
        return 1;
    }
    return 0;
}

int is_valid_rook_move(unsigned char from, unsigned char to, unsigned char side) {
    unsigned char target = board[to];
    int from_rank = from >> 4;
    int from_file = from & 0xF;
    int to_rank = to >> 4;
    int to_file = to & 0xF;

    int dr = to_rank - from_rank;
    int df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side) return 0;

    if ((dr == 0 && df != 0) || (dr != 0 && df == 0)) {
        int step_r = (dr == 0) ? 0 : (dr > 0 ? 1 : -1);
        int step_f = (df == 0) ? 0 : (df > 0 ? 1 : -1);
        int r = from_rank + step_r;
        int f = from_file + step_f;
        while (r != to_rank || f != to_file) {
            if (board[INDEX(r, f)] != EMPTY) return 0;
            r += step_r;
            f += step_f;
        }
        return 1;
    }
    return 0;
}

int is_valid_queen_move(unsigned char from, unsigned char to, unsigned char side) {
    // Queen combines rook and bishop moves
    return is_valid_rook_move(from, to, side) || is_valid_bishop_move(from, to, side);
}

int is_valid_king_move(unsigned char from, unsigned char to, unsigned char side) {
    unsigned char target = board[to];
    int from_rank = from >> 4;
    int from_file = from & 0xF;
    int to_rank = to >> 4;
    int to_file = to & 0xF;

    int dr = to_rank - from_rank;
    int df = to_file - from_file;

    if (target != EMPTY && (target & (WHITE | BLACK)) == side) return 0;

    if (abs(dr) <= 1 && abs(df) <= 1) {
        return 1;
    }

    // TODO: castling rules (optional)

    return 0;
}

int generate_legal_moves(unsigned char side, Move moves[], int max_moves) {
    int count = 0;
    for (unsigned char sq = 0; sq < 128; sq++) {
        if ((sq & 0x88) != 0) continue;  // skip invalid squares
        unsigned char piece = board[sq];
        if (piece == EMPTY) continue;
        if ((piece & (WHITE | BLACK)) != side) continue;

        for (unsigned char to = 0; to < 128; to++) {
            if ((to & 0x88) != 0) continue;

            if (is_valid_move(sq, to, side)) {
                // Add promotion moves if pawn reaches last rank
                if ((piece & 7) == PAWN) {
                    int rank = to >> 4;
                    int promotion_rank = (side == WHITE) ? 7 : 0;
                    if (rank == promotion_rank) {
                        // Generate moves for all promotion pieces
                        unsigned char promo_pieces[] = {QUEEN, ROOK, BISHOP, KNIGHT};
                        for (int i = 0; i < 4; i++) {
                            if (count >= max_moves) return count;
                            moves[count].from = sq;
                            moves[count].to = to;
                            moves[count].promotion = promo_pieces[i];
                            count++;
                        }
                        continue;
                    }
                }

                if (count >= max_moves) return count;
                moves[count].from = sq;
                moves[count].to = to;
                moves[count].promotion = 0;
                count++;
            }
        }
    }
    return count;
}

void make_move(Move *m) {
    unsigned char piece = board[m->from];
    m->captured = board[m->to];  // save captured piece for undo

    // Move piece to destination
    if (m->promotion) {
        // Replace pawn with promoted piece (keep side bits)
        unsigned char side = piece & (WHITE | BLACK);
        board[m->to] = m->promotion | side;
    } else {
        board[m->to] = piece;
    }

    // Clear source square
    board[m->from] = EMPTY;

    // TODO: Update castling rights, en passant, halfmove clock if needed
}

void undo_move(Move *m) {
    unsigned char piece = board[m->to];
    unsigned char side = piece & (WHITE | BLACK);

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

int piece_value(unsigned char piece) {
    switch (piece & 7) {
        case PAWN:   return PAWN_VALUE;
        case KNIGHT: return KNIGHT_VALUE;
        case BISHOP: return BISHOP_VALUE;
        case ROOK:   return ROOK_VALUE;
        case QUEEN:  return QUEEN_VALUE;
        case KING:   return KING_VALUE;
        default:     return 0;
    }
}

int evaluate_board(unsigned char side) {
    int score = 0;

    for (unsigned char sq = 0; sq < 128; sq++) {
        if (sq & 0x88) continue;
        unsigned char piece = board[sq];
        if (piece == EMPTY) continue;

        int value = piece_value(piece);
        if ((piece & (WHITE | BLACK)) == side)
            score += value;
        else
            score -= value;
    }
    return score;
}

int file_char_to_index(char c) {
    if (c >= 'a' && c <= 'h') return c - 'a';
    if (c >= 'A' && c <= 'H') return c - 'A';
    return -1;
}

int rank_char_to_index(char c) {
    if (c >= '1' && c <= '8') return c - '1';
    return -1;
}

// Parses move string into Move struct
// Returns 1 on success, 0 on failure
int parse_move(const char *str, Move *move) {
    if (!str || !move) return 0;

    // Expected formats:
    // e2e4 (4 chars) or e7e8q (5 chars promotion)

    int len = 0;
    while (str[len] != '\0' && len < 6) len++;

    if (len < 4) return 0;

    int from_file = file_char_to_index(str[0]);
    int from_rank = rank_char_to_index(str[1]);
    int to_file = file_char_to_index(str[2]);
    int to_rank = rank_char_to_index(str[3]);

    if (from_file < 0 || from_rank < 0 || to_file < 0 || to_rank < 0)
        return 0;

    // Convert to 0x88 index
    unsigned char from_sq = (from_rank << 4) | from_file;
    unsigned char to_sq = (to_rank << 4) | to_file;

    move->from = from_sq;
    move->to = to_sq;
    move->promotion = 0;

    if (len == 5) {
        // Promotion character: q,r,b,n
        char promo = tolower(str[4]);
        switch (promo) {
            case 'q': move->promotion = QUEEN; break;
            case 'r': move->promotion = ROOK; break;
            case 'b': move->promotion = BISHOP; break;
            case 'n': move->promotion = KNIGHT; break;
            default: return 0;
        }
    }

    return 1;
}

void pick_best_move(unsigned char side, Move* move) {
    Move moves[256];
    Move best_move = {0, 0, 0, 0};
    int move_count = generate_legal_moves(side, moves, 256);

    if (move_count == 0) {
        // No legal moves, zero out move struct
        move->from = 0;
        move->to = 0;
        move->piece = 0;
        move->captured = 0;
        return;
    }

    int best_score = -999999;

    for (int i = 0; i < move_count; i++) {
        make_move(&moves[i]);
        int score = evaluate_board(side);
        undo_move(&moves[i]);

        if (score > best_score) {
            best_score = score;
            best_move = moves[i];
        }
    }

    // Copy the best move found back to the pointer provided
    *move = best_move;
}
