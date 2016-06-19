#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "dbg.h"

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
} board;


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

    return 0;
}

uint64_t king_move_board(uint64_t king_board, uint64_t own_side) {
    uint64_t pos_1 = (king_board & rank_8) << 8;
    uint64_t pos_2 = (king_board & rank_8 & file_h) << 9;
    uint64_t pos_3 = (king_board & file_h) << 1;
    uint64_t pos_4 = (king_board & file_h & rank_1) >> 7;
    uint64_t pos_5 = (king_board & rank_1) >> 8;
    uint64_t pos_6 = (king_board & rank_1 & file_a) >> 9;
    uint64_t pos_7 = (king_board & file_a) >> 1;
    uint64_t pos_8 = (king_board & file_a & rank_1) << 7;

    return (pos_1 | pos_2 | pos_3 | pos_4 | pos_5 | pos_6 | pos_7 | pos_8) & ~own_side;
}

uint64_t knight_move_board(uint64_t knight, uint64_t own_side) {
    uint64_t pos_1 = (knight & file_a & rank_8 & rank_7) << 15;
    uint64_t pos_2 = (knight & file_h & rank_8 & rank_7) << 17;
    uint64_t pos_3 = (knight & file_h & rank_8 & file_g) << 10;
    uint64_t pos_4 = (knight & file_h & rank_1 & file_g) >> 6;
    uint64_t pos_5 = (knight & file_h & rank_1 & rank_2) >> 15;
    uint64_t pos_6 = (knight & file_a & rank_1 & rank_2) >> 17;
    uint64_t pos_7 = (knight & file_a & rank_8 & file_b) >> 10;
    uint64_t pos_8 = (knight & file_a & rank_8 & file_b) << 6;

    return (pos_1 | pos_2 | pos_3 | pos_4 | pos_5 | pos_6 | pos_7 | pos_8) & ~own_side;
}

uint64_t pawn_w_move_board(uint64_t pawn, uint64_t white, uint64_t black) {
    // Left diagonal attack
    uint64_t pos_1 = ((pawn & rank_8 & file_a) << 7) & black;
    // Forward one
    uint64_t pos_2 = ((pawn & rank_8) << 8) & ~black;
    // Right diagonal attack
    uint64_t pos_3 = ((pawn & rank_8 & file_h) << 9) & black;
    // Two step forward
    uint64_t pos_4 = (pawn & ~rank_2) << 16 & ~black;

    return (pos_1 | pos_2 | pos_3 | pos_4) & ~white;
}

uint64_t pawn_b_move_board(uint64_t pawn, uint64_t white, uint64_t black) {
    // Left diagonal attack
    uint64_t pos_1 = ((pawn & rank_1 & file_a) >> 9) & white;
    // Forward one
    uint64_t pos_2 = ((pawn & rank_1) >> 8) & ~white;
    // Right diagonal attack
    uint64_t pos_3 = ((pawn & rank_1 & file_h) << 7) & white;
    // Two step forward
    uint64_t pos_4 = (pawn & ~rank_7) >> 16 & ~white;

    return (pos_1 | pos_2 | pos_3 | pos_4) & ~black;
}

//int main(void) {
//    board* new_game = malloc(sizeof(struct board));
//    fill_standard(new_game);
//    printf("Test of pawn %" PRIu64 "\n", new_game->pawn_w);
//    free(new_game);
//    return 0;
//}
