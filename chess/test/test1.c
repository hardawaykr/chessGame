#include "../chess.c"

int main(void) {
    board* perft_board = board_alloc();
    set_standard(perft_board); 
    uint64_t perft_test_1 = perft(perft_board, 4); 
    if (perft_test_1 != 400) {
        printf("Error invalid perft test 1 %" PRIu64 "\n", perft_test_1);
        printf("The board after perft test 1 is %s\n",board_string(perft_board) );
    } else {
        printf("Success. First perft test success.\n");
    }
    char fen_1[73] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1";
    parse_fen(perft_board, fen_1);
    perft_board->castle_w_l = 0;
    perft_board->castle_w_r = 0;
    perft_board->castle_b_l = 0;
    perft_board->castle_b_r = 0;
    uint64_t perft_test_2 = perft(perft_board, 3); 
    if (perft_test_2 != 48) {
        printf("Error invalid perft test 2 %" PRIu64 "\n", perft_test_2);
        printf("The board after perft test 2 is %s\n",board_string(perft_board));
    } else {
        printf("Success. Second perft test success.\n");
    }
    // Basic test for white legal move generation. 
    board* b = board_alloc();
    set_standard(b);
    uint64_t white_legal_moves = w_legal_moves(b);
    if (white_legal_moves != 0x00000000FFFF0000) {
        printf("Error invalid legal white move gen %" PRIu64 "\n", white_legal_moves);
    } else {
        printf("Success. White legal moves correct.\n");
    }
    
    if (b->king_w != 0x0000000000000010) {
        printf("Error king position %" PRIu64 "\n", b->king_w);
    }
    if (b->white != 0x000000000000FFFF) {
        printf("Error bad white side %" PRIu64 "\n", b->white);
    }
    uint64_t king_w_moves = king_move_board(b->king_w, b->white, b->black); 
    if (king_w_moves != 0) {
        printf("Error invalid move board %" PRIu64 "\n", king_w_moves);
    } else {
        printf("Success.\n");
    }

    uint64_t knight_b_moves = knight_move_board(b->knight_w, b->white);
    if (knight_b_moves != 0x0000000000A50000) {
        printf("Error knight board %" PRIu64 "\n", knight_b_moves);
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
    
    
    char fen[] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    board* b_2 = board_alloc();
    parse_fen(b_2, fen);
    free(b);
    b = board_alloc();
    set_standard(b);
    if ((b->white | b-> black) != 0xFFFF00000000FFFF) {
        printf("Board initialization error %" PRIu64 "\n", b->white | b->black);
    } else {
        printf("Correct board initilization\n");
    }

    b_2->turn = b->turn;
    b_2->castle_w_l = b->castle_w_l;                               
    b_2->castle_w_r = b->castle_w_r;                               
    b_2->castle_b_l = b->castle_b_l;                               
    b_2->castle_b_r = b->castle_b_r;                               
    if (!board_equals(b, b_2)) {
       printf("Incorrect fen position parsing new board %" PRIu64 " correct board %" PRIu64 "\n", b_2->white | b_2->black, b->white | b->black);
       printf("Piece check %d\n", (b->queen_w == b_2->queen_w));
       printf("The black king is %" PRIu64 "\n", b_2->king_w);
    } else {
        printf("Success on fen parsing\n");
    }

    free(b_2);
    char fen_2[] = "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8";
    parse_fen(b, fen_2);
    printf("The board is %s\n", board_string(b));
    free(b);
    return 0;
}    
