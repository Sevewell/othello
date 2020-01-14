import pickle
from matplotlib import pyplot

with open('record.pkl', 'rb') as f:
    record = pickle.load(f)

print(len(record))

for winner in ['black', 'white', 'draw']:

    hp_b = [row['hp_b'] for row in record if row['winner'] == winner]
    hp_w = [row['hp_w'] for row in record if row['winner'] == winner]

    pyplot.scatter(hp_b, hp_w, alpha=0.5, label=winner)

pyplot.title('win-lose about learning rate')
pyplot.xlabel('hp_b')
pyplot.ylabel('hp_w')
pyplot.legend()

pyplot.show()