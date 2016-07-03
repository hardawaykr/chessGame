#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include "dbg.h"

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

void board_delete(board *b) {
    free(b);
}

/* 
 * Fills each side, white and black, in bitboard with respective sides pieces. 
*/
void fill_sides(board *b) {
    b->white = b->pawn_w | b->queen_w | b->king_w | b->bishop_w | b->knight_w | b->rook_w;
    b->black = b->pawn_b | b->queen_b | b->king_b | b->bishop_b | b->knight_b | b->rook_b;
}

/* 
 * Sets each piece in board to starting represenation. 
 * Also initializes boolean values for check and castling.
*/
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

    fill_sides(b);

    b->castle_w_l = 0;
    b->castle_w_r = 0;
    b->castle_b_l = 0;
    b->castle_b_r = 0;

    b->check_w = 0;
    b->check_b = 0;
    return 0;
}

/* 
 * Generates all legal moves for the king and returns them in one combined board. 
 * Requires both castling booleans for the current side to generate castling move for king. 
*/
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
        pos_9 = ((pos_9 & file_a) >> 1) & ~other_side & ~own_side;

        // Check that next space is free for rook to slide through
        uint64_t space = ((pos_9 & file_a) >> 1) & (other_side | own_side);
        if (space) {
            pos_9 = 0;
        }

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

uint64_t b_move_board(board *b) {
    uint64_t pawn_moves = pawn_b_move_board(b->pawn_b, b->white, b->black);
    uint64_t queen_moves = queen_move_board(b->queen_b, b->black, b->white);
    uint64_t king_moves = king_move_board(b->king_b, b->black, b->white, b->castle_w_l, b->castle_w_r);
    uint64_t rook_moves = rook_move_board(b->rook_b, b->black, b->white);
    uint64_t knight_moves = knight_move_board(b->knight_b, b->black);
    uint64_t bishop_moves = bishop_move_board(b->bishop_b, b->black, b->white);

    return pawn_moves | queen_moves | king_moves | rook_moves | knight_moves | bishop_moves;
}

uint64_t w_move_board(board *b) {
    uint64_t pawn_moves = pawn_w_move_board(b->pawn_w, b->white, b->black);
    uint64_t queen_moves = queen_move_board(b->queen_w, b->white, b->black);
    uint64_t king_moves = king_move_board(b->king_w, b->white, b->black, b->castle_w_l, b->castle_w_r);
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

board* board_copy(board *b) {
    board *new_board = board_alloc();
    
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
    fill_sides(new_board);
    new_board->castle_w_l = b->castle_w_l;
    new_board->castle_w_r = b->castle_w_r;
    new_board->castle_b_l = b->castle_b_l;
    new_board->castle_b_r = b->castle_b_r;

    return new_board;
}

board* get_intersecting_w(board *b, uint64_t spaces) {
    board* new_board = board_copy(b);
    new_board->pawn_w &= spaces;
    new_board->queen_w &= spaces;
    new_board->king_w &= spaces;
    new_board->rook_w &= spaces;
    new_board->knight_w &= spaces;
    new_board->bishop_w &= spaces;

    return new_board;
}

board* get_intersecting_b(board *b, uint64_t spaces) {
    board* new_board = board_copy(b);
    new_board->pawn_b &= spaces;
    new_board->queen_b &= spaces;
    new_board->king_b &= spaces;
    new_board->rook_b &= spaces;
    new_board->knight_b &= spaces;
    new_board->bishop_b &= spaces;

    return new_board;
}

uint64_t w_legal_moves(board* b) {
    // First generate moves of pinned pieces, ie pieces limited by placing king in check
    uint64_t rook_attacks_to_king = rook_move_board(b->king_w, b->white, b->black)  & b->black;
    // Board with potentially pinned pieces ie pieces in possible rook attack path to king.
    board* b_pinned = get_intersecting_w(b, rook_attacks_to_king);
    // Compute new board without pinned pieces by finding intersection of board and the flip of all pinned pieces. 
    board* b_unpinned = get_intersecting_w(b, ~(b_pinned->white));
    // Re-compute rook attacks with new board.
    rook_attacks_to_king = rook_move_board(b_unpinned->king_w, b_unpinned->white,b_unpinned->black) & b_unpinned->black;
    
    // Find all enemy, black, pinning pieces by finding black intersection with rook attacks from king
    board* b_pinners = get_intersecting_b(b, rook_attacks_to_king);
    uint64_t pinners = b_pinners->rook_b | b_pinners->queen_b;
    uint64_t pinner_attacks = rook_move_board(pinners, b_pinners->black, b_pinners->white);
    // Intersect pinner_attacks with pinned pieces to find actually pinned pieces.
    b_pinned = get_intersecting_w(b_pinned, pinner_attacks);
    uint64_t pinned_piece_moves = w_move_board(b_pinned);
    // Intersect with pinner_attacks and pinners to find moves which capture pinner or move along attacking path.
    pinned_piece_moves &= (pinner_attacks | pinners); 

    // Remove pinned pieces from rest of move generation.
    b = get_intersecting_w(b, ~(b_pinned->white));

    return 0;
}

void play_game() {
    board* b = board_alloc();
    fill_standard(b);
    

}

char* get_move() {
    return "";
}

/* 
 * Converts an input fen representation into internal bitboard representation. 
 * Used to parse various game boards for move evaluation. 
*/
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
