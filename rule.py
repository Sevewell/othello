def MovableLeftShift(player, masked, blank, dir):

    tmp = masked & (player << dir)
    tmp |= masked & (tmp << dir)
    tmp |= masked & (tmp << dir)
    tmp |= masked & (tmp << dir)
    tmp |= masked & (tmp << dir)
    tmp |= masked & (tmp << dir)
    legal = blank & (tmp << dir)

    return legal

def MovableRightShift(player, masked, blank, dir):

    tmp = masked & (player >> dir)
    tmp |= masked & (tmp >> dir)
    tmp |= masked & (tmp >> dir)
    tmp |= masked & (tmp >> dir)
    tmp |= masked & (tmp >> dir)
    tmp |= masked & (tmp >> dir)
    legal = blank & (tmp >> dir)

    return legal

def GetMovable(m, y):

    blank = ~(m | y)
    h = y & 0x7e7e7e7e7e7e7e7e
    v = y & 0x00ffffffffffff00
    a = y & 0x007e7e7e7e7e7e00
    legal = MovableLeftShift(m, h, blank, 1)
    legal |= MovableLeftShift(m, v, blank, 8)
    legal |= MovableLeftShift(m, a, blank, 7)
    legal |= MovableLeftShift(m, a, blank, 9)
    legal |= MovableRightShift(m, h, blank, 1)
    legal |= MovableRightShift(m, v, blank, 8)
    legal |= MovableRightShift(m, a, blank, 7)
    legal |= MovableRightShift(m, a, blank, 9)

    return legal

def _reversed_l(player, blank_masked, site, dir):
    """Direction << for reversed()."""
    rev = 0
    tmp = ~(player | blank_masked) & (site << dir)
    if tmp:
        for i in range(6):
            tmp <<= dir
            if tmp & blank_masked:
                break
            elif tmp & player:
                rev |= tmp >> dir
                break
            else:
                tmp |= tmp >> dir
    return rev

def _reversed_r(player, blank_masked, site, dir):
    """Direction >> for reversed()."""
    rev = 0
    tmp = ~(player | blank_masked) & (site >> dir)
    if tmp:
        for i in range(6):
            tmp >>= dir
            if tmp & blank_masked:
                break
            elif tmp & player:
                rev |= tmp << dir
                break
            else:
                tmp |= tmp << dir
    return rev

def GetReversable(m, y, move):
    """Return reversed site board."""
    blank_h = ~(m | y & 0x7e7e7e7e7e7e7e7e)
    blank_v = ~(m | y & 0x00ffffffffffff00)
    blank_a = ~(m | y & 0x007e7e7e7e7e7e00)
    rev = _reversed_l(m, blank_h, move, 1)
    rev |= _reversed_l(m, blank_v, move, 8)
    rev |= _reversed_l(m, blank_a, move, 7)
    rev |= _reversed_l(m, blank_a, move, 9)
    rev |= _reversed_r(m, blank_h, move, 1)
    rev |= _reversed_r(m, blank_v, move, 8)
    rev |= _reversed_r(m, blank_a, move, 7)
    rev |= _reversed_r(m, blank_a, move, 9)
    return rev

def Reverse(m, y, move, reversable):
    
    m |= move
    m |= reversable
    y ^= reversable
    
    return m,y

def Put(turn, black, white, move):

    if turn == 'b':
        m = black
        y = white
    elif turn == 'w':
        m = white
        y = black
    elif turn == 'e':
        return turn, black, white

    if move & (black|white):
        print('石があるので置けません')
    elif not move & GetMovable(m, y):
        print('石を返せないので置けません')
    else:
        reversable = GetReversable(m, y, move)
        m, y = Reverse(m, y, move, reversable)
        turn, black, white = UpdateTurn(turn, m, y)

    return turn, black, white

def UpdateTurn(turn, m, y):

    if turn == 'b':
        black = m
        white = y
        if GetMovable(y, m):
            turn = 'w'
        else:
            if GetMovable(m, y):
                turn = 'b'
            else:
                turn = 'e'
    elif turn == 'w':
        black = y
        white = m
        if GetMovable(y, m):
            turn = 'b'
        else:
            if GetMovable(m, y):
                turn = 'w'
            else:
                turn = 'e'

    return turn, black, white
