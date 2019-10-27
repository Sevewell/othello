import random

# このモジュール関数・メソッドにルールモジュールor関数を渡すような設計がいいな
# 汎用AIっぽいから

class Node():

    def __init__(self, key, db):

        self.key = key
        self.m = int(key[:32], 2)
        self.y = int(key[-32:], 2)
        self.record = []
        self.memory = 20
        self.children = []
        self.a = 1
        self.b = 1
        db[self.key] = self

    def FindChildren(self, db, rule):
        
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

    def Record(self, state):

        if len(self.record) == self.memory:
            self.record.pop(0)
        self.record.append(state)

def Children(node, db):

    children = [key_child for key_child in node.children]

    for child in children:

        # 実体にする
        if child in db:
            child = db[child]
        else:
            child = Node(child, db)

    return children

# 子は必ずいるときのみ呼ばれる
def ChoiceNode(children, db):

    probs = [Dice(child) for child in children]
    child_choiced = children[probs.index(max(probs))]

    return child_choiced

def Dice(node):

    x = node.record.count('w')
    n = len(node.record)

    a = node.a + x
    b = node.b + (n - x)

    uniforms = [random.random() for i in range(a+b-1)]
    uniforms.sort()

    return uniforms[a-1]

def Playout(node, db, rule):
    
    node.FindChildren(db, rule)

    if node.children:

        children = Children(node, db)
        child = ChoiceNode(children, db)
        state = Playout(child, db, rule) #これって参照渡しだよな？

    else:

        state = node.End()

    node.Record(state)

    return state

def Initialize(m, y):

    key_m = format(m, '064b')
    key_y = format(y, '064b')
    db['{}{}'.format(key_m, key_y)] = Node(m, y)

db = {}