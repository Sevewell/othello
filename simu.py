import sys
import subprocess
import threading

def stream_std_err(pipe):
    for line in iter(pipe.readline, ''):
        print(line.strip())

def Execute(stone_mine, stone_oppo, playout, store_mode):
    return subprocess.Popen(
        [
            './engine/target/release/engine',
            str(stone_mine),
            str(stone_oppo),
            playout,
            store_mode
        ],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding='utf-8',
        text=True
    )

def learn(playout, store_mode, iter):
    for i in range(iter):
        print(f"{i+1}回目：")
        black = 34628173824
        white = 68853694464
        process = Execute(black, white, playout, store_mode)
        thread_err = threading.Thread(target=stream_std_err, args=(process.stderr,))
        thread_err.start()
        process.wait()
        thread_err.join()

if __name__ == '__main__':
    learn(sys.argv[1], int(sys.argv[2]), sys.argv[3])
