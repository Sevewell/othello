import tkinter
from tkinter import ttk
import search
import threading
import pickle
import os
import sys

seed = sys.argv[1]

class Start(tkinter.Tk):

    def __init__(self):

        super().__init__()
        self.title('Othello')
        self.CreateMode()
        self.CreateStart()

    def CreateMode(self):

        self.mode = tkinter.StringVar()
        self.mode.set('対戦')

        for mode in ['対戦', '観戦']:
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

        self.black = 34628173824
        self.white = 68853694464
        
        self.CreateBoard()
        self.CreateControl()

        if mode == '対戦':

            for i,panel in enumerate(self.board):
                panel.config(command=self.Play(i))

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

            if move & (self.black | self.white):                
                self.black = self.black & (~move)
                self.white = self.white & (~move)
            
            else:
                if self.turn.get() == '黒番':
                    self.black = self.black | move
                elif self.turn.get() == '白番':
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

        n = 100

        for i in range(n):

            self.black = 34628173824
            self.white = 68853694464
            self.turn.set('黒番')

            learning_rate_b = search.random.random() * 2 + 1.0
            learning_rate_w = search.random.random() * 2 + 1.0
            self.param.set(learning_rate_b)

            self.trial.set(1000000)

            self.Draw()

            print('{}/{}'.format(i + 1, n), end=' ')

            while True:

                children = self.Search()

                if children:

                    children.sort(key=lambda x: x['winrate'])
                    choice = children[0]

                    if self.turn.get() == '黒番':
                        self.black = choice['y']
                        self.white = choice['m']
                        self.param.set(learning_rate_w)
                        self.turn.set('白番')
                    else:
                        self.black = choice['m']
                        self.white = choice['y']
                        self.param.set(learning_rate_b)
                        self.turn.set('黒番')

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

            record.append({'lr_b': learning_rate_b, 'lr_w': learning_rate_w, 'winner': winner})

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

        trial = 2000000
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

        self.param = tkinter.DoubleVar()
        self.param.set(2.0)
        scale_param = tkinter.Scale(
            control,
            orient='horizontal',
            variable=self.param,
            resolution=0.001,
            from_=1.0,
            to=3.0
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

    def Info(self, info):

        info['turn'] = self.turn.get()
        info['param'] = '{:.3f}'.format(self.param.get())
        #info['trial'] = '{:,}'.format(self.trial.get())
        info['time'] = '{:.3f}'.format(info['time'])
        info['winrate'] = '{:.3f}'.format(info['winrate'])
        print(info)

    #着手可能箇所がなかったら？
    def Search(self):

        turn = self.turn.get()

        if turn == '黒番':
            children = search.Search(
                self.black,
                self.white,
                self.param.get(),
                self.trial.get(),
                self.core.get(),
                seed
            )
            #self.Info(info)
            #self.turn.set('白番')
        elif turn == '白番':
            children = search.Search(
                self.white,
                self.black,
                self.param.get(),
                self.trial.get(),
                self.core.get(),
                seed
            )
            #self.Info(info)
            #self.turn.set('黒番')

        for child in children:
            now = self.black | self.white
            new = child['m'] | child['y']
            move = format(now ^ new, '064b').find('1')
            column = 'ABCDEFGH'[move % 8]
            row = move // 8 + 1
            print('{}{} {:.5f} {} {}'.format(column, row, 1 - child['winrate'], child['a'], child['b']))
        print()

        return children
        #self.Draw()
