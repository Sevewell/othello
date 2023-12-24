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
(uint64_t player, uint64_t blank_masked, uint64_t site, int dir)
{
    uint64_t rev = 0;
    uint64_t tmp = ~(player | blank_masked) & (site << dir);

    if (tmp)
    {
        for (int i = 0; i < 6; i++)
        {
            tmp <<= dir;
            if (tmp & blank_masked)
            {
                break;
            }
            else if (tmp & player)
            {
                rev |= tmp >> dir;
                break;
            }
            else
            {
                tmp |= tmp >> dir;
            }
        }
    }

    return rev;
}

uint64_t GetReversableR
(uint64_t player, uint64_t blank_masked, uint64_t site, int dir)
{
    uint64_t rev = 0;
    uint64_t tmp = ~(player | blank_masked) & (site >> dir);

    if (tmp)
    {
        for (int i = 0; i < 6; i++)
        {
            tmp >>= dir;
            if (tmp & blank_masked)
            {
                break;
            }
            else if (tmp & player)
            {
                rev |= tmp << dir;
                break;
            }
            else
            {
                tmp |= tmp << dir;
            }
        }
    }

    return rev;
}

uint64_t GetReversable
(uint64_t m, uint64_t y, uint64_t move)
{
    uint64_t blank_h = ~(m | (y & 0x7e7e7e7e7e7e7e7e));
    uint64_t blank_v = ~(m | (y & 0x00ffffffffffff00));
    uint64_t blank_a = ~(m | (y & 0x007e7e7e7e7e7e00));
    uint64_t rev;
    rev = GetReversableL(m, blank_h, move, 1);
    rev |= GetReversableL(m, blank_v, move, 8);
    rev |= GetReversableL(m, blank_a, move, 7);
    rev |= GetReversableL(m, blank_a, move, 9);
    rev |= GetReversableR(m, blank_h, move, 1);
    rev |= GetReversableR(m, blank_v, move, 8);
    rev |= GetReversableR(m, blank_a, move, 7);
    rev |= GetReversableR(m, blank_a, move, 9);
    
    return rev;
}