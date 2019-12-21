import rule
import random
from collections import deque
import time
import pickle
import multiprocessing
import os

# このモジュール関数・メソッドにルールモジュールor関数を渡すような設計がいいな
# 汎用AIっぽいから

# 例えば並列処理しているときに本当に同じオブジェクトが更新されているのか

class Node():

    def __init__(self, m, y):

        self.m = m
        self.y = y
        self.SetPrior()
        self.children = []

    def SetPrior(self):

        bits = 65 - bin(self.m | self.y).count('1')
        self.a = bits ** 3
        self.b = bits ** 3

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

def Search(node, seconds, info):

    time_before = time.time()
    time_after = time.time()
    trial = 0

    while (time_after - time_before) < seconds:
        PlayOut(node)
        time_after = time.time()
        trial += 1

    info['playouts'] = trial
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
    seconds = args[1]
    info = {}

    Search(child, seconds, info)

    info['winrate'] = Dice(child.b, child.a)

    return child.y, child.m, info

def SearchMulti(m, y, seconds):

    cores = os.cpu_count()

    node = Node(m, y)
    node.FindChildren()

    lot = len(node.children) // cores
    if len(node.children) % cores:
        lot += 1
    seconds = seconds // lot

    with multiprocessing.Pool(cores) as p:

        result_children = p.map(WrapSearchMulti, [(child, seconds) for child in node.children])

    info = {
        'playouts': sum([result[2]['playouts'] for result in result_children]),
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

def DumpDB(node):

    with open('node.pkl', mode='wb') as f:
        pickle.dump(node, f)

def LoadDB():

    with open('node.pkl', mode='rb') as f:
        node = pickle.load(f)

    return node
