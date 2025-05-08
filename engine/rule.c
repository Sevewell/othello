#include <immintrin.h>

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

uint64_t GetMovable_SIMD(const uint64_t P, const uint64_t O)
{
	__m256i	PP, mOO, MM, flip_l, flip_r, pre_l, pre_r, shift2;
	__m128i	M;
	const __m256i shift1897 = _mm256_set_epi64x(7, 9, 8, 1);
	const __m256i mflipH = _mm256_set_epi64x(0x7e7e7e7e7e7e7e7e, 0x7e7e7e7e7e7e7e7e, -1, 0x7e7e7e7e7e7e7e7e);

	PP = _mm256_broadcastq_epi64(_mm_cvtsi64_si128(P));
	mOO = _mm256_and_si256(_mm256_broadcastq_epi64(_mm_cvtsi64_si128(O)), mflipH);

	flip_l = _mm256_and_si256(mOO, _mm256_sllv_epi64(PP, shift1897));
	flip_r = _mm256_and_si256(mOO, _mm256_srlv_epi64(PP, shift1897));
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(mOO, _mm256_sllv_epi64(flip_l, shift1897)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(mOO, _mm256_srlv_epi64(flip_r, shift1897)));
	pre_l = _mm256_and_si256(mOO, _mm256_sllv_epi64(mOO, shift1897));
	pre_r = _mm256_srlv_epi64(pre_l, shift1897);
	shift2 = _mm256_add_epi64(shift1897, shift1897);
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(pre_l, _mm256_sllv_epi64(flip_l, shift2)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(pre_r, _mm256_srlv_epi64(flip_r, shift2)));
	flip_l = _mm256_or_si256(flip_l, _mm256_and_si256(pre_l, _mm256_sllv_epi64(flip_l, shift2)));
	flip_r = _mm256_or_si256(flip_r, _mm256_and_si256(pre_r, _mm256_srlv_epi64(flip_r, shift2)));
	MM = _mm256_sllv_epi64(flip_l, shift1897);
	MM = _mm256_or_si256(MM, _mm256_srlv_epi64(flip_r, shift1897));

	M = _mm_or_si128(_mm256_castsi256_si128(MM), _mm256_extracti128_si256(MM, 1));
	M = _mm_or_si128(M, _mm_unpackhi_epi64(M, M));
	return _mm_cvtsi128_si64(M) & ~(P|O);	// mask with empties
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