import tkinter
import search

class Start(tkinter.Tk):

    def __init__(self):

        super().__init__()
        self.title('Othello')
        self.CreateMode()
        self.CreateStart()

    def CreateMode(self):

        self.mode = tkinter.StringVar()
        self.mode.set('play')
        game = tkinter.Radiobutton(self, text='対戦', variable=self.mode, value='play')
        game.pack()
        question = tkinter.Radiobutton(self, text='詰めオセロ', variable=self.mode, value='question')
        question.pack()

    def CreateStart(self):

        button = tkinter.Button(self, text='開始', command=self.Start)
        button.pack()

    def Start(self):

        self.destroy()
        main = Root(self.mode.get())
        main.mainloop()

# このクラスにターンやら石やらノードやら
# データはあくまでNodeで、それを描画するだけにしたいよね
# うーん、いっそBoardをRootにするか？
class Root(tkinter.Tk):

    def __init__(self, mode):
        
        super().__init__()
        self.title('Othello')
        self.config(menu=Menu(self, mode))

        #self.node = search.Node(34628173824, 68853694464)
        self.black = 34628173824
        self.white = 68853694464
        
        self.CreateBoard(mode)
        self.CreateControl()

        # ボードに探索関数を渡す
        # 探索関数は対戦用詰めオセロ用がある
        # 探索時間を引数に持つ
        # ボタンインデックスを引数に持つ

        # ボード生成後にボタンウィジェットに関数セットもあり

    def CreateBoard(self, mode):

        board = tkinter.Frame(self)
        board.pack()

        if mode == 'play':
            self.board = [tkinter.Button(board, command=self.Play(i)) for i in range(64)]
        elif mode == 'question':
            self.board = [tkinter.Button(board, command=self.Question(i)) for i in range(64)]
        for i,panel in enumerate(self.board):
            panel.grid(column=i%8, row=i//8)

        self.color_black = tkinter.PhotoImage(file='img/black.png')
        self.color_white = tkinter.PhotoImage(file='img/white.png')
        self.color_panel = tkinter.PhotoImage(file='img/panel.png')

        self.Draw()

    def Draw(self):

        for i,panel in enumerate(self.board):

            bit = 2**(63 - i)

            if self.black & bit:
                color = self.color_black
            elif self.white & bit:
                color = self.color_white
            else:
                color = self.color_panel

            panel.config(image=color)

    def Play(self, i):

        def Put():

            move = 2**(63 - i)
            self.black, self.white = search.Move(self.black, self.white, move)
            self.Draw()

        return Put

    def Question(self, i):

        def Put():

            move = 2**(63 - i)

            # 既に石があったら空にする
            if move & (self.black | self.white):
                
                self.black = self.black & (~move)
                self.white = self.white & (~move)
            
            else:

                if self.turn.get() == 'b':
                    self.black = self.black | move
                elif self.turn.get() == 'w':
                    self.white = self.white | move

            self.Draw()

        return Put

    def CreateControl(self):

        control = tkinter.Frame(self)
        control.pack()

        starter = tkinter.Button(control, text='探索開始', command=self.Search)
        starter.pack()

        self.seconds = tkinter.IntVar()
        self.seconds.set(30)

        seconds_scale = tkinter.Scale(
            control,
            orient='horizontal',
            variable=self.seconds,
            from_=1,
            to=60
            )
        seconds_scale.pack()

    #着手可能箇所がなかったら？
    def Search(self):

        #self.white, self.black, info = search.SearchSingle(self.white, self.black, self.seconds.get()) #白番の探索
        self.white, self.black, info = search.SearchMulti(self.white, self.black, self.seconds.get()) #白番の探索
        print(info)
        print('...')
        self.Draw() # 描画

class Menu(tkinter.Menu):

    def __init__(self, master, mode):

        super().__init__(master)

        if mode == 'question':
            self.turn = tkinter.StringVar()
            self.turn.set('b')
            self.menu_turn = self.CreateTurn()
            self.add_cascade(menu=self.menu_turn, label='Turn')

    def CreateTurn(self):

        menu_turn = tkinter.Menu(self)
        menu_turn.add_radiobutton(label='Black', variable=self.turn, value='b')
        menu_turn.add_radiobutton(label='White', variable=self.turn, value='w')
        return menu_turn
