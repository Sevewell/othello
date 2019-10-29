import search
import time

m = '0000000000000000000000000000100000010000000000000000000000000000'
y = '0000000000000000000000000001000000001000000000000000000000000000'
#node = search.Node(m + y)
#db = {node.key:node}
before = time.time()
db = search.LoadDB('db.pkl')
after = time.time()
print(after - before)
node = db[m + y]

move, info = search.Search(node, db, 5)
print(info)

children = [db[child] for child in db[node.key].children]
for child in children:
    move_ = (child.m | child.y) ^ (node.m | node.y)
    move_ = format(move_, '064b').index('1')
    record = child.record
    n = len(record)
    k = record.count('l')
    print('{}:{}/{}:{}'.format(move_, k, n, k/n))

#search.DumpDB(db)
