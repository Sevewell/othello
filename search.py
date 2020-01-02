import rule
import random
from collections import deque
import time
import pickle
import multiprocessing

hyper_param = 1

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

def Search(node, trial, info):

    playouts = 0

    while playouts < trial:
        PlayOut(node)
        time_after = time.time()
        playouts += 1

    info['nodes'] = Count(node, 0)

def SearchSingle(m, y, seconds):

    node = Node(m, y)
    info = {}
    Search(node, seconds, info)

    child = ChoiceNode(node.children)

    info['winrate'] = int(Dice(child.b, child.a) * 100)

    return child.y, child.m, info

def WrapSearchMulti(args):

    child = args[0]
    trial = args[1]
    info = {}

    Search(child, trial, info)

    info['winrate'] = Dice(child.b, child.a)

    return child.y, child.m, info

def SearchMulti(m, y, trial, cores):

    node = Node(m, y)
    node.FindChildren()

    trial = trial // cores
    time_before = time.time()

    with multiprocessing.Pool(cores) as p:

        result_children = p.map(WrapSearchMulti, [(child, trial) for child in node.children])

    info = {
        'time': time.time() - time_before,
        'nodes': sum([result[2]['nodes'] for result in result_children])
    }

    def Key(result):
        return result[2]['winrate']
    result_child = max(result_children, key=Key)

    info['winrate'] = result_child[2]['winrate']

    return result_child[0], result_child[1], info

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
