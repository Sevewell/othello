import tkinter
import simu
import threading

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
        self.engine_button = tkinter.Button(self, text='探索', command=self.Engine)
        self.engine_button.pack()
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
            self.board.delete('win_rate')
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
            self.stones[row][col] = self.board.create_oval( # マスいっぱいのサイズは見にくい
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

    def Engine(self): # 非同期にしたい
        self.engine_button.config(state="disabled")

        def stream_stdout(pipe):
            for line in iter(pipe.readline, ''):
                result = eval(line.strip())
                if result["value"]["children"]:
                    for child in result["value"]["children"]:
                        child["win_rate"] = child["beta"] / (child["alpha"] + child["beta"])
                        child["index"] = 63 - ((child["move"] & -child["move"]).bit_length() - 1)
                    moves = result["value"]["children"]
                else: # 置ける石がなかった場合
                    moves = []
                self.RenderWinRate(moves)

        def stream_stderr(pipe):
            for line in iter(pipe.readline, ''):
                print(line.strip())

        def search():
            match self.turn.get():
                case "black":
                    process = simu.Execute(self.black, self.white)
                case "white":
                    process = simu.Execute(self.white, self.black)
            thread_out = threading.Thread(target=stream_stdout, args=(process.stdout,))
            thread_err = threading.Thread(target=stream_stderr, args=(process.stderr,))
            thread_out.start()
            thread_err.start()
            process.wait()
            thread_out.join()
            thread_err.join()
            self.engine_button.config(state="normal")

        threading.Thread(target=search, daemon=True).start()

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

    def RenderWinRate(self, moves):
        self.board.delete('win_rate')  # タグで一括削除
        size = self.board_size / 8
        for move in moves:
            index = move['index']
            row, col = index // 8, index % 8
            x = size * col + size / 2
            y = size * row + size / 2
            text = f"{move['win_rate']:.0%}"  # "62%" のような表示
            self.board.create_text(x, y, text=text, fill=self.turn.get(), tags='win_rate')

    def Import(self):
        black_tv = tkinter.IntVar(self)
        white_tv = tkinter.IntVar(self)
        entry_black = tkinter.Entry(self, textvariable=black_tv, name='black')
        entry_white = tkinter.Entry(self, textvariable=white_tv, name='white')
        entry_black.pack()
        entry_white.pack()
        def Input():
            self.black = int(black_tv.get())
            self.white = int(white_tv.get())
            self.Render()
        button_import = tkinter.Button(self, text='入力', command=Input)
        button_import.pack()

root = tkinter.Tk()
