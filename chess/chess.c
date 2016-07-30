#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "dbg.h"
#include <limits.h>
#include <stdio.h>
#include <errno.h>



// Board representations with rank / file 0d out to ease computation of overflows
uint64_t rank_8 = 0x00FFFFFFFFFFFFFF;
uint64_t rank_7 = 0xFF00FFFFFFFFFFFF;
uint64_t rank_1 = 0xFFFFFFFFFFFFFF00;
uint64_t rank_2 = 0xFFFFFFFFFFFF00FF;
uint64_t file_a = 0xFEFEFEFEFEFEFEFE;
uint64_t file_b = 0xFDFDFDFDFDFDFDFD;
uint64_t file_h = 0x7F7F7F7F7F7F7F7F;
uint64_t file_g = 0xBFBFBFBFBFBFBFBF;

/*
 * Struct containing current board state. 
 * Uses bitbaord representation to store location of each piece. 
 * Each "board" is a 64 bit int with each bit denoting if that type of 
 * exists in that location. Bit 0 corresponds to the bottom left corner
 * and bit 63 corresponds to the top right corner. 
 */

typedef struct board {
    uint64_t pawn_w;
    uint64_t pawn_b;
    uint64_t queen_w;
    uint64_t queen_b;
    uint64_t king_w;
    uint64_t king_b;
    uint64_t rook_w;
    uint64_t rook_b;
    uint64_t knight_w;
    uint64_t knight_b;
    uint64_t bishop_w;
    uint64_t bishop_b;

    uint64_t black;
    uint64_t white;

    // True if either rook and king is in correct position to castle.
    // Does not track whether there are pieces obstructing 
    // l and r denote castle that is a left or right shift from the repective king
    int castle_w_l;
    int castle_w_r;
    int castle_b_l;
    int castle_b_r;

    // 1 if white's turn, 0 if black's
    int turn; 

} board;

board* board_alloc() {
    board *b = malloc(sizeof(board));
    if (!b) {
        printf("Board failed to allocate\n");
        return NULL;
    }
    return b;
}

void board_delete(board *b) {
    free(b);
}

void make_move_b(uint64_t from, uint64_t to, board* b);
void make_move_w(uint64_t from, uint64_t to, board* b);
/* 
 * Returns all pieces for currently active side. 
*/
uint64_t get_curr_side(board *b) {
    return (b->turn) ? b->white: b->black;
}

/* 
 * Returns all pieces for opposing side.
*/
uint64_t get_opp_side(board *b) {
    return (!b->turn) ? b->white: b->black;
}

/* 
 * Fills each side, white and black, in bitboard with respective sides pieces. 
*/
void set_sides(board *b) {
    b->white = b->pawn_w | b->queen_w | b->king_w | b->bishop_w | b->knight_w | b->rook_w;
    b->black = b->pawn_b | b->queen_b | b->king_b | b->bishop_b | b->knight_b | b->rook_b;
}

/* 
 * Sets each piece in board to starting represenation. 
 * Also initializes boolean values for check and castling.
*/
int set_standard(board* b) {
    b->pawn_w = 0x000000000000FF00;
    b->pawn_b = 0x00FF000000000000;
    b->queen_w = 0x0000000000000008;
    b->queen_b = 0x0800000000000000;
    b->king_w = 0x0000000000000010;
    b->king_b = 0x1000000000000000;
    b->rook_w = 0x0000000000000081;
    b->rook_b = 0x8100000000000000;
    b->knight_w = 0x0000000000000042;
    b->knight_b = 0x4200000000000000;
    b->bishop_w = 0x0000000000000024;
    b->bishop_b = 0x2400000000000000;

    set_sides(b);

    b->castle_w_l = 0;
    b->castle_w_r = 0;
    b->castle_b_l = 0;
    b->castle_b_r = 0;

    b->turn = 1;

    return 0;
}

/* 
 * Initializes empty board.  
*/
int set_empty(board* b) {
    b->pawn_w = 0;
    b->pawn_b = 0;
    b->queen_w = 0;
    b->queen_b = 0;
    b->king_w = 0;
    b->king_b = 0;
    b->rook_w = 0;
    b->rook_b = 0;
    b->knight_w = 0;
    b->knight_b = 0;
    b->bishop_w = 0;
    b->bishop_b = 0;

    b->white = 0;
    b->black = 0;

    b->castle_w_l = 0;
    b->castle_w_r = 0;
    b->castle_b_l = 0;
    b->castle_b_r = 0;

    b->turn = 1;

    return 0;
}

int board_equals(board* b1, board* b2) {
    int p_w = b1->pawn_w == b2->pawn_w;
    int p_b = b1->pawn_b == b2->pawn_b;
    int q_w = b1->queen_w == b2->queen_w;
    int q_b = b1->queen_b ==  b2->queen_b;
    int k_w = b1->king_w == b2->king_w;
    int k_b = b1->king_b == b2->king_b;
    int r_w = b1->rook_w == b2->rook_w;
    int r_b = b1->rook_b == b2->rook_b;
    int n_w = b1->knight_w == b2->knight_w;
    int n_b = b1->knight_b == b2->knight_b;
    int b_w = b1->bishop_w == b2->bishop_w;
    int b_b = b1->bishop_b == b2->bishop_b;

    int w = b1->white == b2->white;
    int b = b1->black == b2->black;

    int c_w_l = b1->castle_w_l == b2->castle_w_l;
    int c_w_r = b1->castle_w_r == b2->castle_w_r;
    int c_b_l = b1->castle_b_l == b2->castle_b_l;
    int c_b_r = b1->castle_b_r == b2->castle_b_r;

    int t = b1->turn == b2->turn;
    return p_w & p_b & q_w & q_b & q_w & k_w & k_b & r_w & r_b & n_w & n_b & b_b & b_w \
          & w & b & c_w_r & c_w_l & c_b_r & c_b_l & t;
}

