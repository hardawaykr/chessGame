class Chess:
    def __init__(self):
        self.board = deepcopy(standard_board)

class Piece:
    def __init__(self, side, x, y):
        self.side = side
        self.x = x
        self.y = y

    def move(self, board, x, y):
        pass

class Pawn(Piece):
    def move(self, board, x, y):
        xDiff = abs(self.x - x) 
        yDiff = abs(self.y - y)
        piece = board[x][y] 

        if xDiff == 1 ^ yDiff == 1:
            if not piece or piece and piece.side != self.side:
                board[x][y] = self
                self.x = x
                self.y = y
        elif xDiff == 2 ^ yDiff == 2:
            if board == standard_board:
                board[x][y] = self

class Rook(Piece):
    def move(self, board, x, y):
        xDiff = abs(self.x - x) 
        yDiff = abs(self.y - y)
        piece = board[x][y] 

        if piece and piece.side != self.side:
            if yDiff == 0 and horizontal_path_clear(self.x, self.y, x, board):
                self.x = x
                self.y = y
                board[x][y] = self
            elif xDiff == 0 and vertical_path_clear(self.x, self.y, y, board)
                self.x = x
                self.y = y
                board[x][y] = self

class Knight(Piece):
    def move(self, board, x, y):
        xDiff = abs(self.x - x) 
        yDiff = abs(self.y - y)
        piece = board[x][y] 

        if piece and piece.side != self.side or not piece:
            if xDiff == 2 and yDiff == 1:
                self.x = x
                self.y = y
                board[x][y] = self
            elif xDiff == 1 and yDiff == 2:
                self.x = x
                self.y = y
                board[x][y] = self

class Bishop(Piece):
    def move(self, board, x, y):
        pass

class Queen(Piece):
    def move(self, board, x, y):
        pass

class King(Piece):
    def move(self, board, x, y):
        pass

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
        if board[s_x + s * (i + 1)][s_y]:
            return False

    return True

def vertical_path_clear(s_x, s_y, y, board):
    yDiff = y - s_y
    s = 1 if yDiff > 0 else -1
    for i in range(abs(yDiff)):
        if board[s_x][s_y + s * (i + 1)]:
            return False

    return True

def diagonal_path_clear(s_x, s_y, x, y, board):
    yDiff = y - s_y
    s = 1 if yDiff > 0 else -1
    xDiff = x - s_x
    s = 1 if xDiff > 0 else -1
    
    for i in range(xDiff):
        if board[s_x + s * (i + 1)][s_y + s * (i + 1)]:
            return False

    return True
