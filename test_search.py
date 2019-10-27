import unittest
import rule
import search

class TestNode(unittest.TestCase):

    def test_FindChildrenFirst(self):

        m = 0b0100000010000000000000000000000000000
        y = 0b1000000001000000000000000000000000000
        db = {}
        node = search.Node(m, y, db)
        node.FindChildren(db, rule)
        correct_key_m = '0000000000000000000000000001000000000000000000000000000000000000'
        correct_key_y = '0000000000000000000000000000100000011000000010000000000000000000'
        self.assertEqual(node.children[0], correct_key_m+correct_key_y)
        print(db)

    def test_FindChildrenLast(self):

        m = 0b0100000010000000000000000000000000000
        y = 0b1000000001000000000000000000000000000
        db = {}
        node = search.Node(m, y, db)
        node.FindChildren(db, rule)
        correct_key_m = '0000000000000000000000000000000000001000000000000000000000000000'
        correct_key_y = '0000000000000000000100000001100000010000000000000000000000000000'
        print(db)
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

    def test_Playout

if __name__ == '__main__':

    unittest.main()
