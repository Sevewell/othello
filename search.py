import random
import time
import multiprocessing
import subprocess

def ConvertStdoutToDict(stdout):

    args = stdout.split(', ')

    return {
        'm': int(args[0]),
        'y': int(args[1]),
        'a': float(args[2]),
        'b': float(args[3])
    }

def ConvertMoveToPoint(child, m, y):

    now = m | y
    new = child['m'] | child['y']
    move = format(now ^ new, '064b').find('1')
    column = 'ABCDEFGH'[move % 8]
    row = move // 8 + 1
    child['move'] = column + str(row)
    return child

def SumParameter(move, children):

    sum = 0
    m = 0
    y = 0
    for child in children:
        if child['move'] == move:
            sum += child['a'] + child['b']
            m = child['m']
            y = child['y']
    return {'move': move, 'm': m, 'y': y, 'param': sum}

def AggregateLearning(children):

    moves = set([child['move'] for child in children])
    moves = [SumParameter(move, children) for move in moves]
    return moves

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

    #print(time.time() - time_before , '秒')
    stdout = cp.stdout.decode('utf-8').strip()

    if stdout == '':

        return []

    else:

        children = [ConvertStdoutToDict(child) for child in stdout.split('\n')]
        children = [ConvertMoveToPoint(child, m, y) for child in children]
        #for child in children:
        #    print(child)
        moves = AggregateLearning(children)

        return moves

def CheckEnd(m, y):

    return 1
