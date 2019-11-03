import rule
import random
from collections import deque
import time
import pickle
import multiprocessing

# このモジュール関数・メソッドにルールモジュールor関数を渡すような設計がいいな
# 汎用AIっぽいから

# 例えば並列処理しているときに本当に同じオブジェクトが更新されているのか

seconds = 10

class Node():

    def __init__(self, m, y):

        self.m = m
        self.y = y
        # メモリ消費が激しかったので不採用
        #self.record = deque(maxlen=100)
        self.record = []
        self.memory = 100
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
def ChoiceNode(children):

    probs = [Dice(child) for child in children]
    child_choiced = children[probs.index(min(probs))]

    return child_choiced

def Dice(node):

    n = len(node.record)
    k = node.record.count('w')

    alpha = 1 + k
    beta = 1 + (n - k)

    return random.betavariate(alpha, beta)

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
        child = PlayOut(child)
        state = ReverseState(child.record[-1])

    else:

        state = End(node.m, node.y)

    if len(node.record) == node.memory:
        node.record.pop(0)
    node.record.append(state)
    
    return node

def Search(node):

    time_before = time.time()
    time_after = time.time()
    info = {
        'n_playout': 0,
        'n_node': 0
    }

    while (time_after - time_before) < seconds:
        node = PlayOut(node)
        info['n_playout'] += 1
        time_after = time.time()

    # ノードを数える
    info['n_node'] = Count(node, 0)

    return node, info

def SearchMulti(node):

    with multiprocessing.Pool() as p:
        node.children = p.map(Search, node.children)
    
    return node

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
