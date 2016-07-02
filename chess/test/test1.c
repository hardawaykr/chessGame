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
    uint64_t king_w_moves = king_move_board(b->king_w, b->white, b->black, b->castle_w_l, b->castle_w_r); 
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
    uint64_t white_rook_moves = rook_move_board(b->rook_w, b->white, b->black);
    if (white_rook_moves != 0) {
        printf("Rook error %" PRIu64 "\n", white_pawn_moves);
    } else {
        printf("Rook valid. \n");
    }
    uint64_t test_rook = 0x0000000008000000;
    uint64_t test_white = 0x0000000048000000;
    uint64_t test_black = 0x0008000000000000;
    uint64_t rook_moves = rook_move_board(test_rook, 0, 0);
    if (rook_moves != 0x08080808F7080808) {
       printf("Test Rook error %" PRIu64 "\n", rook_moves);
    } else {
        printf("Valid rook board. \n");
    }
    rook_moves = rook_move_board(test_rook, test_white, test_black);

    if (rook_moves != 0x0008080837080808) {
       printf("Test Rook error %" PRIu64 "\n", rook_moves);
    } else {
        printf("Valid rook board. \n");
    }
 
    uint64_t white_bishop_moves = bishop_move_board(b->bishop_w, b->white, b->black);
    if (white_bishop_moves != 0) {
       printf("Test bishop error %" PRIu64 "\n", white_bishop_moves);
    } else {
        printf("Success on bishop\n");
    }
    
    uint64_t test_bishop = 0x0000000008000000;
    uint64_t empty = 0;
    uint64_t bishop_board = bishop_move_board(test_bishop, test_bishop, empty);

    if (bishop_board != 0x8041221400142241) {
       printf("Test bishop error %" PRIu64 "\n", bishop_board);
    } else {
        printf("Success on bishop\n");
    }

    test_bishop = 0x0000000008000000;
    uint64_t black = 0x0000001000000000;
    bishop_board = bishop_move_board(test_bishop, test_bishop, black);

    if (bishop_board != 0x0001021400142241) {
       printf("Test bishop error %" PRIu64 "\n", bishop_board);
    } else {
        printf("Success on bishop\n");
    }

    uint64_t queen_moves = queen_move_board(b->queen_b, b->black, b->white); 

    if (queen_moves != 0) {
       printf("Queen 1 error %" PRIu64 "\n", queen_moves);
    } else {
        printf("Success on queen 1\n");
    }

    queen_moves = queen_move_board(0x0000000008000000, 0x0000000002002000, 0x8008000000000000); 
    if (queen_moves != 0x80492A1CF41C0A09) {
       printf("Queen 2 error %" PRIu64 "\n", queen_moves);
    } else {
        printf("Success on queen 2\n");
    }
    
    // King castle test
    uint64_t king_moves = king_move_board(0x1000000000000000, 0x1000000000000000, 0x8000000000000000, 1, 0);
    if (king_moves != 0x6838000000000000) {
       printf("Incorrect king castle test %" PRIu64 "\n", king_moves);
    } else {
        printf("Success on king castle test\n");
    }
        
    king_moves = king_move_board(0x1000000000000000, 0x1000000000000000, 0x8100000000000000, 1, 1);
    if (king_moves != 0x6C38000000000000) {
       printf("Incorrect king castle test 2 %" PRIu64 "\n", king_moves);
    } else {
        printf("Success on king castle test 2\n");
    }
    
    char fen[] = "1 2 3 4 5 6";
    board* b_2 = parse_fen(fen);
    free(b);
    free(b_2);
    return 0;
}    
