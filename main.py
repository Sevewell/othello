import search
import widget

def Search(menu, board, node, seconds):

    def Search_():

        if menu.turn.get() == 'b':
            m = board.black
            y = board.white
        elif menu.turn.get() == 'w':
            m = board.white
            y = board.black

        result = search.Search({'node':node, 'seconds':seconds.get()**2})
        node = result['node']
        info = result['info']
        for child in node.children:
            print(child.record)
        print(info)
        for panel in board.board:
            panel.config(bg='#FFFFFF')
        for child in node.children:
            move = (child.m | child.y) ^ (node.m | node.y)
            move = format(move, '064b').index('1')
            n = len(child.record)
            k = child.record.count('l')
            rgb = format(255-255*k//n, 'x')*3
            board.board[move].config(bg='#{}'.format(rgb))

    return Search_
    '''
    child = search.ChoiceNode(node.children)
    if turn.get() == 'b':
        black = format(child.y, '064b')
        white = format(child.m, '064b')
    elif turn.get() == 'w':
        white = format(child.y, '064b')
        black = format(child.m, '064b')
    '''

if __name__ == '__main__':

    # もう始めにモード選択しよ…
    start = widget.Start()
    start.mainloop()

'''
    root = widget.Root()

    menu = widget.Menu(root)
    root['menu'] = menu

    node = search.Node(34628173824, 68853694464)

    board = widget.Board(master=root, mode=menu.mode, turn=menu.turn, node=node)
    
    control = widget.Control(master=root)
    control.start.config(command=Search(menu, board, node, control.seconds))

    board.pack()
    control.pack()

    root.mainloop()
'''