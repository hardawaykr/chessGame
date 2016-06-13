from copy import deepcopy
WIDTH = 8
HEIGHT = 8
class Chess(object):
    def __init__(self):
        self.board = deepcopy(standard_board)
        self.castling_w = True
        self.castling_b = True

    def valid_space(self, side, x, y):
        piece = self.board[x][y]
        return not piece or (piece and piece.side != side)

    def add_piece(self, x, y, piece):
        self.board[x][y] = piece
        piece.x = x
        piece.y = y

    def get_piece(self, x, y):
        return self.board[x][y]

class Piece(object):
    def __init__(self, side, x, y):
        self.side = side
        self.x = x
        self.y = y

    def move(self, board, x, y):
        pass 
        
    def can_move(self, board, x, y):
        return False

    def possible_moves(self, board):
        return set()

    def get_x(self):
        return self.x

    def get_y(self):
        return self.y


class Pawn(Piece):
    def __init__(self, side, x, y):
        super().__init__(side, x, y)
        self.en_passant = True

    def can_move(self, board, x, y):
        xDiff = abs(self.x - x) 
        yDiff = abs(self.y - y)
        
        if (xDiff == 1) ^ (yDiff == 1):
            if board.valid_space(self.side, x, y):
                return True
        elif yDiff == 2 and self.en_passant and vertical_path_clear(self.x, self.y, y, board) and board.valid_space(self.side, x, y):
            return True
        return False

    def move(self, board, x, y):
        if self.can_move(board, x, y):
            self.en_passant = False
            board.add_piece(x, y, self)

    def possible_moves(self, board):
        moves = set()
        positions = [(self.x, self.y + 1), (self.x + 1, self.y), (self.x, self.y - 1), (self.x - 1, self.y)]
        for (x, y) in positions:
            if 0 <= x < WIDTH and 0 <= y < HEIGHT and self.can_move(board, x, y):
                moves.add((x, y)) 

        if self.en_passant and self.y + 2 < HEIGHT and self.can_move(board, self.x, self.y + 2):
            moves.add((self.x, self.y + 2))
        return moves

class Rook(Piece):
    def can_move(self, board, x, y):
        xDiff = abs(self.x - x) 
        yDiff = abs(self.y - y)

        if board.valid_space(self.side, x, y):
            if yDiff == 0 and horizontal_path_clear(self.x, self.y, x, board):
                return True
            elif xDiff == 0 and vertical_path_clear(self.x, self.y, y, board):
                return True
        return False

    def move(self, board, x, y):
        if self.can_move(board, x, y):
            board.add_piece(x, y, self)

class Knight(Piece):
    def can_move(self, board, x, y):
        xDiff = abs(self.x - x) 
        yDiff = abs(self.y - y)

        if board.valid_side(self.side, x, y):
            if xDiff == 2 and yDiff == 1:
                return True
            elif xDiff == 1 and yDiff == 2:
                return True
        return False

    def move(self, board, x, y):
        if self.can_move(board, x, y):
            board.add_piece(x, y, self)

class Bishop(Piece):
    def can_move(self, board, x, y):
        xDiff = abs(self.x - x) 
        yDiff = abs(self.y - y)
        
        if xDiff == yDiff and diagonal_path_clear(self.x, self.y, x, y, board):
            if board.valid_space(self.side, x, y):
                return True
        return False

    def move(self, board, x, y):
        if self.can_move(board, x, y):
            board.add_piece(x, y, self)

class Queen(Piece):
    def can_move(self, board, x, y):
        piece = board.get_piece(x, y)
        if board.valid_space(self.side, x, y) and any_path_clear(self.x, self.y, x, y, board):
            return True
        return False

    def move(self, board, x, y):
        if self.can_move(board, x, y):
            board.add_piece(x, y, self)
            
class King(Piece):
    def can_move(self, board, x, y):
        xDiff = abs(self.x - x) 
        yDiff = abs(self.y - y)
        
        if xDiff <= 1 and yDiff <= 1:
            if board.valid_space(self.side, x, y):
                return True
        return False

    def move(self, board, x, y):
        if self.can_move(board, x, y):
            board.add_piece(x, y, self)

standard_board = [[Rook('w', 0, 0), Pawn('w', 0, 1), None, None, None, None, Pawn('b', 0, 6), Rook('b', 0, 7)],
                  [Knight('w', 1, 0), Pawn('w', 1, 1), None, None, None, None, Pawn('b', 1, 6), Knight('b', 1, 7)],      
                  [Bishop('w', 2, 0), Pawn('w', 2, 1), None, None, None, None, Pawn('b', 2, 6), Bishop('b', 2, 7)],
                  [King('w', 3, 0), Pawn('w', 3, 1), None, None, None, None, Pawn('b', 3, 6), King('b', 3, 7)],
                  [Queen('w', 4, 0), Pawn('w', 4, 1), None, None, None, None, Pawn('b', 4, 6), Queen('b', 4, 7)],
                  [Bishop('w', 5, 0), Pawn('w', 5, 1), None, None, None, None, Pawn('b', 5, 6), Bishop('b', 5, 7)],  
                  [Knight('w', 6, 0), Pawn('w', 6, 1), None, None, None, None, Pawn('b', 6, 6), Knight('b', 6, 7)],
                  [Rook('w', 7, 0), Pawn('w', 7, 1), None, None, None, None, Pawn('b', 7, 6), Rook('b', 7, 7)]]


def horizontal_path_clear(s_x, s_y, x, board):
    xDiff = x - s_x
    s = 1 if xDiff > 0 else -1
    for i in range(abs(xDiff)):
        if board.get_piece(s_x + s * (i + 1), s_y):
            return False

    return True

def vertical_path_clear(s_x, s_y, y, board):
    yDiff = y - s_y
    s = 1 if yDiff > 0 else -1
    for i in range(1, abs(yDiff)):
        if board.get_piece(s_x, s_y + s * i):
            return False

    return True

def diagonal_path_clear(s_x, s_y, x, y, board):
    yDiff = y - s_y
    s = 1 if yDiff > 0 else -1
    xDiff = x - s_x
    s = 1 if xDiff > 0 else -1
    
    for i in range(xDiff):
        if board.get_piece(s_x + s * (i + 1), s_y + s * (i + 1)):
            return False

    return True

def any_valid_path(s_x, s_y, x, y, board):
    return  diagonal_path_clear(s_x, s_y, x, y, board) or horizontal_path_clear(s_x, s_y, x, board) or vertical_path_clear(s_x, s_y, y, board)
