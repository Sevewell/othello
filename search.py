import rule
import random

class Node():

    def __init__(self, m, y):

        self.m = m
        self.y = y
        self.record = []
        self.memory = 20
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
                    key_m = format(m, '064b')
                    key_y = format(y, '064b')
                    self.children.append('{}{}'.format(key_y, key_m))
                    movable ^= move

            else:
                
                if rule.GetMovable(self.y, self.m):
                    key_m = format(m, '064b')
                    key_y = format(y, '064b')
                    self.children.append('{}{}'.format(key_y, key_m))

    # FindChildrenしても空だったら
    def End(self):

        if len(self.record) == self.memory:
            self.record.pop(0)
        
        count_m = bin(self.m).count('1')
        count_y = bin(self.y).count('1')

        if count_m > count_y:
            state = 'w'
        elif count_m < count_y:
            state = 'l'
        elif count_m == count_y:
            state = 'd'

        self.record.append(state)

def ChoiceNode(node):

    probs = [Dice(child) for child in node.children] #ここの関係で関数をメソッド化するか悩みどころ
    key_choiced = node.children[probs.index(max(probs))]
    # DBになければ登録もする
    if key_choiced in db:
        child = db[key_choiced]
    else:
        child = Node(int(key_choiced[:64], 2), int(key_choiced[-64:], 2))
        db[key_choiced] = child

    return child

def Dice(child_key, tree, a, b):

    child = tree[child_key]
    count_w = child.record.count('w')
    a = a + count_w
    b = b + (len(child.record) - count_w)

    uniforms = [random.random() for i in range(a+b-1)]
    uniforms.sort()

    return uniforms[a-1]

def Initialize(m, y):

    key_m = format(m, '064b')
    key_y = format(y, '064b')
    db['{}{}'.format(key_m, key_y)] = Node(m, y)

def Playout(node):
    
    node.FindChildren()

    if node.children:

        child = ChoiceNode(node)
        state = Playout(child) #これって参照渡しだよな？

    else:

        node.End()
        state = node.record[-1]

    return state

def GenerateKey(m, y):
    
    return '{}{}'.format(format(m, '064b'), format(y, '064b'))

db = {}