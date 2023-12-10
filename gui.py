import tkinter

class App(tkinter.Frame):

    def __init__(self, master, engine):
        super().__init__(master)
        self.pack()
        self.board_size = 480
        self.board = self.Board()
        self.Cell()
        self.stones = [[None for col in range(8)] for row in range(8)]
        self.Stone()
        self.Turn()
        self.turn.set('black')
        self.engine = engine
        button = tkinter.Button(self, text='探索', command=self.Engine)
        button.pack()
    
    def Board(self):
        canvas = tkinter.Canvas(
            self,
            background='green',
            width=self.board_size,
            height=self.board_size
        )
        def ChangeStone(event):
            size = int(self.board_size / 8)
            col = event.x // size
            row = event.y // size
            if event.num == 1:
                if self.stones[row][col]:
                    if self.board.itemcget(self.stones[row][col], 'fill') == 'black':
                        self.board.itemconfigure(self.stones[row][col], fill='white')
                    elif self.board.itemcget(self.stones[row][col], 'fill') == 'white':
                        self.board.itemconfigure(self.stones[row][col], fill='black')
                else:
                    self.stones[row][col] = self.board.create_oval(
                        size*col,size*row,
                        size*col+size,size*row+size,
                        fill=self.turn.get()
                    )
            elif event.num == 3:
                self.board.delete(self.stones[row][col])
                self.stones[row][col] = None
        canvas.bind('<ButtonPress>', ChangeStone)
        canvas.pack()
        return canvas
    
    def Cell(self):
        size = self.board_size / 8
        for column in range(8):
            self.board.create_line(
                size*column,0,
                size*column,size*8
            )
        for row in range(8):
            self.board.create_line(
                0,size*row,
                size*8,size*row
            )

    def Stone(self):
        size = self.board_size / 8
        self.stones[3][3] = self.board.create_oval(
            size*3,size*3,
            size*3+size,size*3+size,
            fill='white'
        )
        self.stones[3][4] = self.board.create_oval(
            size*4,size*3,
            size*4+size,size*3+size,
            fill='black'
        )
        self.stones[4][3] = self.board.create_oval(
            size*3,size*4,
            size*3+size,size*4+size,
            fill='black'
        )
        self.stones[4][4] = self.board.create_oval(
            size*4,size*4,
            size*4+size,size*4+size,
            fill='white'
        )

    def Turn(self):
        self.turn = tkinter.StringVar()
        button_black = tkinter.Radiobutton(
            self,
            text='黒番',
            value='black',
            variable=self.turn
        )
        button_black.pack()
        button_white = tkinter.Radiobutton(
            self,
            text='白番',
            value='white',
            variable=self.turn
        )
        button_white.pack()

    def Engine(self):
        turn = self.turn.get()
        black = ''
        white = ''
        for row in self.stones:
            for stone in row:
                if stone:
                    if self.board.itemcget(stone, 'fill') == 'black':
                        black += '1'
                        white += '0'
                    if self.board.itemcget(stone, 'fill') == 'white':
                        black += '0'
                        white += '1'
                else:
                    black += '0'
                    white += '0'
        if turn == 'black':
            move, m, y = self.engine(black, white)
            old_new = zip(move, m, white)
        if turn == 'white':
            move, m, y = self.engine(white, black)
            old_new = zip(move, m, black)
        for i,set_ in enumerate(old_new):
            col = i % 8
            row = i // 8
            if set_[0] == '1':
                size = self.board_size / 8
                self.stones[row][col] = self.board.create_oval(
                    size*col,size*row,
                    size*col+size,size*row+size,
                    fill=turn
                )
            elif set_[1] == '1' and set_[2] == '1':
                self.board.itemconfigure(self.stones[row][col], fill=turn)

root = tkinter.Tk()
