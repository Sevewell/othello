import tkinter
import rule

root = tkinter.Tk()
root.title('オセロ')

frame = {
    'board': tkinter.Frame(root),
    'opponent': tkinter.Frame(root),
    'player': tkinter.Frame(root),
    'message': tkinter.Frame(root)
}
frame['board'].grid(column=0, rowspan=2)
frame['opponent'].grid(column=1, row=0)
frame['player'].grid(column=1, row=1)
frame['message'].grid(columnspan=2, row=2)

color_null = tkinter.PhotoImage(file='img/null.png')
color_black = tkinter.PhotoImage(file='img/black.png')
color_white = tkinter.PhotoImage(file='img/white.png')
color_movable = tkinter.PhotoImage(file='img/movable.png')

# 初期位置
black = 0b0100000010000000000000000000000000000
white = 0b1000000001000000000000000000000000000

# 先手番がTrue
turn = 'b'

def Draw():

    if turn == 'b':
        text = '黒番'
        movable = rule.GetMovable(black, white)
    elif turn == 'w':
        text = '白番'
        movable = rule.GetMovable(white, black)
    elif turn == 'e':
        text = '終局'
        movable = 0

    for i in range(64):
        if 2**i & black:
            board[i].config(image=color_black)
        elif 2**i & white:
            board[i].config(image=color_white)
        elif 2**i & movable:
            board[i].config(image=color_movable)
        else:
            board[i].config(image=color_null)

    count_black = bin(black).count('1')
    count_white = bin(white).count('1')

    message.config(text='{}'.format(text))
    opponent_info.config(text='相手：白：{}個'.format(count_white))
    player_info.config(text='あなた：黒：{}個'.format(count_black))

def Put(i):

    def Put_():

        global black
        global white
        global turn

        move = 2**i

        turn, black, white = rule.Put(turn, black, white, move)

        Draw()

    return Put_

board = [tkinter.Button(frame['board'], image=color_null, command=Put(i)) for i in range(64)]
for i,panel in enumerate(board):
    panel.grid(column=i%8, row=i//8)

#opponent_pic = tkinter.Label(frame['opponent'], image=tkinter.PhotoImage(file='robot.png'))
#opponent_pic.pack(fill='both')
opponent_info = tkinter.Label(frame['opponent'])
opponent_info.pack()

#player_pic = tkinter.Label(frame['player'], image=tkinter.PhotoImage(file='player.png'))
#player_pic.pack(fill='both')
player_info = tkinter.Label(frame['player'])
player_info.pack()

message = tkinter.Label(frame['message'])
message.pack()

Draw()

root.mainloop()