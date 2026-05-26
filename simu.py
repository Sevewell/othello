import main
import json
import sys
import random
import time
import subprocess
import threading

random.seed(time.time_ns())

with open('config.json', 'r') as f:
    config = json.load(f)

def Execute(stone_mine, stone_oppo):
    return subprocess.Popen(
        [
            './engine/target/release/engine',
            str(stone_mine),
            str(stone_oppo),
            str(5 * 10000),
            "update"
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding='utf-8',
        text=True
    )

def parse_result(process):
    process.wait()
    result = eval(process.stdout)
    if result["value"]["children"]:
        for child in result["value"]["children"]:
            child["win_rate"] = child["beta"] / (child["alpha"] + child["beta"])
            child["index"] = 63 - ((child["move"] & -child["move"]).bit_length() - 1)
        moves = result["value"]["children"]
    else: # 置ける石がなかった場合
        moves = []
    return moves

def choice_move(moves):
    if moves:
        return max(moves, key=lambda move: move["win_rate"])

def Play(player_black, player_white):

    black = 34628173824
    white = 68853694464
    pass_count = 0

    while pass_count < 2:
        player = player_black
        black, white, pass_count = Put(player, black, white, pass_count)
        player = player_white
        white, black, pass_count = Put(player, white, black, pass_count)
    
    if bin(black).count('1') > bin(white).count('1'):
        winner = 'black'
    elif bin(black).count('1') < bin(white).count('1'):
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

def learn(iter):
    for i in range(iter):
        print(f"{i+1}回目")
        black = 34628173824
        white = 68853694464
        process = Execute(black, white)
        process.wait()
        for line in process.stderr.readlines():
            print(line.strip())

if __name__ == '__main__':
    learn(10)
