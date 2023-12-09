import gui
import json
import random
import subprocess
import time

with open('config.json', 'r') as f:
    config = json.load(f)

def Engine(m, y, seed):
    return subprocess.Popen(
        ['othello.exe', m, y, str(config['playout']), seed, str(config['learning_rate'])],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding='utf-8',
        text=True
    )

def Aggregate(processes):
    votes = []
    for process in processes:
        vote = [vote for vote in votes if process['move'] == vote['move']]
        if vote:
            vote[0]['count'] += 1
            vote[0]['rate'].append(process['rate'])
            vote[0]['node'].append(process['node'])
        else:
            votes.append({
                'move': process['move'],
                'm': process['m'],
                'y': process['y'],
                'rate': [process['rate']],
                'node': [process['node']],
                'count': 1
            })
    votes.sort(key=lambda x: x['count'], reverse=True)
    return votes

def Explore(stone_m, stone_y):
    seeds = [str(random.randint(1, 1000)) for i in range(config['process'])]
    processes = [Engine(stone_m, stone_y, seed) for seed in seeds]
    seconds = 0
    while any([process.poll() == None for process in processes]):
        time.sleep(1)
        seconds += 1
    def Convert(stdout):
        result = eval(stdout)
        result['move'] = result['move'].zfill(64)
        result['m'] = result['m'].zfill(64)
        result['y'] = result['y'].zfill(64)
        return result
    processes = [Convert(process.stdout.read()) for process in processes]
    votes = Aggregate(processes)
    print(seconds, votes[0]['rate'])
    return votes[0]['move'], votes[0]['m'], votes[0]['y']

app = gui.App(gui.root, Explore)
app.mainloop()