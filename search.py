import random
import time
import multiprocessing
import subprocess

def ConvertStdoutToDict(stdout):

    args = stdout.split(', ')

    return {
        'm': int(args[0]),
        'y': int(args[1]),
        'pass': float(args[2]),
        'rate': float(args[3])
    }

def ConvertMove(child, m, y):

    now = m | y
    new = child['m'] | child['y']
    move = format(now ^ new, '064b').find('1')
    column = 'ABCDEFGH'[move % 8]
    row = move // 8 + 1
    return column + str(row)

def PackChild(children, m, y):

    children_packing = []

    for child in children:
        moves = [(child_['m'], child_['y']) for child_ in children_packing]
        if not (child['m'], child['y']) in moves:
            children_packing.append({
                'move': ConvertMove(child, m, y),
                'm': child['m'],
                'y': child['y'],
                'pass': 0,
                'rate': []
                })

    for child in children:
        for child_ in children_packing:
            if (child['m'] == child_['m']) and (child['y'] == child_['y']):
                child_['pass'] += child['pass']
                child_['rate'].append(child['rate'])

    return children_packing

def Search(m, y, param, trial, cores, seed):

    time_before = time.time()

    stdin = [
        "./search",
        str(m),
        str(y),
        str(trial),
        str(param),
        str(cores),
        str(seed)
    ]
    cp = subprocess.run(stdin, capture_output=True)

    print(time.time() - time_before , '秒')

    stdout = cp.stdout.decode('utf-8').strip()

    if stdout == '':

        return []

    else:

        children = [ConvertStdoutToDict(child) for child in stdout.split('\n')]
        children = PackChild(children, m, y)

        return children
