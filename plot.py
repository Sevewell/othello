import pickle
from matplotlib import pyplot

with open('record.pkl', 'rb') as f:
    record = pickle.load(f)

print(len(record))

for winner in ['black', 'white', 'draw']:

    param_b = [row['lr_b'] for row in record if row['winner'] == winner]
    param_w = [row['lr_w'] for row in record if row['winner'] == winner]

    pyplot.scatter(param_b, param_w, alpha=0.5, label=winner)

pyplot.title('win-lose about learning rate')
pyplot.xlabel('lr_b')
pyplot.ylabel('lr_w')
pyplot.legend()

pyplot.show()