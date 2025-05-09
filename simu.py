import main
import json
import sys
import random
import time

def Put(player, m, y, pass_count):
    main.config['playout'] = player['playout']
    main.config['process'] = player['process']
    main.config['batch'] = player['batch']
    main.config['learning_rate'] = player['learning_rate']
    move, m_, y_ = main.Explore(m, y)
    if m == m_ and y == y_:
        pass_count += 1
    else:
        pass_count = 0
    return m_, y_, pass_count

def Play(player_black, player_white):

    black = '0000000000000000000000000000100000010000000000000000000000000000'
    white = '0000000000000000000000000001000000001000000000000000000000000000'
    pass_count = 0

    while pass_count < 2:
        player = player_black
        black, white, pass_count = Put(player, black, white, pass_count)
        player = player_white
        white, black, pass_count = Put(player, white, black, pass_count)
    
    if black.count('1') > white.count('1'):
        winner = 'black'
    elif black.count('1') < white.count('1'):
        winner = 'white'
    else:
        winner = 'draw'

    with open('simulation.json', 'r') as f:
        record = json.load(f)
    record.append({
        'black': player_black,
        'white': player_white,
        'winner': winner
    })
    with open('simulation.json', 'w') as f:
        json.dump(record, f, indent='\t')        

if __name__ == '__main__':
    random.seed(time.time_ns())
    for i in range(int(sys.argv[1])):
        rates = [0.99, 1.0]
        random.shuffle(rates)
        player_1 = {
            'playout': 1000000,
            'process': 8,
            'batch': 1,
            'learning_rate': rates[0]
        }
        player_2 = {
            'playout': 1000000,
            'process': 8,
            'batch': 1,
            'learning_rate': rates[1]
        }
        print('black', player_1)
        print('white', player_2)
        Play(player_1, player_2)
