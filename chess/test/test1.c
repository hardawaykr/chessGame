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
    return 0;
}    
