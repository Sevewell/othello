import pickle
from matplotlib import pyplot

with open('record.pkl', 'rb') as f:
    record = pickle.load(f)

print(len(record))

point = 'black'

if point == 'black':

    for winner in ['black', 'white', 'draw']:

        difference_trial = []
        difference_param = []
        difference_syner = []
        trial_m = []
        trial_y = []
        param_m = []
        if winner == 'black':
            result = 'win'
        elif winner == 'white':
            result = 'lose'
        else:
            result = 'draw'

        for row in record:
            if winner == row['winner']:
                difference_trial.append(row['n_b'] - row['n_w'])
                difference_param.append(row['hp_b'] - row['hp_w'])
                trial_m.append(row['n_b'])
                trial_y.append(row['n_w'])
                param_m.append(row['hp_b'])

        pyplot.scatter(trial_m, param_m, alpha=0.5, label=result)

pyplot.xlabel('trial')
pyplot.ylabel('param')
pyplot.legend()

pyplot.show()