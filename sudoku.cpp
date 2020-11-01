/*******************************************************************
*
*	Author: Kareem Omar
*	kareem.h.omar@gmail.com
*	https://github.com/komrad36
*
*	Last updated Nov 1, 2020
*******************************************************************/

#include "sudoku.h"

#include <immintrin.h>

using U8 = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;

#if defined(__clang__) || defined(__GNUC__)
#define UNLIKELY(x) (__builtin_expect((x), 0))
#else
#define UNLIKELY(x) (x)
#include <intrin.h>
#endif

struct SolveState
{
	U8 m_board[96];
	U16 m_row[16];
	U16 m_col[16];
	U16 m_sqr[16];
};

static inline U16 Popcnt16(U16 x)
{
#if defined(__clang__) || defined(__GNUC__)
	U16 o;
	asm("popcntw %[x], %[o]\n" : [o] "=r" (o) : [x] "r" (x));
	return o;
#else
	return __popcnt16(x);
#endif
}

template <
	U8 a, U8 b, U8 c, U8 d, U8 e, U8 f, U8 g, U8 h,
	U8 i, U8 j, U8 k, U8 l, U8 m, U8 n, U8 o, U8 p
>
static inline __m256i mm256_shuffle_epi16(__m256i v)
{
	return _mm256_shuffle_epi8(v, _mm256_setr_epi8(
		2 * a, 2 * a + 1,
		2 * b, 2 * b + 1,
		2 * c, 2 * c + 1,
		2 * d, 2 * d + 1,
		2 * e, 2 * e + 1,
		2 * f, 2 * f + 1,
		2 * g, 2 * g + 1,
		2 * h, 2 * h + 1,
		2 * i, 2 * i + 1,
		2 * j, 2 * j + 1,
		2 * k, 2 * k + 1,
		2 * l, 2 * l + 1,
		2 * m, 2 * m + 1,
		2 * n, 2 * n + 1,
		2 * o, 2 * o + 1,
		2 * p, 2 * p + 1));
}

static inline __m256i CellCount(const __m256i p)
{
	const __m256i T = _mm256_setr_epi64x(0x0302020102010100, 0x0403030203020201, 0x0302020102010100, 0x0403030203020201);
	return _mm256_add_epi16(
		_mm256_srli_epi16(p, 9),
		_mm256_add_epi16(
			_mm256_shuffle_epi8(T, _mm256_srli_epi16(p, 5)),
			_mm256_shuffle_epi8(T, _mm256_srli_epi16(_mm256_slli_epi16(p, 11), 12))
		)
	);
}

