import rule
import engine
import random
import time
import pickle
import multiprocessing

hyper_param = 2

class Node():

    def __init__(self, m, y):

        self.m = m
        self.y = y
        bits = 65 - bin(self.m | self.y).count('1')
        self.a = bits ** hyper_param
        self.b = bits ** hyper_param
        self.children = []

    def FindChildren(self):
        
        if self.children:

            # 既に子がいれば何もしない
            pass

        else:

            movable = rule.GetMovable(self.m, self.y)

            if movable:

                while movable:
                    move = movable & (-movable)
                    reversable = rule.GetReversable(self.m, self.y, move)
                    m = self.m | move | reversable
                    y = self.y ^ reversable
                    self.children.append(Node(y, m))
                    movable ^= move

            else:
                
                if rule.GetMovable(self.y, self.m):
                    self.children.append(Node(self.y, self.m))

def PlayOut(node):
    
    node.FindChildren()

    if node.children:

        probs = [random.betavariate(child.a, child.b) for child in node.children]
        child = node.children[probs.index(min(probs))]
        state = PlayOut(child)
        if state == 'w':
            state = 'l'
        elif state == 'l':
            state = 'w'

    else:

        count_m = bin(node.m).count('1')
        count_y = bin(node.y).count('1')
        if count_m > count_y:
            state = 'w'
        elif count_m < count_y:
            state = 'l'
        else:
            state = 'd'

    if state == 'w':
        node.a += 1
    elif state == 'l':
        node.b += 1
    else:
        node.a += 0.5
        node.b += 0.5
    
    return state

def SearchC(m, y, param, trial, core):

    winrate = engine.Search(m, y, 1.0, trial)
    print(winrate)

    return m, y

def WrapSearch(args):

    child = args[0]
    param = args[1]
    trial = args[2]

    '''
    playouts = 0
    while playouts < trial:
        PlayOut(child)
        playouts += 1
    return random.betavariate(child.a, child.b)
    '''

    return engine.Search(child.m, child.y, param, trial)

def Search(m, y, param, trial, cores):

    node = Node(m, y)
    node.FindChildren()

    trial = trial // len(node.children)
    time_before = time.time()

    # プレイアウト進捗とノード数を共有メモリしたい
    with multiprocessing.Pool(cores) as p:

        winrates = p.map(WrapSearch, [(child, param, trial) for child in node.children])

    choiced_winrate = min(winrates)
    choiced_child = node.children[winrates.index(choiced_winrate)]

    print({
        'time': time.time() - time_before,
        'winrate': 1 - choiced_winrate
    })

    return choiced_child.y, choiced_child.m

def Move(m, y, move):

    movable = rule.GetMovable(m, y)

    if move & movable:

        reversable = rule.GetReversable(m, y, move)
        m |= move | reversable
        y ^= reversable

    return m, y

def Count(node, n):

    n += 1
    for child in node.children:
        n = Count(child, n)
    
    return n

def CheckEnd(m, y):

    return rule.GetMovable(m, y) | rule.GetMovable(y, m)
