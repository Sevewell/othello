uint64_t GetMovableL
(uint64_t m, uint64_t y, uint64_t mask, uint64_t dir)
{
    uint64_t y_masked = y & mask;
    uint64_t tmp = (m << dir) & y_masked;
    tmp |= (tmp << dir) & y_masked;
    tmp |= (tmp << dir) & y_masked;
    tmp |= (tmp << dir) & y_masked;
    tmp |= (tmp << dir) & y_masked;
    tmp |= (tmp << dir) & y_masked;
    tmp |= (tmp << dir) & y_masked;
    uint64_t stone = m | y;
    uint64_t empty = ~stone;
    return (tmp << dir) & empty;
}

uint64_t GetMovableR
(uint64_t m, uint64_t y, uint64_t mask, uint64_t dir)
{
    uint64_t y_masked = y & mask;
    uint64_t tmp = (m >> dir) & y_masked;
    tmp |= (tmp >> dir) & y_masked;
    tmp |= (tmp >> dir) & y_masked;
    tmp |= (tmp >> dir) & y_masked;
    tmp |= (tmp >> dir) & y_masked;
    tmp |= (tmp >> dir) & y_masked;
    tmp |= (tmp >> dir) & y_masked;
    uint64_t stone = m | y;
    uint64_t empty = ~stone;
    return (tmp >> dir) & empty;
}

uint64_t GetMovable(uint64_t m, uint64_t y)
{
    uint64_t mask_horizontal = 9114861777597660798;
    uint64_t mask_vertical = 72057594037927680;
    uint64_t mask_allside = 35604928818740736;
    uint64_t legal = 0;
    legal |= GetMovableL(m, y, mask_horizontal, 1);
    legal |= GetMovableL(m, y, mask_vertical, 8);
    legal |= GetMovableL(m, y, mask_allside, 7);
    legal |= GetMovableL(m, y, mask_allside, 9);
    legal |= GetMovableR(m, y, mask_horizontal, 1);
    legal |= GetMovableR(m, y, mask_vertical, 8);
    legal |= GetMovableR(m, y, mask_allside, 7);
    legal |= GetMovableR(m, y, mask_allside, 9);
    return legal;
}

uint64_t GetReversableL
(uint64_t m, uint64_t y, uint64_t move, uint64_t mask, uint8_t dir)
{
    uint64_t tmp = (move << dir) & mask;
    uint64_t reverse = tmp & y;
    while (tmp & y)
    {
        tmp = tmp << dir;
        reverse |= tmp & y;
    }
    if (m & tmp)
    {
        return reverse;
    }
    else
    {
        return 0;
    }
}

uint64_t GetReversableR
(uint64_t m, uint64_t y, uint64_t move, uint64_t mask, uint8_t dir)
{
    uint64_t tmp = (move >> dir) & mask;
    uint64_t reverse = tmp & y;
    while (tmp & y)
    {
        tmp = tmp >> dir;
        reverse |= tmp & y;
    }
    if (m & tmp)
    {
        return reverse;
    }
    else
    {
        return 0;
    }
}

uint64_t GetReversable
(uint64_t m, uint64_t y, uint64_t move)
{
    uint64_t mask_horizontal = 9114861777597660798;
    uint64_t mask_vertical = 72057594037927680;
    uint64_t mask_allside = 35604928818740736;
    uint64_t reverse = 0;
    reverse |= GetReversableL(m, y, move, mask_horizontal, 1);
    reverse |= GetReversableL(m, y, move, mask_vertical, 8);
    reverse |= GetReversableL(m, y, move, mask_allside, 7);
    reverse |= GetReversableL(m, y, move, mask_allside, 9);
    reverse |= GetReversableR(m, y, move, mask_horizontal, 1);
    reverse |= GetReversableR(m, y, move, mask_vertical, 8);
    reverse |= GetReversableR(m, y, move, mask_allside, 7);
    reverse |= GetReversableR(m, y, move, mask_allside, 9);
    return reverse;
}