import main
import json
import sys
import random
import time
import subprocess

random.seed(time.time_ns())

with open('config.json', 'r') as f:
    config = json.load(f)

def Execute(m, y):
    playout = str(config['playout'])
    seed = str(random.randint(1, 10000))
    learning_rate = str(config['learning_rate'])
    return subprocess.Popen(
        ['explorer.exe', m, y, playout, seed, learning_rate],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding='utf-8',
        text=True
    )

def Caluculate(moves):
    for move in moves:
        move['rate'] = [round(b / (a + b), 4) for a, b in zip(move['a'], move['b'])]
    # これが正しいかは議論の余地あり
    # こうするなら学習率は低めが良さそう
    return max(moves, key=lambda x: x['rate'][config['process'] // 2])

def Aggregate(processes):
    moves = processes.pop()
    for move in moves:
        move['a'] = [move['a']]
        move['b'] = [move['b']]
        move['node'] = [move['node']]
    for process in processes:
        for i,move in enumerate(process):
            moves[i]['a'].append(move['a'])
            moves[i]['b'].append(move['b'])
            moves[i]['node'].append(move['node'])
    #votes.sort(key=lambda x: x['count'], reverse=True)
    return moves

def Explore(stone_m, stone_y):
    processes = []
    seconds = 0
    for batch in range(config['batch']):
        processes_batch = [Execute(stone_m, stone_y) for p in range(config['process'])]
        while any([process.poll() == None for process in processes_batch]):
            time.sleep(1)
            seconds += 1
        def Convert(stdout):
            moves = eval(stdout)
            for move in moves:
                move['move'] = move['move'].zfill(64)
                move['m'] = move['m'].zfill(64)
                move['y'] = move['y'].zfill(64)
            return moves
        processes += [Convert(process.stdout.read()) for process in processes_batch]
    moves = Aggregate(processes)
    move = Caluculate(moves)
    for move in moves:
        print(move)
    print(seconds, move)

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
        rates = [0.95, 1.0]
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
