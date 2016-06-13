import sys, os
myPath = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, myPath + '/../')

import pytest
from chess.chess import *

def testPawn():
    new_game = Chess() 
    pawn = new_game.get_piece(0, 1)
    assert(type(pawn) == Pawn)
    assert(pawn.get_x() == 0)
    assert(pawn.get_y() == 1)
    assert(pawn.possible_moves(new_game) == set([(0, 2), (0, 3)]))
