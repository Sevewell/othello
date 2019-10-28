import unittest
import rule
import search
import math

class TestNode(unittest.TestCase):

    def test_FindChildrenFirst(self):

        m = '0000000000000000000000000000100000010000000000000000000000000000'
        y = '0000000000000000000000000001000000001000000000000000000000000000'
        db = {}
        node = search.Node(m + y, db)
        node.FindChildren(db, rule)
        correct_key_m = '0000000000000000000000000001000000000000000000000000000000000000'
        correct_key_y = '0000000000000000000000000000100000011000000010000000000000000000'
        self.assertEqual(node.children[0], correct_key_m+correct_key_y)

    def test_FindChildrenLast(self):

        m = '0000000000000000000000000000100000010000000000000000000000000000'
        y = '0000000000000000000000000001000000001000000000000000000000000000'
        db = {}
        node = search.Node(m + y, db)
        node.FindChildren(db, rule)
        correct_key_m = '0000000000000000000000000000000000001000000000000000000000000000'
        correct_key_y = '0000000000000000000100000001100000010000000000000000000000000000'
        self.assertEqual(node.children[-1], correct_key_m+correct_key_y)

    '''
    def test_ChoiceNode(self):

        m = 0b0100000010000000000000000000000000000
        y = 0b1000000001000000000000000000000000000
        db = {}
        node = search.Node(m, y, db)
        node.FindChildren(db, rule)
        child = search.ChoiceNode(node.children, db)

        correct_key_m_1 = '0000000000000000000000000001000000000000000000000000000000000000'
        correct_key_y_1 = '0000000000000000000000000000100000011000000010000000000000000000'
        correct_key_m_8 = '0000000000000000000000000000000000001000000000000000000000000000'
        correct_key_y_8 = '0000000000000000000100000001100000010000000000000000000000000000'
        print(db)
        self.assertEqual(node.children[-1], correct_key_m+correct_key_y)
    '''

    def test_PlayOut(self):

        m = '0000000000000000000000000000100000010000000000000000000000000000'
        y = '0000000000000000000000000001000000001000000000000000000000000000'
        db = {}
        node = search.Node(m + y, db)
        state = search.PlayOut(node, db, rule)
        self.assertEqual(len(node.record), 1)

    def test_Search(self):

        #m = '0000000000000000110100011110101110001111110000110000000000000000'
        #y = '0111110000111100001011100001010001110000001111000011100000010000'
        #correct = 0b0000000000000001000000000000000000000000000000000000000000000000
        m = '0111111000111100110010001100010011111000110000001000000000000000'
        y = '0000000000000000001101110011101100000110001110100000100000000000'
        correct = "0000000001000000000000000000000000000000000000000000000000000000"
        db = {}
        node = search.Node(m + y, db)
        move = search.Search(node, db, rule, 10000)
        print()
        move = format(move, '064b').index('1')
        correct = correct.index('1')

        children = [db[child] for child in node.children]
        for child in children:
            move_ = (child.m | child.y) ^ (node.m | node.y)
            move_ = format(move_, '064b').index('1')
            record = child.record
            n = len(record)
            k = record.count('l')
            print('{}:{}/{}:{}'.format(move_, k, n, k/n))
        
        self.assertEqual(move, correct)

if __name__ == '__main__':

    unittest.main()
