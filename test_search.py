import unittest

import unittest
import search

class TestNode(unittest.TestCase):

    def test_FindChildrenFirst(self):

        m = 0b0100000010000000000000000000000000000
        y = 0b1000000001000000000000000000000000000
        node = search.Node(m, y)
        node.FindChildren()
        correct_key_m = '0000000000000000000000000001000000000000000000000000000000000000'
        correct_key_y = '0000000000000000000000000000100000011000000010000000000000000000'
        self.assertEqual(node.children[0], correct_key_m+correct_key_y)

    def test_FindChildrenLast(self):

        m = 0b0100000010000000000000000000000000000
        y = 0b1000000001000000000000000000000000000
        node = search.Node(m, y)
        node.FindChildren()
        correct_key_m = '0000000000000000000000000000000000001000000000000000000000000000'
        correct_key_y = '0000000000000000000100000001100000010000000000000000000000000000'
        self.assertEqual(node.children[-1], correct_key_m+correct_key_y)

if __name__ == '__main__':

    unittest.main()
