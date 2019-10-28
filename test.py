import rule
import search

m = '0000000000000000000000000000100000010000000000000000000000000000'
y = '0000000000000000000000000001000000001000000000000000000000000000'
db = {}

node = search.Node(m + y, db)
node.record = ['w']*100 + ['l']*0

for i in range(100):
    print(search.Dice(node))