import rule
import random
import multiprocessing

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

def PlayOut(node, db, record):
    
    node.FindChildren()

    if node.children:

        children = [db[key_child] if key_child in db else Node(key_child) for key_child in node.children]
        child = ChoiceNode(children)
        state, record = PlayOut(child, db, record) #これって参照渡しだよな？
        state = ReverseState(state)

    else:

        state = node.End()

    record.append(node.key)

    return state, record

def UpdateDB(state, record, db):

    for key in reversed(record):

        if key in db:
            node = db[key]
            if len(node.record) == node.memory:
                node.record.pop(0)
        else:
            node = Node(key)
            db[key] = node

        node.record.append(state)
        state = ReverseState(state)

def Search(node, db, n):

    for i in range(n):
        state, record = PlayOut(node, db, [])
        UpdateDB(state, record, db)

    children = [db[key_child] for key_child in node.children]
    child = ChoiceNode(children)

    move = (child.m | child.y) ^ (node.m | node.y)

    return move

def Multi(node, db, rule):

    with multiprocessing.Manager() as manager:

        db = manager.dict(db)
        p = multiprocessing.Process(target=PlayOut, args=(node, db, rule))
        p.start()
        p.join()

        print(db)
        print(node)

def Initialize(m, y):

    key_m = format(m, '064b')
    key_y = format(y, '064b')
    db['{}{}'.format(key_m, key_y)] = Node(m, y)
