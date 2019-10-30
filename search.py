import rule
import random
import time
import pickle

# このモジュール関数・メソッドにルールモジュールor関数を渡すような設計がいいな
# 汎用AIっぽいから

# 例えば並列処理しているときに本当に同じオブジェクトが更新されているのか

class Node():

    def __init__(self, m, y):

        self.m = m
        self.y = y
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
    def End(self):

        count_m = bin(self.m).count('1')
        count_y = bin(self.y).count('1')

        if count_m > count_y:
            state = 'w'
        elif count_m < count_y:
            state = 'l'
        elif count_m == count_y:
            state = 'd'

        return state

    def Count(self, n):

        for child in self.children:
            n = child.Count(n)
            n += 1
        
        return n

# 子は必ずいるときのみ呼ばれる
def ChoiceNode(children):

    probs = [Dice(child) for child in children]
    child_choiced = children[probs.index(max(probs))]

    return child_choiced

def Dice(node):

    x = node.record.count('l')
    n = len(node.record)

    a = 1 + x
    b = 1 + (n - x)

    uniforms = [random.random() for i in range(a+b-1)]
    uniforms.sort()

    return uniforms[a-1]

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

        state = node.End()

    if len(node.record) == node.memory:
        node.record.pop(0)
    node.record.append(state)
    
    return node

def Search(node, seconds):

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
    info['n_node'] = node.Count(0)

    child = ChoiceNode(node.children)
    move = (child.m | child.y) ^ (node.m | node.y)

    return move, info

def DumpDB(node):

    with open('node.pkl', mode='wb') as f:
        pickle.dump(node, f)

def LoadDB():

    with open('node.pkl', mode='rb') as f:
        node = pickle.load(f)

    return node
