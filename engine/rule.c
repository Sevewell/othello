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

uint64_t GetReversable_SIMD(uint64_t m, uint64_t y, uint64_t move)
{
    __m256i flipped = _mm256_setzero_si256();

    __m256i ms = _mm256_set1_epi64x(m);
    __m256i ys = _mm256_set1_epi64x(y);
    __m256i move_ = _mm256_set1_epi64x(move);

    __m256i shifts = _mm256_set_epi64x(1, 7, 8, 9);
    __m256i mask = _mm256_set_epi64x(9114861777597660798, 35604928818740736, 72057594037927680, 35604928818740736);

    __m256i blank = _mm256_set1_epi64x(~(m | y));

    __m256i temp = _mm256_setzero_si256();
    __m256i moves = _mm256_setzero_si256();

    temp = _mm256_and_si256(_mm256_sllv_epi64(ms, shifts), mask);
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), mask));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), mask));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), mask));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), mask));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), mask));
    moves = _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), blank);
    temp = _mm256_and_si256(_mm256_srlv_epi64(_mm256_and_si256(move_, moves), shifts), ys);
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), ys));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), ys));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), ys));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), ys));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), ys));
    flipped = _mm256_or_si256(flipped, temp);

    temp = _mm256_and_si256(_mm256_srlv_epi64(ms, shifts), mask);
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), mask));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), mask));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), mask));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), mask));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), mask));
    moves = _mm256_and_si256(_mm256_srlv_epi64(temp, shifts), blank);
    temp = _mm256_and_si256(_mm256_sllv_epi64(_mm256_and_si256(move_, moves), shifts), ys);
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), ys));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), ys));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), ys));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), ys));
    temp = _mm256_or_si256(temp, _mm256_and_si256(_mm256_sllv_epi64(temp, shifts), ys));
    flipped = _mm256_or_si256(flipped, temp);

    __m128i high = _mm256_extracti128_si256(flipped, 1);
    __m128i low = _mm256_castsi256_si128(flipped);
    __m128i combined = _mm_or_si128(high, low);
    __m128i shifted = _mm_unpackhi_epi64(combined, combined);
    __m128i result = _mm_or_si128(combined, shifted);
    return _mm_cvtsi128_si64(result);  
}