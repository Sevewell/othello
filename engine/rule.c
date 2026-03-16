#include <immintrin.h>

uint64_t GetMovableL(uint64_t m, uint64_t y, uint64_t shift, uint64_t mask)
{
    uint64_t legal = m << shift & mask;
    legal |= legal << shift & mask;
    legal |= legal << shift & mask;
    legal |= legal << shift & mask;
    legal |= legal << shift & mask;
    legal |= legal << shift & mask;
    return legal << shift & ~(m | y);
}

uint64_t GetMovableR(uint64_t m, uint64_t y, uint64_t shift, uint64_t mask)
{
    uint64_t legal = m >> shift & mask;
    legal |= legal >> shift & mask;
    legal |= legal >> shift & mask;
    legal |= legal >> shift & mask;
    legal |= legal >> shift & mask;
    legal |= legal >> shift & mask;
    return legal >> shift & ~(m | y);
}

void GetMovable(uint64_t m, uint64_t y, uint64_t* legals)
{
    uint64_t mask_horizontal = y & 9114861777597660798;
    uint64_t mask_vertical = y & 72057594037927680;
    uint64_t mask_diagonal = y & 35604928818740736;
    legals[0] = GetMovableL(m, y, 1, mask_horizontal);
    legals[1] = GetMovableL(m, y, 9, mask_diagonal);
    legals[2] = GetMovableL(m, y, 8, mask_vertical);
    legals[3] = GetMovableL(m, y, 7, mask_diagonal);
    legals[4] = GetMovableR(m, y, 1, mask_horizontal);
    legals[5] = GetMovableR(m, y, 9, mask_diagonal);
    legals[6] = GetMovableR(m, y, 8, mask_vertical);
    legals[7] = GetMovableR(m, y, 7, mask_diagonal);
}

uint64_t GetLegal(uint64_t* legals)
{
    return legals[0] | legals[1] | legals[2] | legals[3] | legals[4] | legals[5] | legals[6] | legals[7];
}

uint64_t GetReversableL(uint64_t y, uint64_t move, uint8_t shift, uint64_t legal)
{
    uint64_t flipped = (move & legal) >> shift & y;
    flipped |= flipped >> shift & y;
    flipped |= flipped >> shift & y;
    flipped |= flipped >> shift & y;
    flipped |= flipped >> shift & y;
    flipped |= flipped >> shift & y;
    return flipped;
}

uint64_t GetReversableR(uint64_t y, uint64_t move, uint8_t shift, uint64_t legal)
{
    uint64_t flipped = (move & legal) << shift & y;
    flipped |= flipped << shift & y;
    flipped |= flipped << shift & y;
    flipped |= flipped << shift & y;
    flipped |= flipped << shift & y;
    flipped |= flipped << shift & y;
    return flipped;
}

uint64_t GetReversable(uint64_t m, uint64_t y, uint64_t move, uint64_t* legals)
{
    uint64_t flipped = 0;
    flipped |= GetReversableL(y, move, 1, legals[0]);
    flipped |= GetReversableL(y, move, 9, legals[1]);
    flipped |= GetReversableL(y, move, 8, legals[2]);
    flipped |= GetReversableL(y, move, 7, legals[3]);
    flipped |= GetReversableR(y, move, 1, legals[4]);
    flipped |= GetReversableR(y, move, 9, legals[5]);
    flipped |= GetReversableR(y, move, 8, legals[6]);
    flipped |= GetReversableR(y, move, 7, legals[7]);
    return flipped;
}