char*  board_string(board* b) {
    uint64_t mask = 0x0100000000000000;
    char* b_str = calloc(sizeof(char), 73);
    strcat(b_str, "\n");
    for (int i = 1; i <= 64; i++) {
        uint64_t pawn_w = b->pawn_w & mask;
        uint64_t queen_w = b->queen_w & mask;
        uint64_t king_w = b->king_w & mask;
        uint64_t rook_w = b->rook_w & mask;
        uint64_t knight_w = b->knight_w & mask;
        uint64_t bishop_w = b->bishop_w & mask;
        uint64_t pawn_b = b->pawn_b & mask;
        uint64_t queen_b = b->queen_b & mask;
        uint64_t king_b = b->king_b & mask;
        uint64_t rook_b = b->rook_b & mask;
        uint64_t knight_b = b->knight_b & mask;
        uint64_t bishop_b = b->bishop_b & mask;

        if (pawn_w) {
            strcat(b_str, "P");
        } else if (pawn_b) {
            strcat(b_str, "p");
        } else if (queen_w) {
            strcat(b_str, "Q");
        } else if (queen_b) {
            strcat(b_str, "q");
        } else if (king_w) {
            strcat(b_str, "K");
        } else if (king_b) {
            strcat(b_str, "k");
        } else if (rook_w) {
            strcat(b_str, "R");
        } else if (rook_b) {
            strcat(b_str, "r");
        } else if (knight_w) {
            strcat(b_str, "N");
        } else if (knight_b) {
            strcat(b_str, "n");
        } else if (bishop_w) {
            strcat(b_str, "B");
        } else if (bishop_b) {
            strcat(b_str, "b");
        } else {
            strcat(b_str, "-");
        } 

        if (!(i % 8)) {
            strcat(b_str, "\n");
            mask = mask >> 15;
            continue;
        }

        mask = mask << 1;
    }
    strcat(b_str, "\n");
    return b_str;
}
/* 
 * Generates all legal moves for the king and returns them in one combined board. 
 * Requires both castling booleans for the current side to generate castling move for king. 
*/
uint64_t king_move_board(uint64_t king_board, uint64_t own_side, uint64_t other_side) {
    uint64_t pos_1 = king_board << 8;
    uint64_t pos_2 = (king_board & file_h) << 9;
    uint64_t pos_3 = (king_board & file_h) << 1;
    uint64_t pos_4 = (king_board & file_h ) >> 7;
    uint64_t pos_5 = king_board >> 8;
    uint64_t pos_6 = (king_board & file_a) >> 9;
    uint64_t pos_7 = (king_board & file_a) >> 1;
    uint64_t pos_8 = (king_board & file_a) << 7;


    return (pos_1 | pos_2 | pos_3 | pos_4 | pos_5 | pos_6 | pos_7 | pos_8) & ~own_side;
}

uint64_t knight_move_board(uint64_t knight, uint64_t own_side) {
    uint64_t pos_1 = (knight & file_a) << 15;
    uint64_t pos_2 = (knight & file_h) << 17;
    uint64_t pos_3 = (knight & file_h & file_g) << 10;
    uint64_t pos_4 = (knight & file_h & file_g) >> 6;
    uint64_t pos_5 = (knight & file_h) >> 15;
    uint64_t pos_6 = (knight & file_a) >> 17;
    uint64_t pos_7 = (knight & file_a & file_b) >> 10;
    uint64_t pos_8 = (knight & file_a & file_b) << 6;

    return (pos_1 | pos_2 | pos_3 | pos_4 | pos_5 | pos_6 | pos_7 | pos_8) & ~own_side;
}

uint64_t pawn_w_move_board(uint64_t pawn, uint64_t white, uint64_t black) {
    // Left diagonal attack
    uint64_t pos_1 = ((pawn & file_a) << 7) & black;
    // Forward one
    uint64_t pos_2 = (pawn << 8) & ~black;
    // Right diagonal attack
    uint64_t pos_3 = ((pawn & file_h) << 9) & black;
    // Two step forward 
    uint64_t pos_4 = ((pawn & ~rank_2) << 8 & ~black & ~white) << 8 & ~black;

    return (pos_1 | pos_2 | pos_3 | pos_4) & ~white;
}

uint64_t pawn_b_move_board(uint64_t pawn, uint64_t white, uint64_t black) {
    // Left diagonal attack
    uint64_t pos_1 = ((pawn & file_a) >> 9) & white;
    // Forward one
    uint64_t pos_2 = (pawn >> 8) & ~white;
    // Right diagonal attack
    uint64_t pos_3 = ((pawn & file_h) << 7) & white;
    // Two step forward, & with pos_2 to make sure intermediate space is free.
    uint64_t pos_4 = ((pawn & ~rank_7) >> 8 & ~black & ~white) >> 8 & ~white;

    return (pos_1 | pos_2 | pos_3 | pos_4) & ~black;
}

