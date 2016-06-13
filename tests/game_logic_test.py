import sys, os
myPath = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, myPath + '/../')

import pytest
from chess.chess import *

def testPawn():
    new_game = Chess() 
    `
