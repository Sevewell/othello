import engine
import random
import time
import multiprocessing

engine.SetSeed(random.randint(0, 100))
hyper_param = 2

def WrapSearch(args):

    child = args[0]
    param = args[1]
    trial = args[2]

    return engine.Search(child['m'], child['y'], param, trial)

def Search(m, y, param, trial, cores):

    movable = engine.GetMovablePy(m, y)
    children = []
    while movable:
        move = movable ^ (movable & (movable - 1))
        reversable = engine.GetReversablePy(m, y, move)
        m_child = m | move | reversable
        y_child = y ^ reversable
        children.append({'m': y_child, 'y': m_child})
        movable ^= move

    trial_p = trial // len(children)
    time_before = time.time()

    # プレイアウト進捗とノード数を共有メモリしたい
    with multiprocessing.Pool(cores) as p:

        winrates = p.map(WrapSearch, [(child, param, trial_p) for child in children])

    choiced_winrate = min(winrates)
    choiced_child = children[winrates.index(choiced_winrate)]

    info = {
        'time': time.time() - time_before,
        'winrate': 1 - choiced_winrate
        }

    return choiced_child['y'], choiced_child['m'], info

def Move(m, y, move):

    movable = engine.GetMovablePy(m, y)

    if move & movable:

        reversable = engine.GetReversablePy(m, y, move)
        m |= move | reversable
        y ^= reversable

    return m, y

def CheckEnd(m, y):

    return engine.GetMovablePy(m, y) | engine.GetMovablePy(y, m)