static SolveResult SolveInternal(SolveState& s, U8* __restrict const pSol)
{
	// 0 1 2 3 4 5 6 7 | 0 1 2 3 4 5 6 7
	const __m256i rowBaseA = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)s.m_row));

	// 1 2 3 4 5 6 7 8 | 1 2 3 4 5 6 7 8
	const __m256i rowBaseB = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)(s.m_row + 1)));

	// row
	// A:  0, 0, 0, 0, 0, 0, 0, 0 | 0, 1, 1, 1, 1, 1, 1, 1
	// B:  1, 1, 2, 2, 2, 2, 2, 2 | 2, 2, 2, 3, 3, 3, 3, 3
	// C:  3, 3, 3, 3, 4, 4, 4, 4 | 4, 4, 4, 4, 4, 5, 5, 5
	// D:  5, 5, 5, 5, 5, 5, 6, 6 | 6, 6, 6, 6, 6, 6, 6, 7
	// E:  7, 7, 7, 7, 7, 7, 7, 7 | 8, 8, 8, 8, 8, 8, 8, 8

	__m256i cellA = mm256_shuffle_epi16<0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1>(rowBaseA);
	__m256i cellB = mm256_shuffle_epi16<1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3>(rowBaseA);
	__m256i cellC = mm256_shuffle_epi16<3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5>(rowBaseA);
	__m256i cellD = mm256_shuffle_epi16<5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7>(rowBaseA);
	__m256i cellE = mm256_shuffle_epi16<6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7>(rowBaseB);

	// col
	// A:  0, 1, 2, 3, 4, 5, 6, 7 | 8, 0, 1, 2, 3, 4, 5, 6
	// B:  7, 8, 0, 1, 2, 3, 4, 5 | 6, 7, 8, 0, 1, 2, 3, 4
	// C:  5, 6, 7, 8, 0, 1, 2, 3 | 4, 5, 6, 7, 8, 0, 1, 2
	// D:  3, 4, 5, 6, 7, 8, 0, 1 | 2, 3, 4, 5, 6, 7, 8, 0
	// E:  1, 2, 3, 4, 5, 6, 7, 8 | 0, 1, 2, 3, 4, 5, 6, 7

	// 0 1 2 3 4 5 6 7 | 0 1 2 3 4 5 6 7
	const __m256i colBaseLo = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)s.m_col));

	// 1 2 3 4 5 6 7 8 | 1 2 3 4 5 6 7 8
	const __m256i colBaseHi = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)(s.m_col + 1)));

	// 0 1 2 3 4 5 6 7 | 8 0 1 2 3 4 5 6
	const __m256i colBaseA = _mm256_blend_epi32(colBaseLo, _mm256_alignr_epi8(colBaseLo, colBaseHi, 14), 240);

	// 1 2 3 4 5 6 7 8 | 0 1 2 3 4 5 6 7
	const __m256i colBaseB = _mm256_blend_epi32(colBaseLo, colBaseHi, 15);

	cellA = _mm256_or_si256(cellA, colBaseA);
	cellB = _mm256_or_si256(cellB, _mm256_alignr_epi8(colBaseA, colBaseB, 12));
	cellC = _mm256_or_si256(cellC, _mm256_alignr_epi8(colBaseA, colBaseB, 8));
	cellD = _mm256_or_si256(cellD, _mm256_alignr_epi8(colBaseA, colBaseB, 4));
	cellE = _mm256_or_si256(cellE, colBaseB);

	// sqr
	// A:  0, 0, 0, 1, 1, 1, 2, 2 | 2, 0, 0, 0, 1, 1, 1, 2
	// B:  2, 2, 0, 0, 0, 1, 1, 1 | 2, 2, 2, 3, 3, 3, 4, 4
	// C:  4, 5, 5, 5, 3, 3, 3, 4 | 4, 4, 5, 5, 5, 3, 3, 3
	// D:  4, 4, 4, 5, 5, 5, 6, 6 | 6, 7, 7, 7, 8, 8, 8, 6
	// E:  6, 6, 7, 7, 7, 8, 8, 8 | 6, 6, 6, 7, 7, 7, 8, 8

	// 0 1 2 3 4 5 6 7 | 0 1 2 3 4 5 6 7
	const __m256i sqrBaseA = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)s.m_sqr));

	// 1 2 3 4 5 6 7 8 | 1 2 3 4 5 6 7 8
	const __m256i sqrBaseB = _mm256_broadcastsi128_si256(_mm_loadu_si128((const __m128i*)(s.m_sqr + 1)));

	cellA = _mm256_or_si256(cellA, mm256_shuffle_epi16<0, 0, 0, 1, 1, 1, 2, 2, 2, 0, 0, 0, 1, 1, 1, 2>(sqrBaseA));
	cellB = _mm256_or_si256(cellB, mm256_shuffle_epi16<2, 2, 0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4>(sqrBaseA));
	cellC = _mm256_or_si256(cellC, mm256_shuffle_epi16<4, 5, 5, 5, 3, 3, 3, 4, 4, 4, 5, 5, 5, 3, 3, 3>(sqrBaseA));
	cellD = _mm256_or_si256(cellD, mm256_shuffle_epi16<3, 3, 3, 4, 4, 4, 5, 5, 5, 6, 6, 6, 7, 7, 7, 5>(sqrBaseB));
	cellE = _mm256_or_si256(cellE, mm256_shuffle_epi16<5, 5, 6, 6, 6, 7, 7, 7, 5, 5, 5, 6, 6, 6, 7, 7>(sqrBaseB));

	const __m256i cellCountF = _mm256_castsi128_si256(_mm_cvtsi32_si128(Popcnt16(s.m_row[8] | s.m_col[8] | s.m_sqr[8])));

	// popcount the cells

	__m256i cellCountA = _mm256_packs_epi16(CellCount(cellA), CellCount(cellB));
	__m256i cellCountB = _mm256_packs_epi16(CellCount(cellC), CellCount(cellD));
	__m256i cellCountC = _mm256_packs_epi16(CellCount(cellE), cellCountF);

	// keep only cells for which there isn't already an entry on the board

	cellCountA = _mm256_and_si256(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_loadu_si256((const __m256i*)s.m_board + 0)), cellCountA);
	cellCountB = _mm256_and_si256(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_loadu_si256((const __m256i*)s.m_board + 1)), cellCountB);
	cellCountC = _mm256_and_si256(_mm256_cmpeq_epi8(_mm256_setzero_si256(), _mm256_loadu_si256((const __m256i*)s.m_board + 2)), cellCountC);

	// get max count, saving intermediate maxes for index calculation

	__m256i cellPopValA1 = _mm256_max_epi8(cellCountA, cellCountB);
	__m256i cellPopValB1 = _mm256_max_epi8(cellPopValA1, cellCountC);

	__m128i cellPopValA2 = _mm256_castsi256_si128(cellPopValB1);
	__m128i cellPopValB2 = _mm256_extracti128_si256(cellPopValB1, 1);

	__m128i cellPopValA3 = _mm_max_epi8(cellPopValA2, cellPopValB2);
	__m128i cellPopValB3 = _mm_unpackhi_epi64(cellPopValA3, cellPopValA3);

	__m128i cellPopValA4 = _mm_max_epi8(cellPopValA3, cellPopValB3);
	__m128i cellPopValB4 = _mm_castps_si128(_mm_movehdup_ps(_mm_castsi128_ps(cellPopValA4)));

	__m128i cellPopValA5 = _mm_max_epi8(cellPopValA4, cellPopValB4);
	__m128i cellPopValB5 = _mm_shufflelo_epi16(cellPopValA5, 225);

	__m128i cellPopValA6 = _mm_max_epi8(cellPopValA5, cellPopValB5);
	__m128i cellPopValB6 = _mm_srli_si128(cellPopValA6, 1);

	const U32 bestCount = (U32)(U8)_mm_cvtsi128_si32(_mm_max_epi8(cellPopValA6, cellPopValB6));

	if (bestCount == 9)
		return SolveResult::kNoSolution;

	if (UNLIKELY(bestCount == 0))
	{
		_mm256_storeu_si256((__m256i*)pSol + 0, _mm256_loadu_si256((const __m256i*)s.m_board + 0));
		_mm256_storeu_si256((__m256i*)pSol + 1, _mm256_loadu_si256((const __m256i*)s.m_board + 1));
		_mm256_storeu_si256((__m256i*)pSol + 2, _mm256_loadu_si256((const __m256i*)s.m_board + 2));
		return SolveResult::kUniqueSolution;
	}

	// after early outs, go back and calculate index of max

	cellCountA = _mm256_andnot_si256(_mm256_cmpgt_epi8(cellCountA, cellCountB), _mm256_set1_epi8(32));
	cellCountA = _mm256_blendv_epi8(cellCountA, _mm256_set1_epi8(64), _mm256_cmpgt_epi8(cellCountC, cellPopValA1));
	cellCountA = _mm256_or_si256(cellCountA, _mm256_setr_epi64x(0x0706050403020100, 0x0f0e0d0c0b0a0908, 0x1716151413121110, 0x1f1e1d1c1b1a1918));

	__m128i maxIdx = _mm256_castsi256_si128(cellCountA);
	maxIdx = _mm_blendv_epi8(_mm256_extracti128_si256(cellCountA, 1), maxIdx, _mm_cmpgt_epi8(cellPopValA2, cellPopValB2));
	maxIdx = _mm_blendv_epi8(_mm_unpackhi_epi64(maxIdx, maxIdx), maxIdx, _mm_cmpgt_epi8(cellPopValA3, cellPopValB3));
	maxIdx = _mm_blendv_epi8(_mm_castps_si128(_mm_movehdup_ps(_mm_castsi128_ps(maxIdx))), maxIdx, _mm_cmpgt_epi8(cellPopValA4, cellPopValB4));
	maxIdx = _mm_blendv_epi8(_mm_shufflelo_epi16(maxIdx, 225), maxIdx, _mm_cmpgt_epi8(cellPopValA5, cellPopValB5));
	maxIdx = _mm_blendv_epi8(_mm_srli_si128(maxIdx, 1), maxIdx, _mm_cmpgt_epi8(cellPopValA6, cellPopValB6));

	const U32 k = (U32)(U8)_mm_cvtsi128_si32(maxIdx);

	static constexpr U8 kLut[] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 4, 4, 4, 4, 4, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 8, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 0, 1, 2, 3, 4, 5, 6, 7, 7, 8, 0,
		1, 2, 3, 4, 5, 8, 0, 1, 2, 3, 4, 5, 6, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 3, 4, 5, 6,
		7, 8, 0, 1, 4, 5, 6, 7, 8, 0, 1, 2, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 8, 0, 0, 0, 0,
		0, 0, 0, 0, 1, 2, 3, 4, 5, 6, 7, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 1, 1, 1, 2, 0, 0, 0, 1, 1,
		1, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6, 4, 4, 5, 5, 5, 3, 3,
		3, 6, 7, 7, 7, 8, 8, 8, 6, 6, 6, 7, 7, 7, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 6, 6, 6, 7, 7, 7, 8, 8,
	};

	const U32 i = kLut[k + 0*88];
	const U32 j = kLut[k + 1*88];
	const U32 m = kLut[k + 2*88];

	U16 f = (s.m_row[i] | s.m_col[j] | s.m_sqr[m]) ^ 0x3FEU;

	U32 c = bestCount - 8;
	U32 res = 0;
	while (res | c)
	{
		SolveState newState = s;
		const U16 r = _blsi_u32(f);
		newState.m_row[i] |= r;
		newState.m_col[j] |= r;
		newState.m_sqr[m] |= r;
		newState.m_board[k] = _tzcnt_u32(r);
		res += (U8)SolveInternal(newState, pSol);
		if (res >= (U8)SolveResult::kMultipleSolutions)
			return SolveResult::kMultipleSolutions;

		f = _blsr_u32(f);
		++c;

		if (!f)
			return (SolveResult)res;
	}

	s.m_row[i] |= f;
	s.m_col[j] |= f;
	s.m_sqr[m] |= f;
	s.m_board[k] = _tzcnt_u32(f);
	return SolveInternal(s, pSol);
}

