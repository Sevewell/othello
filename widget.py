import tkinter
import search
import threading
import pickle

class Start(tkinter.Tk):

    def __init__(self):

        super().__init__()
        self.title('Othello')
        self.CreateMode()
        self.CreateStart()

    def CreateMode(self):

        self.mode = tkinter.StringVar()
        self.mode.set('対戦')

        for mode in ['対戦', '詰め', '観戦']:
            radiobutton = tkinter.Radiobutton(self, text=mode, variable=self.mode, value=mode)
            radiobutton.pack()

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

        self.black = 34628173824
        self.white = 68853694464
        
        self.CreateBoard()
        self.CreateControl()

        if mode == '対戦':

            for i,panel in enumerate(self.board):
                panel.config(command=self.Play(i))

        elif mode == '詰め':

            for i,panel in enumerate(self.board):
                panel.config(command=self.Question(i))

        elif mode == '観戦':

            thread = threading.Thread(target=self.Watch)
            thread.start()

    def CreateBoard(self):

        board = tkinter.Frame(self)
        board.pack()

        self.board = [tkinter.Button(board) for i in range(64)]
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

    def Watch(self):

        name = 'record.pkl'
        if search.os.path.isfile(name):
            with open('record.pkl', 'rb') as f:
                record = pickle.load(f)
        else:
            record = []

        for i in range(10):

            self.black = 34628173824
            self.white = 68853694464
            self.Draw()

            hp_b = search.random.random() * 5
            hp_w = search.random.random() * 5
            print('black: {}'.format(hp_b))
            print('white: {}'.format(hp_w))

            while True:

                if search.CheckEnd(self.black, self.white):
                    search.hyper_param = hp_b
                    self.black, self.white, info = search.SearchMulti(self.black, self.white, self.trial.get())
                    self.Draw()
                else:
                    break

                if search.CheckEnd(self.black, self.white):
                    search.hyper_param = hp_w
                    self.white, self.black, info = search.SearchMulti(self.white, self.black, self.trial.get())
                    self.Draw()
                else:
                    break

            count_b = bin(self.black).count('1')
            count_w = bin(self.white).count('1')
            if count_b > count_w:
                winner = 'black'
            elif count_b < count_w:
                winner = 'white'
            else:
                winner = 'draw'
            print('winner: {}'.format(winner))

            record.append({'hp_b':hp_b, 'hp_w':hp_w, 'winner':winner})

        with open('record.pkl', 'wb') as f:
            pickle.dump(record, f)

    def CreateControl(self):

        control = tkinter.Frame(self)
        control.pack()

        starter = tkinter.Button(control, text='探索開始', command=self.Search)
        starter.pack()

        self.trial = tkinter.IntVar()
        self.trial.set(50000)

        trial_scale = tkinter.Scale(
            control,
            orient='horizontal',
            variable=self.trial,
            from_=1,
            to=100000
            )
        trial_scale.pack()

    #着手可能箇所がなかったら？
    def Search(self):

        #self.white, self.black, info = search.SearchSingle(self.white, self.black, self.seconds.get()) #白番の探索
        self.white, self.black, info = search.SearchMulti(self.white, self.black, self.trial.get()) #白番の探索
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