uint64_t rook_move_board(uint64_t rook, uint64_t own_side, uint64_t other_side) {
    // North ray 
    uint64_t north_ray = (rook << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;

    // South ray 
    uint64_t south_ray = (rook >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;

    // East ray 
    uint64_t east_ray = ((rook & file_h) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;

    // West ray 
    uint64_t west_ray = ((rook & file_a) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;

    return north_ray | south_ray | east_ray | west_ray;
}

uint64_t rook_attacks_to_piece(uint64_t rook, uint64_t own_side, uint64_t other_side, uint64_t piece) {
    // North ray 
    uint64_t north_ray = (rook << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;
    north_ray |= ((north_ray & ~other_side) << 8) & ~own_side;

    if (!(north_ray & piece)) north_ray = 0;

    // South ray 
    uint64_t south_ray = (rook >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;
    south_ray |= ((south_ray & ~other_side) >> 8) & ~own_side;

    if (!(south_ray & piece)) south_ray = 0;

    // East ray 
    uint64_t east_ray = ((rook & file_h) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;
    east_ray |= ((east_ray & file_h & ~other_side) << 1) & ~own_side;

    if (!(east_ray & piece)) east_ray = 0;

    // West ray 
    uint64_t west_ray = ((rook & file_a) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;
    west_ray |= ((west_ray & file_a & ~other_side) >> 1) & ~own_side;

    if (!(west_ray & piece)) west_ray = 0;

    return north_ray | south_ray | east_ray | west_ray;
}

uint64_t bishop_attacks_to_piece(uint64_t bishop, uint64_t own_side, uint64_t other_side, uint64_t piece) {
    uint64_t ne_ray = ((bishop & file_h) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;

    if (!(ne_ray & piece)) ne_ray = 0;

    uint64_t se_ray = ((bishop & file_h) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;

    if (!(se_ray & piece)) se_ray = 0;

    uint64_t sw_ray = ((bishop & file_a) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;

    if (!(sw_ray & piece)) sw_ray = 0;

    uint64_t nw_ray = ((bishop & file_a) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;

    if (!(nw_ray & piece)) nw_ray = 0;

    return ne_ray | se_ray | sw_ray | nw_ray;
}

uint64_t bishop_move_board(uint64_t bishop, uint64_t own_side, uint64_t other_side) {
    uint64_t ne_ray = ((bishop & file_h) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;
    ne_ray |= ((ne_ray & file_h & ~other_side) << 9) & ~own_side;

    uint64_t se_ray = ((bishop & file_h) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;
    se_ray |= ((se_ray & file_h & ~other_side) >> 7) & ~own_side;

    uint64_t sw_ray = ((bishop & file_a) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;
    sw_ray |= ((sw_ray & file_a & ~other_side) >> 9) & ~own_side;

    uint64_t nw_ray = ((bishop & file_a) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;
    nw_ray |= ((nw_ray & file_a & ~other_side) << 7) & ~own_side;

    return ne_ray | se_ray | sw_ray | nw_ray;
}

uint64_t queen_move_board(uint64_t queen, uint64_t own_side, uint64_t other_side) {
    return bishop_move_board(queen, own_side, other_side) | rook_move_board(queen, own_side, other_side);
}

uint64_t queen_attacks_to_piece(uint64_t queen, uint64_t own_side, uint64_t other_side, uint64_t piece) {
    return bishop_attacks_to_piece(queen, own_side, other_side, piece) | rook_attacks_to_piece(queen, own_side, other_side, piece);
}

uint64_t b_move_board(board *b) {
    uint64_t pawn_moves = pawn_b_move_board(b->pawn_b, b->white, b->black);
    uint64_t queen_moves = queen_move_board(b->queen_b, b->black, b->white);
    uint64_t king_moves = king_move_board(b->king_b, b->black, b->white);
    uint64_t rook_moves = rook_move_board(b->rook_b, b->black, b->white);
    uint64_t knight_moves = knight_move_board(b->knight_b, b->black);
    uint64_t bishop_moves = bishop_move_board(b->bishop_b, b->black, b->white);

    return pawn_moves | queen_moves | king_moves | rook_moves | knight_moves | bishop_moves;
}

uint64_t w_move_board(board *b) {
    uint64_t pawn_moves = pawn_w_move_board(b->pawn_w, b->white, b->black);
    uint64_t queen_moves = queen_move_board(b->queen_w, b->white, b->black);
    uint64_t king_moves = king_move_board(b->king_w, b->white, b->black);
    uint64_t rook_moves = rook_move_board(b->rook_w, b->white, b->black);
    uint64_t knight_moves = knight_move_board(b->knight_w, b->white);
    uint64_t bishop_moves = bishop_move_board(b->bishop_w, b->white, b->black);

    return pawn_moves | queen_moves | king_moves | rook_moves | knight_moves | bishop_moves;
}

/* 
 * Returns all possible attacks by black pieces.
*/
uint64_t attack_board_b(board* b) {
    return b_move_board(b) & b->white;
}

/* 
 * Returns all possible attacks by white pieces.
*/
uint64_t attack_board_w(board* b) {
    return w_move_board(b) & b->black;
}

/*
 * Returns true if current piece is being attacked. 
*/
int is_attacked_w(uint64_t piece_w, board* b) {
    return (attack_board_b(b) & piece_w) ? 1: 0; 
}

/*
 * Returns true if current piece is being attacked. 
*/
int is_attacked_b(uint64_t piece_b, board* b) {
    return (attack_board_w(b) & piece_b) ? 1: 0; 
}

int in_check_w(board* b) {
    return is_attacked_w(b->king_w, b);    
}

int in_check_b(board* b) {
    return is_attacked_b(b->king_b, b);    
}

int in_check(board* b, int side) {
    return (side) ? in_check_w(b): in_check_b(b);
}


board* board_copy(board *new_board, board* b) {
    new_board->pawn_w = b->pawn_w;
    new_board->queen_w = b->queen_w;
    new_board->king_w = b->king_w;
    new_board->rook_w = b->rook_w;
    new_board->knight_w = b->knight_w;
    new_board->bishop_w = b->bishop_w;
    new_board->pawn_b = b->pawn_b;
    new_board->queen_b = b->queen_b;
    new_board->king_b = b->king_b;
    new_board->rook_b = b->rook_b;
    new_board->knight_b = b->knight_b;
    new_board->bishop_b = b->bishop_b;
    set_sides(new_board);
    new_board->castle_w_l = b->castle_w_l;
    new_board->castle_w_r = b->castle_w_r;
    new_board->castle_b_l = b->castle_b_l;
    new_board->castle_b_r = b->castle_b_r;

    new_board->turn = b->turn;
    return new_board;
}

void get_intersecting_w(board *b, uint64_t spaces) {
    b->pawn_w &= spaces;
    b->queen_w &= spaces;
    b->king_w &= spaces;
    b->rook_w &= spaces;
    b->knight_w &= spaces;
    b->bishop_w &= spaces;
    b->white &= spaces;
}

void get_intersecting_b(board *b, uint64_t spaces) {
    b->pawn_b &= spaces;
    b->queen_b &= spaces;
    b->king_b &= spaces;
    b->rook_b &= spaces;
    b->knight_b &= spaces;
    b->bishop_b &= spaces;
    b->black &= spaces;
}

uint64_t w_legal_moves(board* b) {
    // Determine if in check and return only moves that escape check. 
    uint64_t black_moves = b_move_board(b);

    // In check
    if (black_moves & b->king_w) {
        uint64_t moves = 0;
        // Remove king from psuedo legal. Will generate separately at end.
        uint64_t king = b->king_w;
        b->king_w = 0;
        uint64_t psuedo_legal_moves = w_move_board(b);
        b->king_w = king;
        uint64_t rook_attacks_to_king = rook_attacks_to_piece(b->rook_b | b->queen_b, b->black, b->white, b->king_w);
        uint64_t bishop_attacks_to_king = bishop_attacks_to_piece(b->bishop_b | b->queen_b, b->black, b->white, b->king_w);
        uint64_t attack_paths_to_king = rook_attacks_to_king | bishop_attacks_to_king; 
        // Find interpositions
        if (attack_paths_to_king) {
            moves |= (psuedo_legal_moves & attack_paths_to_king);
        }
        // Compute capture moves.
        moves |= psuedo_legal_moves & pawn_w_move_board(b->king_w, b->white, b->black) & b->pawn_b;
        moves |= psuedo_legal_moves & bishop_move_board(b->king_w, b->white, b->black) & (b->bishop_b | b->queen_b);
        moves |= psuedo_legal_moves & knight_move_board(b->king_w, b->white) & b->knight_b;
        moves |= psuedo_legal_moves & rook_move_board(b->king_w, b->white, b->black) & (b->rook_b | b->queen_b);
        // Compute king moves. Add unchecked king moves as "white" pieces to remove moves still being attacked.
        uint64_t king_moves = king_move_board(b->king_w, b->white, b->black);
        uint64_t white_side = b->white;
        b->white |= king_moves;
        black_moves = b_move_board(b);
        moves |= (king_moves & ~black_moves);
        b->white = white_side;
        return moves;
    }
    // First generate moves of pinned pieces, ie pieces limited by placing king in check
    uint64_t attacks_to_king = queen_move_board(b->king_w, b->black, b->white);
    // Board with potentially pinned pieces ie pieces in possible rook attack path to king.
    board *b_pinned = board_alloc();
    board_copy(b_pinned, b);
    get_intersecting_w(b_pinned, attacks_to_king);
    // Compute new board without pinned pieces by finding intersection of board and the flip of all pinned pieces. 
    board* b_unpinned = board_alloc();
    board_copy(b_unpinned, b);
    get_intersecting_w(b_unpinned, ~(b_pinned->white));
    // Now compute queen attacks from king square against enemy side to find pinners.
    attacks_to_king = queen_move_board(b->king_w, b_unpinned->white, b_unpinned->black);
    // Find all enemy, black, pinning pieces by finding black intersection with queen attacks from king
    board* b_pinners = board_alloc();
    board_copy(b_pinners, b);
    get_intersecting_b(b_pinners, attacks_to_king);
    uint64_t pinners = b_pinners->rook_b | b_pinners->bishop_b | b_pinners->queen_b;
    // Rook attacks are actual rooks and horizontal / vertical attacks of queen
    uint64_t rook_pinner_attacks = rook_move_board(b_pinners->rook_b | b_pinners->queen_b, b_pinners->black, b_pinners->white);
    // Bishop attacks are actual bishop attacks and diagonal attacks of queen
    uint64_t bishop_pinner_attacks = bishop_move_board(b_pinners->bishop_b | b_pinners->queen_b, b_pinners->black, b_pinners->white);
    // Intersect pinner_attacks with pinned pieces to find actually pinned pieces.
    uint64_t pinner_attacks = rook_pinner_attacks | bishop_pinner_attacks;
    get_intersecting_w(b_pinned, pinner_attacks);
    uint64_t pinned_piece_moves = w_move_board(b_pinned);
    // Intersect with pinner_attacks and pinners to find moves which capture pinner or move along attacking path.
    pinned_piece_moves &= (pinner_attacks | pinners); 
    // Find board of actually unpinned pieces by intersecting with flip of all pinners 
    board_copy(b_unpinned, b);
    // Remove pinned pieces themselves but leave them in white to preserve blocks / obstructions
    uint64_t unpinned_white = b_unpinned->white;
    get_intersecting_w(b_unpinned, ~(b_pinned->white));
    b_unpinned->white = unpinned_white;
    // Unpinned piece moves masked with all moves to remove extraneous moves generated by removing pinned pieces ie. extra castle paths.
    uint64_t king = b_unpinned->king_w;
    b_unpinned->king_w = 0;
    uint64_t unpinned_piece_moves = w_move_board(b_unpinned);
    b_unpinned->king_w = king;
    // Remove all king moves that place it in check.
    uint64_t king_moves = king_move_board(b->king_w, b->white, b->black);
    uint64_t white_side = b->white;
    b->white |= king_moves;
    // Compute black captures with king moves as "pieces" to account for attack squares.
    black_moves = b_move_board(b);
    // Mask with flip of black moves to remove any king moves into black move squares.
    unpinned_piece_moves |= (king_moves & ~black_moves);
    b->white = white_side;
    board_delete(b_pinned);
    board_delete(b_unpinned);
    board_delete(b_pinners);

    return pinned_piece_moves | unpinned_piece_moves;
}

uint64_t b_legal_moves(board* b) {
    uint64_t white_moves = w_move_board(b);

    // In check
    if (white_moves & b->king_b) {
        uint64_t moves = 0;
        // Remove king from psuedo legal. Will generate separately at end.
        uint64_t king = b->king_b;
        b->king_b = 0;
        uint64_t psuedo_legal_moves = b_move_board(b);
        b->king_b = king;
        uint64_t rook_attacks_to_king = rook_attacks_to_piece(b->rook_w | b->queen_w, b->white, b->black, b->king_b);
        uint64_t bishop_attacks_to_king = bishop_attacks_to_piece(b->bishop_w | b->queen_w, b->white, b->black, b->king_b);
        uint64_t attack_paths_to_king = rook_attacks_to_king | bishop_attacks_to_king; 
        // Find interpositions
        if (attack_paths_to_king) {
            moves |= (psuedo_legal_moves & attack_paths_to_king);
        }
        // Compute capture moves.
        moves |= psuedo_legal_moves & pawn_b_move_board(b->king_b, b->white, b->black) & b->pawn_w;
        moves |= psuedo_legal_moves & bishop_move_board(b->king_b, b->black, b->white) & (b->bishop_w | b->queen_w);
        moves |= psuedo_legal_moves & knight_move_board(b->king_b, b->black) & b->knight_w;
        moves |= psuedo_legal_moves & rook_move_board(b->king_b, b->black, b->white) & (b->rook_w | b->queen_w);
        uint64_t king_moves = king_move_board(b->king_b, b->black, b->white);
        // Compute king moves. Add unchecked king moves as "black" pieces to remove moves still being attacked.
        uint64_t black_side = b->black;
        b->black |= king_moves;
        white_moves = w_move_board(b);
        moves |= (king_moves & ~white_moves);
        b->black = black_side;
        return moves;

    }
    // First generate moves of pinned pieces, ie pieces limited by placing king in check
    uint64_t attacks_to_king = queen_move_board(b->king_b, b->white, b->black);
    // Board with potentially pinned pieces ie pieces in possible rook attack path to king.
    board *b_pinned = board_alloc();
    board_copy(b_pinned, b);
    get_intersecting_b(b_pinned, attacks_to_king);
    // Compute new board without pinned pieces by finding intersection of board and the flip of all pinned pieces. 
    board* b_unpinned = board_alloc();
    board_copy(b_unpinned, b);
    get_intersecting_b(b_unpinned, ~(b_pinned->black));
    // Re-compute rook attacks with new board.
    attacks_to_king = queen_move_board(b->king_b, b_unpinned->black, b_unpinned->white);
    
    // Find all enemy, black, pinning pieces by finding black intersection with queen attacks from king
    board* b_pinners = board_alloc();
    board_copy(b_pinners, b);
    get_intersecting_w(b_pinners, attacks_to_king);
    uint64_t pinners = b_pinners->rook_w | b_pinners->bishop_w | b_pinners->queen_w;
    // Rook attacks are actual rooks and horizontal / vertical attacks of queen
    uint64_t rook_pinner_attacks = rook_move_board(b_pinners->rook_w | b_pinners->queen_w, b_pinners->white, b_pinners->black);
    // Bishop attacks are actual bishop attacks and diagonal attacks of queen
    uint64_t bishop_pinner_attacks = bishop_move_board(b_pinners->bishop_w | b_pinners->queen_w, b_pinners->white, b_pinners->black);
    // Intersect pinner_attacks with pinned pieces to find actually pinned pieces.
    uint64_t pinner_attacks = rook_pinner_attacks | bishop_pinner_attacks;
    get_intersecting_b(b_pinned, pinner_attacks);
    uint64_t pinned_piece_moves = b_move_board(b_pinned);
    // Intersect with pinner_attacks and pinners to find moves which capture pinner or move along attacking path.
    pinned_piece_moves &= (pinner_attacks | pinners); 
    // Find board of actually unpinned pieces by intersecting with flip of all pinners 
    board_copy(b_unpinned, b);
    uint64_t unpinned_black = b_unpinned->black;
    uint64_t king = b_unpinned->king_b;
    get_intersecting_b(b_unpinned, ~(b_pinned->black));
    b_unpinned->black = unpinned_black;
    b_unpinned->king_b = 0;
    uint64_t unpinned_piece_moves = b_move_board(b_unpinned);
    b_unpinned->king_b = king;
    // Remove all king moves that place it in check.
    uint64_t king_moves = king_move_board(b->king_b, b->black, b->white);
    uint64_t black_side = b->black;
    b->black |= king_moves;
    // Compute white captures with king moves as "pieces" to account for attack squares.
    white_moves = w_move_board(b);
    // Mask with flip of white moves to remove any king moves into white move squares.
    unpinned_piece_moves |= (king_moves & ~white_moves);
    b->black = black_side;
    board_delete(b_pinned);
    board_delete(b_unpinned);
    board_delete(b_pinners);

    return pinned_piece_moves | unpinned_piece_moves;
}

uint64_t get_legal_moves(board *b) {
    return (b->turn) ? w_legal_moves(b): b_legal_moves(b);
}

uint64_t move_board_w(board* b, uint64_t piece) {
    uint64_t pawn = b->pawn_w & piece;
    uint64_t queen = b->queen_w & piece;
    uint64_t king = b->king_w & piece;
    uint64_t rook = b->rook_w & piece;
    uint64_t knight = b->knight_w & piece;
    uint64_t bishop = b->bishop_w & piece;
    
    if (pawn) {
        return pawn_w_move_board(piece, b->white, b->black);
    } else if (queen) {
        return queen_move_board(piece, b->white, b->black);
    } else if (king) {
        // Remove king moves into attacked squares
        uint64_t king_moves = king_move_board(piece, b->white, b->black);
        uint64_t white = b->white;
        b->white |= king_moves;
        king_moves &= ~(b_move_board(b));
        b->white = white;
        return king_moves;
    } else if (rook) {
        return rook_move_board(piece, b->white, b->black);
    } else if (knight) {
        return knight_move_board(piece, b->white);
    } else if (bishop) {
        return bishop_move_board(piece, b->white, b->black);
    }
    return 0;
}

uint64_t move_board_b(board* b, uint64_t piece) {
    uint64_t pawn = b->pawn_b & piece;
    uint64_t queen = b->queen_b & piece;
    uint64_t king = b->king_b & piece;
    uint64_t rook = b->rook_b & piece;
    uint64_t knight = b->knight_b & piece;
    uint64_t bishop = b->bishop_b & piece;
    
    if (pawn) {
        return pawn_b_move_board(piece, b->white, b->black);
    } else if (queen) {
        return queen_move_board(piece, b->black, b->white);
    } else if (king) {
        // Remove king moves into attacked squares
        uint64_t king_moves = king_move_board(piece, b->black, b->white);
        uint64_t black = b->black;
        b->black |= king_moves;
        king_moves &= ~(b_move_board(b));
        b->black = black;
        return king_moves;
    } else if (rook) {
        return rook_move_board(piece, b->black, b->white);
    } else if (knight) {
        return knight_move_board(piece, b->black);
    } else if (bishop) {
        return bishop_move_board(piece, b->black, b->white);
    }
    return 0;
}

uint64_t move_board(board* b, uint64_t piece) {
    return (b->turn) ? move_board_w(b, piece): move_board_b(b, piece);
}

/* 
 * Updates move location in given board. 
 * from: single bit location being moved, to is destination bit location.
 * board: board to be updated 
*/
void make_move_b(uint64_t from, uint64_t to, board* b) {
    uint64_t pawn = b->pawn_b & from;
    uint64_t queen = b->queen_b & from;
    uint64_t king = b->king_b & from;
    uint64_t rook = b->rook_b & from;
    uint64_t knight = b->knight_b & from;
    uint64_t bishop = b->bishop_b & from;
    
    if (pawn) {
        b->pawn_b &= ~from;
        b->pawn_b |= to;
    } else if (queen) {
        b->queen_b &= ~from;
        b->queen_b |= to;
    } else if (king) {
        b->king_b &= ~from;
        b->king_b |= to;
        b->castle_b_l = 0;
        b->castle_b_r = 0;
    } else if (rook) {
        b->rook_b &= ~from;
        b->rook_b |= to;
        b->castle_b_l = 0;
        b->castle_b_r = 0;
    } else if (knight) {
        b->knight_b &= ~from;
        b->knight_b |= to;
    } else if (bishop) {
        b->bishop_b &= ~from;
        b->bishop_b |= to;
    }

    b->black &= ~from;
    b->black |= to;

    if (b->white & to) {
        make_move_w(to, 0, b);
    }

}

/* 
 * Updates move location in given board. 
 * from: single bit location being moved, to is destination bit location.
 * board: board to be updated 
*/
void make_move_w(uint64_t from, uint64_t to, board* b) {
    uint64_t pawn = b->pawn_w & from;
    uint64_t queen = b->queen_w & from;
    uint64_t king = b->king_w & from;
    uint64_t rook = b->rook_w & from;
    uint64_t knight = b->knight_w & from;
    uint64_t bishop = b->bishop_w & from;
    
    if (pawn) {
        b->pawn_w &= ~from;
        b->pawn_w |= to;
    } else if (queen) {
        b->queen_w &= ~from;
        b->queen_w |= to;
    } else if (king) {
        b->king_w &= ~from;
        b->king_w |= to;
        b->castle_w_l = 0;
        b->castle_w_r = 0;
    } else if (rook) {
        b->rook_w &= ~from;
        b->rook_w |= to;
        b->castle_w_l = 0;
        b->castle_w_r = 0;
    } else if (knight) {
        b->knight_w &= ~from;
        b->knight_w |= to;
    } else if (bishop) {
        b->bishop_w &= ~from;
        b->bishop_w |= to;
    }
    
    b->white &= ~from;
    b->white |= to;

    if (b->black & to) {
        make_move_b(to, 0, b);
    }

}

/*
 * Undo move by calling moving in the reverse direction. 
 * Requires castle booleans from board prior to move for reset.
*/
void undo_move_b(uint64_t from, uint64_t to, board* b) {
    make_move_b(to, from, b);
}

/*
 * Undo move by calling moving in the reverse direction. 
*/
void undo_move_w(uint64_t from, uint64_t to, board* b) {
    make_move_w(to, from, b);
}

void undo_move(uint64_t from, uint64_t to, board* b, board* old_board) {
    board_copy(b, old_board);
}

void make_move(uint64_t from, uint64_t to, board* b) {
    (b->turn) ? make_move_w(from, to, b): make_move_b(from, to, b);
    b->turn = !b->turn;
}

int make_castle_b_r(board *b) {
    // King and rook starting positions for w left castle. 
    // No need to check if king or rook in these positions as booleans verify they haven't been moved.
    uint64_t king = 0x1000000000000000;
    uint64_t rook = 0x8000000000000000;
    uint64_t king_right = 0x2000000000000000;
    // Check if rook can move to kings left and if no enemy piece ie no obstruction for castling.
    uint64_t can_castle = rook_move_board(rook, b->white, b->black) & king_right & ~b->black;

    // Move rook and king to correct positions if castle availiable. 
    if (can_castle) {
        make_move_w(rook, king_right, b);
        make_move_w(king, 0x4000000000000000, b);
        b->turn = 1;
        return 1;
    }
    return 0;
}

int make_castle_b_l(board *b) {
    // King and rook starting positions for w left castle. 
    // No need to check if king or rook in these positions as booleans verify they haven't been moved.
    uint64_t king = 0x1000000000000000;
    uint64_t rook = 0x0100000000000000;
    uint64_t king_left = 0x0800000000000000;
    // Check if rook can move to kings left and if no enemy piece ie no obstruction for castling.
    uint64_t can_castle = rook_move_board(rook, b->white, b->black) & king_left & ~b->black;

    // Move rook and king to correct positions if castle availiable. 
    if (can_castle) {
        make_move_w(rook, king_left, b);
        make_move_w(king, 0x0400000000000000, b);
        b->turn = 1;
        return 1;
    }
    return 0;
}

int make_castle_w_r(board *b) {
    // King and rook starting positions for w left castle. 
    // No need to check if king or rook in these positions as booleans verify they haven't been moved.
    uint64_t king = 0x10;
    uint64_t rook = 0x80;
    uint64_t king_right = 0x20;
    // Check if rook can move to kings left and if no enemy piece ie no obstruction for castling.
    uint64_t can_castle = rook_move_board(rook, b->white, b->black) & king_right & ~b->black;

    // Move rook and king to correct positions if castle availiable. 
    if (can_castle) {
        make_move_w(rook, king_right, b);
        make_move_w(king, 0x40, b);
        b->turn = 0;
        return 1;
    }
    return 0;
}

int make_castle_w_l(board *b) {
    // King and rook starting positions for w left castle. 
    // No need to check if king or rook in these positions as booleans verify they haven't been moved.
    uint64_t king = 0x10;
    uint64_t rook = 0x1;
    uint64_t king_left = 0x8;
    // Check if rook can move to kings left and if no enemy piece ie no obstruction for castling.
    uint64_t can_castle = rook_move_board(rook, b->white, b->black) & king_left & ~b->black;

    // Move rook and king to correct positions if castle availiable. 
    if (can_castle) {
        make_move_w(rook, king_left, b);
        make_move_w(king, 0x4, b);
        b->turn = 0;
        return 1;
    }
    return 0;
}

void undo_castle_b_r(board *b) {
    uint64_t king = 0x1000000000000000;
    uint64_t rook = 0x8000000000000000;
    uint64_t king_right = 0x2000000000000000;
    uint64_t king_dest = 0x4000000000000000;

    make_move_w(king_right, rook, b);
    make_move_w(king_dest, king, b);
    b->turn = 0;
}

void undo_castle_b_l(board *b) {
    uint64_t king = 0x1000000000000000;
    uint64_t rook = 0x0100000000000000;
    uint64_t king_left = 0x0800000000000000;
    uint64_t king_dest = 0x0400000000000000;

    make_move_w(king_left, rook, b);
    make_move_w(king_dest, king, b);
    b->turn = 0;
}

void undo_castle_w_r(board *b) {
    uint64_t king = 0x10;
    uint64_t rook = 0x80;
    uint64_t king_right = 0x20;
    uint64_t king_dest = 0x40;

    make_move_w(king_right, rook, b);
    make_move_w(king_dest, king, b);
    b->turn = 1;
}

void undo_castle_w_l(board *b) {
    uint64_t king = 0x10;
    uint64_t rook = 0x1;
    uint64_t king_left = 0x8;
    uint64_t king_dest = 0x4;

    make_move_w(king_left, rook, b);
    make_move_w(king_dest, king, b);
    b->turn = 1;
}

int make_castle_l(board *b) {
    return (b->turn) ? make_castle_w_l(b) : make_castle_b_l(b);
}

int make_castle_r(board *b) {
    return (b->turn) ? make_castle_w_r(b): make_castle_b_r(b);
}

void undo_castle_l(board *b, board* old_board) {
    board_copy(b, old_board);
}

void undo_castle_r(board *b, board* old_board) {
    board_copy(b, old_board);
}

int can_castle_l(board *b) {
    return ((b->turn) ? b->castle_w_l: b->castle_b_l);
}

int can_castle_r(board *b) {
    return ((b->turn) ? b->castle_w_r: b->castle_b_r);
}

uint64_t perft(board* b, int depth) {
    //printf("Board in perft %s\n", board_string(b));
    if (!depth) return 1;
    uint64_t nodes = 0;
    uint64_t moves = get_legal_moves(b);
    uint64_t move_pieces = (queen_move_board(moves, get_opp_side(b), get_curr_side(b)) \
                                | knight_move_board(moves, get_opp_side(b))) & get_curr_side(b);
   
    board* b_copy = board_alloc();
    board_copy(b_copy, b);

    uint64_t from_mask = 1;
    while (from_mask) {
        uint64_t from = move_pieces & from_mask;
        if (from) { 
            uint64_t piece_moves = move_board(b, from) & moves;
            if (piece_moves) {
                uint64_t to_mask = 1;
                while (to_mask) {
                    uint64_t to = piece_moves & to_mask;
                    if (to) {
                        make_move(from, to, b);
                        // If the current side is not in check after move, use node.
                        if (!in_check(b, !b->turn)) {
                            nodes += perft(b, depth - 1);
                        }
                        board_copy(b, b_copy);
                    }
                    to_mask = to_mask << 1;
                }
            }
        }
        from_mask = from_mask << 1;
    }

    // Compute castling boards 
    if (can_castle_l(b)) {
        if (make_castle_l(b)) {
            nodes += perft(b, depth - 1);
            board_copy(b, b_copy);
        }
    } 
    if (can_castle_r(b)) {
        if (make_castle_r(b)) {
            nodes += perft(b, depth - 1);
            board_copy(b, b_copy);
        }
    }
    board_delete(b_copy);

    return nodes;
}

void play_game() {
    board* b = board_alloc();
    set_standard(b);
    

}

/* 
 * Converts an input fen representation into internal bitboard representation. 
 * Used to parse various game boards for move evaluation. 
*/
void parse_fen(board* b, char *fen) {
    set_empty(b);
    char* fields[6]; 
    char* token = strtok(fen, " ");
    fields[0] = token;
    for (int i = 1; i < 6; i++) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("fen must contain six fields separated by spaces");
            return;
        }
        fields[i] = token;
    }
    
    uint64_t loc_mask = 0x0100000000000000;
    for (int i = 0; i < strlen(fields[0]); i++) {
        char c = fields[0][i];
        int n = 0;
        if (c == '1') n = 1;
        if (c == '2') n = 2;
        if (c == '3') n = 3;            
        if (c == '4') n = 4;
        if (c == '5') n = 5;
        if (c == '6') n = 6;
        if (c == '7') n = 7;
        if (c == '8') n = 8;

        if (!n) {
            switch(c) {
                case 'p':
                    b->pawn_b |= loc_mask;
                    break;
                case 'q':
                    b->queen_b |= loc_mask;
                    break;
                case 'k':
                    b->king_b |= loc_mask;
                    break;
                case 'r':
                    b->rook_b |= loc_mask;
                    break;
                case 'n':
                    b->knight_b |= loc_mask;
                    break;
                case 'b':
                    b->bishop_b |= loc_mask;
                    break;
                case 'P':
                    b->pawn_w |= loc_mask;
                    break;
                case 'Q':
                    b->queen_w |= loc_mask;
                    break;
                case 'K':
                    b->king_w |= loc_mask;
                    break;
                case 'R':
                    b->rook_w |= loc_mask;
                    break;
                case 'N':
                    b->knight_w |= loc_mask;
                    break;
                case 'B':
                    b->bishop_w |= loc_mask;
                    break;
                case '/':
                    if (!loc_mask) {
                        loc_mask = 0x0001000000000000;
                    } else {
                        loc_mask = loc_mask >> 16;
                    }
                default:
                    continue;
            }
            loc_mask = loc_mask << 1;
        } else {
            loc_mask = loc_mask << n;
        }
    }
    if (fields[1][0] == 'w') {
        b->turn = 1;
    } else {
       b->turn = 0; 
    }
    set_sides(b);
}

