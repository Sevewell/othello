import gui
import subprocess
import time

def Engine(m, y, seed):
    return subprocess.Popen(
        ['othello.exe', m, y, seed, '1.0'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        encoding='utf-8',
        text=True
    )

def Explore(black, white, turn):
    if turn == 'black':
        m = black
        y = white
    if turn == 'white':
        m = white
        y = black
    seeds = ['101', '102', '103', '104']
    processes = [Engine(m, y, seed) for seed in seeds]
    while any([process.poll() == None for process in processes]):
        time.sleep(1)
    def Convert(stdout):
        result = eval(stdout)
        result['m'] = result['m'].zfill(64)
        result['y'] = result['y'].zfill(64)
        return result
    votes = [Convert(process.stdout.read()) for process in processes]
    print(votes)

app = gui.App(gui.root, Explore)
app.mainloop()