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
        button_export = tkinter.Button(self, text='出力', command=self.Export)
        button_export.pack()
        self.Import()
    
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
                if self.board.itemcget(self.stones[row][col], 'fill') == 'black':
                    self.board.itemconfigure(self.stones[row][col], fill='white')
                elif self.board.itemcget(self.stones[row][col], 'fill') == 'white':
                    self.board.itemconfigure(self.stones[row][col], fill='green')
                elif self.board.itemcget(self.stones[row][col], 'fill') == 'green':
                    self.board.itemconfigure(self.stones[row][col], fill='black')
            elif event.num == 3:
                self.board.itemconfigure(self.stones[row][col], fill='green')
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
        for i in range(64):
            row = i // 8
            col = i % 8
            self.stones[row][col] = self.board.create_oval(
                size*col,size*row,
                size*col+size,size*row+size,
                fill='green'
            )
        self.board.itemconfigure(self.stones[3][3], fill='white')
        self.board.itemconfigure(self.stones[3][4], fill='black')
        self.board.itemconfigure(self.stones[4][3], fill='black')
        self.board.itemconfigure(self.stones[4][4], fill='white')

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

    def ConvertBits(self):
        black = ''
        white = ''
        for row in self.stones:
            for stone in row:
                if self.board.itemcget(stone, 'fill') == 'black':
                    black += '1'
                    white += '0'
                elif self.board.itemcget(stone, 'fill') == 'white':
                    black += '0'
                    white += '1'
                elif self.board.itemcget(stone, 'fill') == 'green':
                    black += '0'
                    white += '0'
        return black, white

    def Engine(self):
        turn = self.turn.get()
        black, white = self.ConvertBits()
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
                self.board.itemconfigure(self.stones[row][col], fill=turn)
            elif set_[1] == '1' and set_[2] == '1':
                self.board.itemconfigure(self.stones[row][col], fill=turn)
    
    def Export(self):
        black, white = self.ConvertBits()
        print('black', black, int(black, 2))
        print('white', white, int(white, 2))

    def Import(self):
        black = tkinter.IntVar(self)
        white = tkinter.IntVar(self)
        entry_black = tkinter.Entry(self, textvariable=black, name='black')
        entry_white = tkinter.Entry(self, textvariable=white, name='white')
        entry_black.pack()
        entry_white.pack()
        def Output():
            size = int(self.board_size / 8)
            for i in range(64):
                row = i // 8
                col = i % 8
                self.board.itemconfigure(self.stones[row][col], fill='green')
            for i,b in enumerate(format(black.get(), '064b')):
                if b == '1':
                    row = i // 8
                    col = i % 8
                    self.board.itemconfigure(self.stones[row][col], fill='black')
            for i,w in enumerate(format(white.get(), '064b')):
                if w == '1':
                    row = i // 8
                    col = i % 8
                    self.board.itemconfigure(self.stones[row][col], fill='white')
        button_import = tkinter.Button(self, text='入力', command=Output)
        button_import.pack()

root = tkinter.Tk()
