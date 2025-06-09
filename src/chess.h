#include <stdbool.h>
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

#define INDEX(rank, file) (((rank) << 4) | (file))
#define IS_ON_BOARD(sq) (((sq) & 0x88) == 0)

typedef struct {
    unsigned char from;      // Source square (0-127 if using 0x88 board)
    unsigned char to;        // Destination square
    unsigned char piece;     // Piece being moved (optional but useful)
    unsigned char captured;  // Piece captured, if any (0 if none)
    unsigned char promotion;  // 0 if no promotion, else one of your piece codes (QUEEN, ROOK, etc.)
} Move;


extern unsigned char board[128]; // 0x88 board, 16x8
extern unsigned char side_to_move;

void init_board(void);
char piece_char(unsigned char piece);
void print_board(void);

int human_move_turn(char* input);
int ai_move_turn(void);

unsigned char parse_file(char c);
unsigned char parse_rank(char c);
unsigned char parse_square(const char *s);

int is_friendly(unsigned char piece, unsigned char side);

int is_valid_move(unsigned char from, unsigned char to, unsigned char side);
int is_valid_pawn_move(unsigned char from, unsigned char to, unsigned char side);
int is_valid_knight_move(unsigned char from, unsigned char to, unsigned char side);
int is_valid_bishop_move(unsigned char from, unsigned char to, unsigned char side);
int is_valid_rook_move(unsigned char from, unsigned char to, unsigned char side);
int is_valid_queen_move(unsigned char from, unsigned char to, unsigned char side);
int is_valid_king_move(unsigned char from, unsigned char to, unsigned char side);

int generate_legal_moves(unsigned char side, Move moves[], int max_moves);
void make_move(Move *m);
void undo_move(Move *m);
void pick_best_move(unsigned char side, Move* move);


bool is_in_check(unsigned char side);
bool has_legal_moves(unsigned char side);


