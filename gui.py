import tkinter
import simu

class App(tkinter.Frame):

    def __init__(self, master):
        super().__init__(master)
        self.pack()
        self.black = 34628173824
        self.white = 68853694464
        self.board_size = 480
        self.board = self.Board()
        self.Cell()
        self.stones = [[None for col in range(8)] for row in range(8)]
        self.Stone()
        self.InputToComputer()
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
            index = 8 * row + col
            black_bits = '{:064b}'.format(self.black)
            white_bits = '{:064b}'.format(self.white)
            if event.num == 1:
                if self.board.itemcget(self.stones[row][col], 'fill') == 'black':
                    black_bit = '0'
                    white_bit = '1'
                elif self.board.itemcget(self.stones[row][col], 'fill') == 'white':
                    black_bit = '0'
                    white_bit = '0'
                elif self.board.itemcget(self.stones[row][col], 'fill') == 'green':
                    black_bit = '1'
                    white_bit = '0'
            elif event.num == 3:
                black_bit = '0'
                white_bit = '0'
            self.black = int(black_bits[:index] + black_bit + black_bits[index+1:], 2)
            self.white = int(white_bits[:index] + white_bit + white_bits[index+1:], 2)
            self.Render()
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
        self.Render()

    def InputToComputer(self):
        self.turn = tkinter.StringVar()
        self.turn.set('black')
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
        if turn == 'black':
            move = simu.Explore(self.black, self.white)
            self.black = move['m']
            self.white = move['y']
        if turn == 'white':
            move = simu.Explore(self.white, self.black)
            self.black = move['y']
            self.white = move['m']
        self.Render()
    
    def Export(self):
        print('black', self.black)
        print('white', self.white)

    def Render(self):
        assert not (self.black & self.white)
        black_bits = '{:064b}'.format(self.black)
        white_bits = '{:064b}'.format(self.white)
        for i in range(64):
            row = i // 8
            col = i % 8
            if black_bits[i] == '1':
                color = 'black'
            elif white_bits[i] == '1':
                color = 'white'
            else:
                color = 'green'
            self.board.itemconfigure(self.stones[row][col], fill=color)

    def Import(self):
        black_tv = tkinter.IntVar(self)
        white_tv = tkinter.IntVar(self)
        entry_black = tkinter.Entry(self, textvariable=black_tv, name='black')
        entry_white = tkinter.Entry(self, textvariable=white_tv, name='white')
        entry_black.pack()
        entry_white.pack()
        def Input(self):
            black = int(black_tv.get())
            white = int(white_tv.get())
            self.Render()
        button_import = tkinter.Button(self, text='入力', command=Input)
        button_import.pack()

root = tkinter.Tk()
