import tkinter
import search

def Draw():

    for i in range(64):

        if black[i] == '1':
            board[i].config(image=color_black)
        elif white[i] == '1':
            board[i].config(image=color_white)
        else:
            board[i].config(image=color_null)

def Put(i):

    # 既に石があったら空にしたいな

    def Put_():

        global black
        global white
        global turn

        move = 2**(63 - i)

        if move & (int(black, 2) | int(white, 2)):
            
            black_ = int(black, 2) & (~move)
            black = format(black_, '064b')
            white_ = int(white, 2) & (~move)
            white = format(white_, '064b')
        
        else:

            if turn.get() == 'b':
                black_ = int(black, 2) | move
                black = format(black_, '064b')
            elif turn.get() == 'w':
                white_ = int(white, 2) | move
                white = format(white_, '064b')

        Draw()

    return Put_

def Search():

    global black
    global white

    if turn.get() == 'b':
        m = black
        y = white
    elif turn.get() == 'w':
        m = white
        y = black

    node = search.Node(int(m, 2), int(y, 2))
    node, info = search.Search(node)
    print(info)
    for i in range(64):
        board[i].config(bg='#FFFFFF')
    for child in node.children:
        move = (child.m | child.y) ^ (node.m | node.y)
        move = format(move, '064b').index('1')
        n = len(child.record)
        k = child.record.count('l')
        rgb = format(255-255*k//n, 'x')*3
        board[move].config(bg='#{}'.format(rgb))

    '''
    child = search.ChoiceNode(node.children)
    if turn.get() == 'b':
        black = format(child.y, '064b')
        white = format(child.m, '064b')
    elif turn.get() == 'w':
        white = format(child.y, '064b')
        black = format(child.m, '064b')
    '''

    Draw()

def Scale(x):
    search.seconds = seconds_var.get()**2
    label_seconds.config(text='{}秒'.format(search.seconds))

if __name__ == '__main__':

    root = tkinter.Tk()
    root.title('詰めオセロ解くやつ')

    menu = tkinter.Menu(root)
    menu_file = tkinter.Menu(menu)
    menu.add_cascade(menu=menu_file, label='モード')

    frame = {
        'board': tkinter.Frame(root),
        'control': tkinter.Frame(root)
    }
    frame['board'].pack()
    frame['control'].pack()

    color_null = tkinter.PhotoImage(file='img/null.png')
    color_black = tkinter.PhotoImage(file='img/black.png')
    color_white = tkinter.PhotoImage(file='img/white.png')
    color_movable = tkinter.PhotoImage(file='img/movable.png')

    # 初期位置
    black = '0000000000000000000000000000100000010000000000000000000000000000'
    white = '0000000000000000000000000001000000001000000000000000000000000000'

    # 先手番がTrue
    turn = 'b'

    board = [tkinter.Button(frame['board'], command=Put(i)) for i in range(64)]
    for i,panel in enumerate(board):
        panel.grid(column=i%8, row=i//8)

    turn = tkinter.StringVar()
    turn.set('b')
    turn_black = tkinter.Radiobutton(frame['control'], text='黒', value='b', variable=turn)
    turn_black.pack()
    turn_white = tkinter.Radiobutton(frame['control'], text='白', value='w', variable=turn)
    turn_white.pack()

    start = tkinter.Button(frame['control'], command=Search, text='探索開始')
    start.pack()

    # 設定で探索時間を指定したい
    seconds_var = tkinter.IntVar()
    seconds_var.set(3)
    search.seconds = seconds_var.get()
    label_seconds = tkinter.Label(frame['control'], text='{}秒'.format(seconds_var.get()**2))
    label_seconds.pack()
    scale_seconds = tkinter.Scale(
        frame['control'],
        orient='horizontal',
        variable=seconds_var,
        from_=1,
        to=32,
        command=Scale,
        showvalue=False
        )
    scale_seconds.pack()

    Draw()

    root.mainloop()