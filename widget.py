import tkinter
from tkinter import ttk
import search
import threading
import pickle
import os

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

        #self.black = 34628173824
        #self.white = 68853694464
        self.black = 136391182778368
        self.white = 68988960768
        #self.black = 1152324562583552
        #self.white = 481036337152
        #self.black = 9230168337524523777
        #self.white = 9216575736185027838
        #self.black = 136391316996096
        #self.white = 68853956608
        
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
        board.pack(side='left')

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
            turn = self.turn.get()
            if turn == '黒番':
                self.turn.set('白番')
            elif turn == '白番':
                self.turn.set('黒番')
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
        if os.path.isfile(name):
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
            n_b = search.random.randint(10000, 200000)
            n_w = search.random.randint(10000, 200000)

            while True:

                if search.CheckEnd(self.black, self.white):
                    self.param.set(hp_b)
                    self.trial.set(n_b)
                    self.Search()
                    self.Draw()
                else:
                    break

                if search.CheckEnd(self.black, self.white):
                    self.param.set(hp_w)
                    self.trial.set(n_w)
                    self.Search()
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

            record.append({'hp_b':hp_b, 'hp_w':hp_w, 'n_b':n_b, 'n_w':n_w, 'winner':winner})

        with open('record.pkl', 'wb') as f:
            pickle.dump(record, f)

    def CreateControl(self):

        control = tkinter.Frame(self)
        control.pack(side='right')

        self.turn = tkinter.StringVar()
        self.turn.set('黒番')
        button_turn_b = tkinter.Radiobutton(
            control,
            text='黒番',
            value='黒番',
            variable=self.turn
        )
        button_turn_w = tkinter.Radiobutton(
            control,
            text='白番',
            value='白番',
            variable=self.turn
        )
        button_turn_b.pack()
        button_turn_w.pack()

        trial = 200000
        self.trial = tkinter.IntVar()
        self.trial.set(trial // 2)
        trial_scale = tkinter.Scale(
            control,
            orient='horizontal',
            variable=self.trial,
            from_=1,
            to=trial
            )
        trial_scale.pack()

        param = 5
        self.param = tkinter.DoubleVar()
        self.param.set(param / 2)
        scale_param = tkinter.Scale(
            control,
            orient='horizontal',
            variable=self.param,
            resolution=0.01,
            from_=0,
            to=param
            )
        scale_param.pack()

        core = os.cpu_count()
        self.core = tkinter.IntVar()
        self.core.set(core // 2)
        core_scale = tkinter.Scale(
            control,
            orient='horizontal',
            variable=self.core,
            from_=1,
            to=core
            )
        core_scale.pack()

        starter = tkinter.Button(control, text='探索開始', command=self.StartSearch)
        starter.pack()

    def StartSearch(self):

        thread = threading.Thread(target=self.Search)
        thread.start()

    #着手可能箇所がなかったら？
    def Search(self):

        turn = self.turn.get()

        if turn == '黒番':
            self.black, self.white = search.SearchC(
                self.black,
                self.white,
                self.param.get(),
                self.trial.get(),
                self.core.get()
            )
            self.turn.set('白番')
        elif turn == '白番':
            self.white, self.black = search.SearchC(
                self.white,
                self.black,
                self.param.get(),
                self.trial.get(),
                self.core.get()
            )
            self.turn.set('黒番')

        self.Draw()
