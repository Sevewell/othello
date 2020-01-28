m = '0000000000000000000100000011100000011000000010000000000000000000'
y = '0000000000000000001000000000000000100000000100000000100000000000'
move = format(67108864, '064b')

for i in range(0, 64, 8):
    print(move[i:i+7])

print()

for i in range(0, 64, 8):
    print(m[i:i+8])

print()

for i in range(0, 64, 8):
    print(y[i:i+8])

print()

print(int(m, 2))
print(int(y, 2))
