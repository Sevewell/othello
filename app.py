import tkinter

class App(tkinter.Frame):

    def __init__(self, master):
        super().__init__(master)
        self.pack()
        self.board_size = 480
        self.board = self.Board()
        self.Cell()
        self.stones = [[None for col in range(8)] for row in range(8)]
        self.Stone()
    
    def Board(self):
        canvas = tkinter.Canvas(
            None,
            background='green',
            width=self.board_size,
            height=self.board_size
        )
        def ChangeStone(event):
            size = int(self.board_size / 8)
            col = event.x // size
            row = event.y // size
            if not self.stones[col][row]:
                self.stones[col][row] = self.board.create_oval(
                    size*col,size*row,
                    size*col+size,size*row+size,
                    fill='black'
                )
            elif self.board.itemcget(self.stones[col][row], 'fill') == 'black':
                self.board.itemconfigure(self.stones[col][row], fill='white')
            elif self.board.itemcget(self.stones[col][row], 'fill') == 'white':
                self.board.delete(self.stones[col][row])
                self.stones[col][row] = None
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
        self.stones[4][3] = self.board.create_oval(
            size*4,size*3,
            size*4+size,size*3+size,
            fill='black'
        )
        self.stones[3][4] = self.board.create_oval(
            size*3,size*4,
            size*3+size,size*4+size,
            fill='black'
        )
        self.stones[4][4] = self.board.create_oval(
            size*4,size*4,
            size*4+size,size*4+size,
            fill='white'
        )

root = tkinter.Tk()
app = App(root)
app.mainloop()
