import pickle
from matplotlib import pyplot

with open('record.pkl', 'rb') as f:
    record = pickle.load(f)

print(len(record))

for winner in ['black', 'white', 'draw']:

    hp_b = []
    hp_w = []

    for row in record:

        if row['winner'] == winner:
            hp_b.append(row['hp_b'])
            hp_w.append(row['hp_w'])

    pyplot.scatter(hp_b, hp_w, alpha=0.5, label=winner)

pyplot.xlabel('black')
pyplot.ylabel('white')
pyplot.legend()

pyplot.show()