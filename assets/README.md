# Zeal Chess Assets

## Board

10 `16x16` tiles

The board is arranged in Rank (horiz) and File (vert)


8 rkbqkbkr
7 pppppppp
6 ........
5 ........
4 ........
3 ........
2 pppppppp
1 RKBQKBKR
  ABCDEFGH

## Pieces

6 pieces per side, 4 tiles per piece (32x32), 2 colors

tiles = 48 tiles `((6 * 4) * 2) = 48`
selection = 24 tiles `(6 * 4)`
total = 72 `(48 + 24)`

tiles are offset by 6px along the Y axis to center them onto the rotated board squares


## Current ZVB Layout

ZVB has a 16x16 pixel sprite, with a limit of 128 total sprites

With 72 tiles for the white/black pieces, that consumes the majority of the available sprites

The first 72 sprites will be allocated for pieces, which piece is associated with which sprite will
change dynamically, to force z-index ordering ... sprite tiles will be updated after each move,
processing them from "top to bottom, right to left"


When processing "top down, right to left" the board is traversed in this order

H8
H7, G8
H6, G7, F8
H5, G6, F7, E8
H4, G5, F6, E7, D8
H3, G4, F5, E6, D7, C8
H2, G3, F4, E5, D6, C7, B8
H1, G2, F3, E4, D5, C6, B7, A8
G1, F2, E3, D4, C5, B6, A7
F1, E2, D3, C4, B5, A6
E1, D2, C3, B4, A5
D1, C2, B3, A4
C1, B2, A3
B1, A2
A1

The first piece is always the last "sprite group"

A sprite group is the total sprites required to represent the piece (ie; 4)

We represent the pieces with a struct that looks similar to this:

```C
typedef struct {
    gfx_sprite_t tl;
    gfx_sprite_t tr;
    gfx_sprite_t bl;
    gfx_sprite_t br;
    union {
        struct {
            uint8_t rank; // x
            uint8_t file; // y
        }
        struct {
            uint8_t x; // rank
            uint8_t y; // file
        }
    }
} sprite_piece_t;
```

## New ZVB Layout

Future release of ZVB will support 16x32 sprites, reducing the overall number of total sprites 