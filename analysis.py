import json

def Parse(simu):
    if simu['winner'] == 'draw':
        return 'draw'
    return simu[simu['winner']]['learning_rate']

with open('simulation.json', 'r') as f:
    simulations = json.load(f)

simus = [Parse(simu) for simu in simulations]
print(simus.count(0.99))
print(simus.count(1.0))
print(simus.count('draw'))
