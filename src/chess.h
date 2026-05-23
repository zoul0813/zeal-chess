#include <stdbool.h>
#include <stdint.h>
#include <zos_video.h>

#define COLOR_LIGHT TEXT_COLOR_DARK_GRAY
#define COLOR_DARK  TEXT_COLOR_BROWN

#define EMPTY  0
#define PAWN   1
#define KNIGHT 2
#define BISHOP 3
#define ROOK   4
#define QUEEN  5
#define KING   6

#define PAWN_VALUE   100
#define KNIGHT_VALUE 320
#define BISHOP_VALUE 330
#define ROOK_VALUE   500
#define QUEEN_VALUE  900
#define KING_VALUE   20000

#define WHITE 8
#define BLACK 16

#define CHESS_DIR_LEFT  0
#define CHESS_DIR_RIGHT 1
#define CHESS_DIR_DOWN  2
#define CHESS_DIR_UP    3

#define INDEX(rank, file) (((rank) << 4) | (file))
#define GET_Y(pos)        ((pos) & 7)
#define GET_X(pos)        (((pos) >> 4) & 7)
#define IS_ON_BOARD(sq) (((sq) & 0x88) == 0)

typedef struct {
    uint8_t from;      // Source square (0-127 if using 0x88 board)
    uint8_t to;        // Destination square
    uint8_t piece;     // Piece being moved (optional but useful)
    uint8_t captured;  // Piece captured, if any (0 if none)
    uint8_t promotion;  // 0 if no promotion, else one of your piece codes (QUEEN, ROOK, etc.)
} Move;

typedef enum {
    GAME_STATUS_NORMAL,
    GAME_STATUS_CHECK,
    GAME_STATUS_CHECKMATE,
    GAME_STATUS_STALEMATE,
} GameStatus;


extern uint8_t *board; // 0x88 board, 16x8
extern uint8_t side_to_move;

void board_init(uint8_t *the_board);
char piece_char(uint8_t piece);
void print_board(void);

uint8_t human_move_turn(char* input);
uint8_t ai_move_turn(void);

uint8_t parse_file(char c);
uint8_t parse_rank(char c);
uint8_t parse_square(const char *s);

uint8_t is_friendly(uint8_t piece, uint8_t side);

uint8_t is_valid_move(uint8_t from, uint8_t to, uint8_t side);
uint8_t is_valid_pawn_move(uint8_t from, uint8_t to, uint8_t side);
uint8_t is_valid_knight_move(uint8_t from, uint8_t to, uint8_t side);
uint8_t is_valid_bishop_move(uint8_t from, uint8_t to, uint8_t side);
uint8_t is_valid_rook_move(uint8_t from, uint8_t to, uint8_t side);
uint8_t is_valid_queen_move(uint8_t from, uint8_t to, uint8_t side);
uint8_t is_valid_king_move(uint8_t from, uint8_t to, uint8_t side);

uint16_t generate_legal_moves(uint8_t side, Move moves[], uint16_t max_moves);
uint8_t try_make_legal_move(uint8_t from, uint8_t to, uint8_t side, Move *move);
uint16_t generate_legal_moves_for_square(uint8_t from, uint8_t side, Move moves[], uint16_t max_moves);
uint8_t find_legal_move_piece(uint8_t selected, uint8_t side, uint8_t dir);
void make_move(Move *m);
void undo_move(Move *m);
uint8_t pick_best_move(uint8_t side, Move* move);


bool is_in_check(uint8_t side);
bool has_legal_moves(uint8_t side);
GameStatus game_status(uint8_t side);