SolveResult Solve(SudokuBoard& sol, const SudokuBoard& b)
{
	const __m256i A = _mm256_permute4x64_epi64(_mm256_loadu_si256((const __m256i*)b.m_cells + 0), 216);
	const __m256i B = _mm256_permute4x64_epi64(_mm256_loadu_si256((const __m256i*)b.m_cells + 1), 216);
	const __m256i C = _mm256_and_si256(_mm256_permute4x64_epi64(_mm256_loadu_si256((const __m256i*)b.m_cells + 2), 216),
		_mm256_setr_epi64x(~0ULL, 0xFFULL, ~0ULL, 0ULL));

	SolveState s;
	_mm256_storeu_si256((__m256i*)s.m_board + 0, A);
	_mm256_storeu_si256((__m256i*)s.m_board + 1, B);
	_mm256_storeu_si256((__m256i*)s.m_board + 2, C);
	_mm256_storeu_si256((__m256i*)s.m_row, _mm256_setzero_si256());
	_mm256_storeu_si256((__m256i*)s.m_col, _mm256_setzero_si256());
	_mm256_storeu_si256((__m256i*)s.m_sqr, _mm256_setzero_si256());
	const __m256i mask = _mm256_or_si256(_mm256_or_si256(A, B), C);
	if (_mm256_testz_si256(mask, mask))
	{
		s.m_board[0] = 1;
		s.m_row[0] = 1U << 1;
		s.m_col[0] = 1U << 1;
		s.m_sqr[0] = 1U << 1;
	}
	else
	{
		U32 invalid = 0U;

#define ITER0(k)											\
if (const U32 t = b.m_cells[k])								\
{															\
	constexpr U32 i = (57 * U32(k)) >> 9;					\
	constexpr U32 j = U32(k) - 9 * i;						\
	constexpr U32 m = 3 * (11 * i >> 5) + (11 * j >> 5);	\
															\
	const U32 r = 1U << t;									\
															\
	const U32 a = s.m_row[i];								\
	const U32 b = s.m_col[j];								\
	const U32 c = s.m_sqr[m];								\
															\
	if ((a | b | c) & r)									\
		++invalid;											\
															\
	s.m_row[i] = a | r;										\
	s.m_col[j] = b | r;										\
	s.m_sqr[m] = c | r;										\
}
#define ITER1(n) ITER0(n) ITER0(n+1) ITER0(n+2)
#define ITER2(n) ITER1(n) ITER1(n+3) ITER1(n+6)
#define ITER3(n) ITER2(n) ITER2(n+9) ITER2(n+18)
		ITER3(0) ITER3(27) ITER3(54)

		if (invalid)
			return SolveResult::kInvalidBoard;
	}

	const SolveResult ret = SolveInternal(s, sol.m_cells);

	_mm256_storeu_si256((__m256i*)sol.m_cells + 0, _mm256_permute4x64_epi64(_mm256_loadu_si256((const __m256i*)sol.m_cells + 0), 216));
	_mm256_storeu_si256((__m256i*)sol.m_cells + 1, _mm256_permute4x64_epi64(_mm256_loadu_si256((const __m256i*)sol.m_cells + 1), 216));
	_mm256_storeu_si256((__m256i*)sol.m_cells + 2, _mm256_permute4x64_epi64(_mm256_loadu_si256((const __m256i*)sol.m_cells + 2), 216));

	return ret;
}
