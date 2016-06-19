#include "../chess.c"

int main(void) {
    board* b = malloc(sizeof(board));
    fill_standard(b);
    if (b->king_w != 0x0000000000000010) {
        printf("Error king position %" PRIu64 "\n", b->king_w);
    }
    if (b->white != 0x000000000000FFFF) {
        printf("Error bad white side %" PRIu64 "\n", b->white);
    }
    uint64_t king_w_moves = king_move_board(b->king_w, b->white); 
    if (king_w_moves != 0) {
        printf("Error invalid move board %" PRIu64 "\n", king_w_moves);
    } else {
        printf("Success.\n");
    }

    uint64_t knight_w_moves = knight_move_board(b->knight_w, b->white);
    if (knight_w_moves != 0x0000000000A50000) {
        printf("Error knight board %" PRIu64 "\n", knight_w_moves);
    } else {
        printf("knight success \n");
    }

    uint64_t white_pawn_moves = pawn_w_move_board(b->pawn_w, b->white, b->black);
    if (white_pawn_moves != 0x00000000FFFF0000) {
        printf("Pawn error %" PRIu64 "\n", white_pawn_moves);
    } else {
        printf("Pawn success \n");
    }

    uint64_t black_pawn_moves = pawn_b_move_board(b->pawn_b, b->white, b->black);
    if (black_pawn_moves != 0x0000FFFF00000000) {
        printf("Pawn error %" PRIu64 "\n", black_pawn_moves);
    } else {
        printf("Pawn success \n");
    }
    return 0;
}    
