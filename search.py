import rule
import random
import time
import pickle
import multiprocessing

hyper_param = 2

class Node():

    def __init__(self, m, y):

        self.m = m
        self.y = y
        self.SetPrior()
        self.children = []

    def SetPrior(self):

        bits = 65 - bin(self.m | self.y).count('1')
        self.a = bits ** hyper_param
        self.b = bits ** hyper_param

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
                    m, y = rule.Reverse(self.m, self.y, move, reversable)
                    self.children.append(Node(y, m))
                    movable ^= move

            else:
                
                if rule.GetMovable(self.y, self.m):
                    self.children.append(Node(self.y, self.m))

# FindChildrenしても空だったら
def End(m, y):

    count_m = bin(m).count('1')
    count_y = bin(y).count('1')

    if count_m > count_y:
        state = 'w'
    elif count_m < count_y:
        state = 'l'
    elif count_m == count_y:
        state = 'd'

    return state

# 子は必ずいるときのみ呼ばれる
# 子が終局のときは確率出力するのおかしくない？
def ChoiceNode(children):

    probs = [Dice(child.a, child.b) for child in children]
    child_choiced = children[probs.index(min(probs))]

    return child_choiced

def Dice(a, b):

    return random.betavariate(a, b)

def ReverseState(state):

    if state == 'w':
        state = 'l'
    elif state == 'l':
        state = 'w'

    return state

def PlayOut(node):
    
    node.FindChildren()

    if node.children:
        child = ChoiceNode(node.children)
        state = PlayOut(child)
        state = ReverseState(state)
    else:
        state = End(node.m, node.y)

    if state == 'w':
        node.a += 1
    elif state == 'l':
        node.b += 1
    
    return state

def WrapSearchMulti(args):

    child = args[0]
    trial = args[1]

    playouts = 0
    while playouts < trial:
        PlayOut(child)
        playouts += 1

    return Dice(child.a, child.b)

def SearchMulti(m, y, trial, cores):

    node = Node(m, y)
    node.FindChildren()

    trial = trial // len(node.children)
    time_before = time.time()

    # プレイアウト進捗とノード数を共有メモリしたい
    with multiprocessing.Pool(cores) as p:

        winrates = p.map(WrapSearchMulti, [(child, trial) for child in node.children])

    choiced_winrate = min(winrates)
    choiced_child = node.children[winrates.index(choiced_winrate)]

    print('time: {}'.format(time.time() - time_before))
    print('winrate: {}'.format(1 - choiced_winrate))
    print(choiced_child.y, choiced_child.m)
    print('...')

    return choiced_child.y, choiced_child.m

def Move(m, y, move):

    movable = rule.GetMovable(m, y)

    if move & movable:

        reversable = rule.GetReversable(m, y, move)
        m, y = rule.Reverse(m, y, move, reversable)

    return m, y

def Count(node, n):

    n += 1
    for child in node.children:
        n = Count(child, n)
    
    return n

def CheckEnd(m, y):

    return rule.GetMovable(m, y) | rule.GetMovable(y, m)
