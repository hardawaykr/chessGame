#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "dbg.h"

// Board representation with rank / file 0d out to ease computation of overflows
uint64_t rank_8 = 0x00FFFFFFFFFFFFFF;
uint64_t rank_7 = 0xFF00FFFFFFFFFFFF;
uint64_t rank_1 = 0xFFFFFFFFFFFFFF00;
uint64_t rank_2 = 0xFFFFFFFFFFFF00FF;
uint64_t file_a = 0xFEFEFEFEFEFEFEFE;
uint64_t file_b = 0xFDFDFDFDFDFDFDFD;
uint64_t file_h = 0x7F7F7F7F7F7F7F7F;
uint64_t file_g = 0xBFBFBFBFBFBFBFBF;

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

    int check_w;
    int check_b;
} board;

board* board_alloc() {
    board *b = malloc(sizeof(board));
    if (!b) {
        printf("Board failed to allocate\n");
        return NULL;
    }
    return b;
}

int fill_standard(board* b) {
    b->pawn_w = 0x000000000000FF00;
    b->pawn_b = 0x00FF000000000000;
    b->queen_w = 0x0000000000000008;
    b->queen_b = 0x8000000000000000;
    b->king_w = 0x0000000000000010;
    b->king_b = 0x1000000000000000;
    b->rook_w = 0x0000000000000081;
    b->rook_b = 0x8100000000000000;
    b->knight_w = 0x0000000000000042;
    b->knight_b = 0x4200000000000000;
    b->bishop_w = 0x0000000000000024;
    b->bishop_b = 0x2400000000000000;

    b->white = b->pawn_w | b->queen_w | b->king_w | b->bishop_w | b->knight_w | b->rook_w;
    b->black = b->pawn_b | b->queen_b | b->king_b | b->bishop_b | b->knight_b | b->rook_b;

    b->castle_w_l = 0;
    b->castle_w_r = 0;
    b->castle_b_l = 0;
    b->castle_b_r = 0;

    b->check_w = 0;
    b->check_b = 0;
    return 0;
}

uint64_t king_move_board(uint64_t king_board, uint64_t own_side, uint64_t other_side, int can_castle_left, int can_castle_right) {
    uint64_t pos_1 = king_board << 8;
    uint64_t pos_2 = (king_board & file_h) << 9;
    uint64_t pos_3 = (king_board & file_h) << 1;
    uint64_t pos_4 = (king_board & file_h ) >> 7;
    uint64_t pos_5 = king_board >> 8;
    uint64_t pos_6 = (king_board & file_a) >> 9;
    uint64_t pos_7 = (king_board & file_a) >> 1;
    uint64_t pos_8 = (king_board & file_a) << 7;

    uint64_t pos_9 = 0;
    uint64_t pos_10 = 0;
    if (can_castle_right) {
        pos_9 = ((king_board & file_a) >> 1) & ~other_side & ~own_side;
        pos_9 = ((pos_9 & file_a) >> 1) & ~other_side;
    }
    if (can_castle_left) {
        pos_10 = ((king_board & file_h) << 1) & ~other_side & ~ own_side;
        pos_10 = ((pos_10 & file_h) << 1) & ~other_side;

    } 

    return (pos_1 | pos_2 | pos_3 | pos_4 | pos_5 | pos_6 | pos_7 | pos_8 | pos_9 | pos_10) & ~own_side;
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
    uint64_t pos_4 = (pawn & ~rank_2) << 16 & ~black;

    return (pos_1 | pos_2 | pos_3 | pos_4) & ~white;
}

uint64_t pawn_b_move_board(uint64_t pawn, uint64_t white, uint64_t black) {
    // Left diagonal attack
    uint64_t pos_1 = ((pawn & file_a) >> 9) & white;
    // Forward one
    uint64_t pos_2 = (pawn >> 8) & ~white;
    // Right diagonal attack
    uint64_t pos_3 = ((pawn & file_h) << 7) & white;
    // Two step forward
    uint64_t pos_4 = (pawn & ~rank_7) >> 16 & ~white;

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
    uint64_t east_ray = (rook << 1) & ~own_side;
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

void play_game() {
    board* b = board_alloc();
}

board* parse_fen(char *fen) {
    board *b = board_alloc();
    char* fields[6]; 
    char* token = strtok(fen, " ");
    fields[0] = token;
    for (int i = 0; i < 5; i++) {
        token = strtok(NULL, " ");
        if (token == NULL) {
            printf("fen must contain six fields separated by spaces");
            return NULL;
        }
        printf("The %d token is %s\n", i + 1, token);
        fields[i] = token;
    }
    return 0;
}

board* parse_piece_fen(char *row) {
    return NULL; 
}
