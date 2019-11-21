import tkinter

class Root(tkinter.Tk):

    def __init__(self):
        
        super().__init__()
        self.title('Othello')

class Menu(tkinter.Menu):

    def __init__(self, master=None):

        super().__init__(master)

        self.mode = tkinter.StringVar()
        self.mode.set('player')
        self.add_cascade(menu=self.CreateMode(), label='Mode')

        self.turn = tkinter.StringVar()
        self.turn.set('b')
        self.add_cascade(menu=self.CreateTurn(), label='Turn')

    def CreateMode(self):

        menu_mode = tkinter.Menu(self)
        menu_mode.add_radiobutton(label='Player', variable=self.mode, value='player')
        menu_mode.add_radiobutton(label='Question', variable=self.mode, value='question')
        return menu_mode

    def CreateTurn(self):

        menu_turn = tkinter.Menu(self)
        menu_turn.add_radiobutton(label='Black', variable=self.turn, value='b')
        menu_turn.add_radiobutton(label='White', variable=self.turn, value='w')
        return menu_turn

class Board(tkinter.Frame):

    def __init__(self, master=None, mode=None, turn=None):

        super().__init__(master)
        self.black = 34628173824
        self.white = 68853694464
        self.mode = mode
        self.turn = turn

        self.board = [tkinter.Button(self, command=self.Put(i)) for i in range(64)]
        for i,panel in enumerate(self.board):
            panel.grid(column=i%8, row=i//8)

        self.color_black = tkinter.PhotoImage(file='img/black.png')
        self.color_white = tkinter.PhotoImage(file='img/white.png')
        self.color_null = tkinter.PhotoImage(file='img/null.png')
        self.Draw()

    def Draw(self):

        for i,panel in enumerate(self.board):

            bit = 2**(63 - i)

            if self.black & bit:
                color = self.color_black
            elif self.white & bit:
                color = self.color_white
            else:
                color = self.color_null

            panel.config(image=color)

    # 生成時に実行される
    def Put(self, i):

        def Put_():

            if self.mode.get() == 'player':
                
                pass
                # 関数を返す

            elif self.mode.get() == 'question':

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

        return Put_

class Control(tkinter.Frame):

    def __init__(self, master=None):

        super().__init__(master)

        self.start = tkinter.Button(self, text='探索開始')
        self.start.pack()

        self.seconds = tkinter.IntVar()
        self.seconds.set(3)
        self.seconds_label = tkinter.Label(self, text='{}秒'.format(self.seconds.get()**2))
        self.seconds_label.pack()
        def Scale(x):
            self.seconds_label.config(text='{}秒'.format(int(x)**2))
        self.seconds_scale = tkinter.Scale(
            self,
            orient='horizontal',
            variable=self.seconds,
            from_=1,
            to=32,
            command=Scale,
            showvalue=False
            )
        self.seconds_scale.pack()


    