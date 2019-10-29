import rule
import random
import time
import pickle

# このモジュール関数・メソッドにルールモジュールor関数を渡すような設計がいいな
# 汎用AIっぽいから

# 例えば並列処理しているときに本当に同じオブジェクトが更新されているのか

class Node():

    def __init__(self, key):

        self.key = key
        self.m = int(key[:64], 2)
        self.y = int(key[-64:], 2)
        self.record = []
        self.memory = 100
        self.children = []
        self.a = 1
        self.b = 1

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
                    key_child = format(y, '064b') + format(m, '064b')
                    self.children.append(key_child)
                    movable ^= move

            else:
                
                if rule.GetMovable(self.y, self.m):
                    key_child = format(self.y, '064b') + format(self.m, '064b')
                    self.children.append(key_child)

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

# 子は必ずいるときのみ呼ばれる
def ChoiceNode(children):

    probs = [Dice(child) for child in children]
    child_choiced = children[probs.index(max(probs))]

    return child_choiced

def Dice(node):

    x = node.record.count('l')
    n = len(node.record)

    a = node.a + x
    b = node.b + (n - x)

    uniforms = [random.random() for i in range(a+b-1)]
    uniforms.sort()

    return uniforms[a-1]

def ReverseState(state):

    if state == 'w':
        state = 'l'
    elif state == 'l':
        state = 'w'

    return state

def PlayOut(node, db, path):
    
    node.FindChildren()

    if node.children:

        children = [db[key] if key in db else Node(key) for key in node.children]
        child = ChoiceNode(children)
        path = PlayOut(child, db, path)
        state = ReverseState(path[-1].record[-1])

    else:

        state = node.End()

    if len(node.record) == node.memory:
        node.record.pop(0)
    node.record.append(state)
    
    path.append(node)

    return path

def Search(node, db, seconds):

    time_before = time.time()
    time_after = time.time()
    info = {
        'n_playout': 0,
        'n_node': 0
    }

    while (time_after - time_before) < seconds:

        path = PlayOut(node, db, [])
        for leaf in path:
            db[leaf.key] = leaf
        info['n_playout'] += 1
        time_after = time.time()

    info['n_node'] = len(db)

    children = [db[key] for key in db[node.key].children]
    child = ChoiceNode(children)

    move = (child.m | child.y) ^ (node.m | node.y)

    return move, info

def DumpDB(db):

    with open('db.pkl', mode='wb') as f:
        pickle.dump(db, f)

def LoadDB(name):

    with open('db.pkl', mode='rb') as f:
        db = pickle.load(f)

    return db
