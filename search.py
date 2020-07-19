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
        'b': float(args[3]),
        'winrate': float(args[4])
    }

def Search(m, y, param, trial, cores, seed):

    time_before = time.time()

    stdin = [
        "./search",
        str(m),
        str(y),
        str(trial),
        str(param),
        str(seed)
    ]
    cp = subprocess.run(stdin, capture_output=True)

    #print(time.time() - time_before , '秒')
    stdout = cp.stdout.decode('utf-8').strip()

    if stdout == '':

        return []

    else:

        children = [ConvertStdoutToDict(child) for child in stdout.split('\n')]
        return children

def CheckEnd(m, y):

    return 1
