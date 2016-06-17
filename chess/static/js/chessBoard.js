var board1 = ChessBoard('board1', {
    draggable:  true,
    dropOffBoard: 'trash'
});

$('#startBtn').on('click', board1.start)
$('#clearBtn').on('click', board1.clear)
