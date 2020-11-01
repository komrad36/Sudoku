/*******************************************************************
*
*	Author: Kareem Omar
*	kareem.h.omar@gmail.com
*	https://github.com/komrad36
*
*	Last updated June 7, 2020
*******************************************************************/

#include "sudoku.h"

#include <immintrin.h>

// 36.1

using U16 = uint16_t;

static constexpr U32 kLookupI[81] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8,
};

static constexpr U32 kLookupJ[81] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8,
	0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8, 0, 1, 2, 3, 4, 5, 6, 7, 8,
};

static constexpr U32 kLookupM[81] = {
	0, 0, 0, 1, 1, 1, 2, 2, 2, 0, 0, 0, 1, 1, 1, 2, 2, 2, 0, 0, 0, 1, 1, 1, 2, 2, 2,
	3, 3, 3, 4, 4, 4, 5, 5, 5, 3, 3, 3, 4, 4, 4, 5, 5, 5, 3, 3, 3, 4, 4, 4, 5, 5, 5,
	6, 6, 6, 7, 7, 7, 8, 8, 8, 6, 6, 6, 7, 7, 7, 8, 8, 8, 6, 6, 6, 7, 7, 7, 8, 8, 8,
};

struct SolveState
{
	U16 m_rowFree[9];
	U16 m_colFree[9];
	U16 m_sqrFree[9];
	Board m_board;
};

#define ITER1(n) ITER0(n) ITER0(n+1) ITER0(n+2)
#define ITER2(n) ITER1(n) ITER1(n+3) ITER1(n+6)
#define ITER3(n) ITER2(n) ITER2(n+9) ITER2(n+18)
#define ITERS()  ITER3(0) ITER3(27)  ITER3(54)

static SolveResult SolveInternal(Board& sol, SolveState& s)
{
	constexpr U8 kAllCellsSolved = 0;
	constexpr U8 kUnsolvedCells = 1;
	constexpr U8 kBoardUpdated = 3;

	U8 status;
	do
	{
		status = kAllCellsSolved;

		#define ITER0(k)		 												\
		if (!s.m_board.Get(k))												    \
		{																	    \
			constexpr U32 i = kLookupI[k];									    \
			constexpr U32 j = kLookupJ[k];									    \
			constexpr U32 m = kLookupM[k];									    \
			status |= kUnsolvedCells;											\
			const U32 f = s.m_rowFree[i] & s.m_colFree[j] & s.m_sqrFree[m];		\
			if (!f)															    \
				return SolveResult::kNoSolution;							    \
			if (!_blsr_u32(f))													\
			{																    \
				status = kBoardUpdated;											\
				s.m_board.Set(k, _tzcnt_u32(f));							    \
				s.m_rowFree[i] &= ~f;										    \
				s.m_colFree[j] &= ~f;										    \
				s.m_sqrFree[m] &= ~f;										    \
			}																    \
		}
		ITERS();
		#undef ITER0

	} while (status == kBoardUpdated);

	if (status == kAllCellsSolved)
	{
		sol = s.m_board;
		return SolveResult::kUniqueSolution;
	}

	U32 bestK;
	U32 bestCount = ~0U;
#define ITER0(k)																\
	if (!s.m_board.Get(k))													    \
	{																		    \
		constexpr U32 i = kLookupI[k];										    \
		constexpr U32 j = kLookupJ[k];										    \
		constexpr U32 m = kLookupM[k];										    \
		const U32 f = s.m_rowFree[i] & s.m_colFree[j] & s.m_sqrFree[m];			\
		ASSUME(f);																\
		const U32 count = _mm_popcnt_u32(f);								    \
		ASSUME(count > 1);														\
		if (count < bestCount)												    \
		{																	    \
			bestCount = count;												    \
			bestK = k;														    \
			if (count == 2)													    \
				goto label_found_best;										    \
		}																	    \
	}
	ITERS();
#undef ITER0

	ASSUME(bestCount != ~0U);

label_found_best:;

	const U32 k = bestK;
	const U32 i = kLookupI[k];
	const U32 j = kLookupJ[k];
	const U32 m = kLookupM[k];

	U32 f = s.m_rowFree[i] & s.m_colFree[j] & s.m_sqrFree[m];
	U8 res = 0;
	ASSUME(f);

	do
	{
		ASSUME(f);
		SolveState newState = s;
		newState.m_board.Set(k, _tzcnt_u32(f));
		const U32 r = _blsi_u32(f);
		newState.m_rowFree[i] &= ~r;
		newState.m_colFree[j] &= ~r;
		newState.m_sqrFree[m] &= ~r;
		res += (U8)SolveInternal(sol, newState);
		if (res >= (U8)SolveResult::kMultipleSolutions)
			return SolveResult::kMultipleSolutions;
	} while ((f = _blsr_u32(f)));

	return (SolveResult)res;
}

SolveResult Solve(Board& sol, const Board& b)
{
	SolveState s;
	for (int i = 0; i < 9; ++i)
		s.m_rowFree[i] = 0x3FE;
	for (int i = 0; i < 9; ++i)
		s.m_colFree[i] = 0x3FE;
	for (int i = 0; i < 9; ++i)
		s.m_sqrFree[i] = 0x3FE;

	#define ITER0(k)													\
	if (const U8 t = b.Get(k))											\
	{																	\
		constexpr U32 i = kLookupI[k];									\
		constexpr U32 j = kLookupJ[k];									\
		constexpr U32 m = kLookupM[k];									\
																		\
		const U32 r = 1U << t;											\
		if (!(s.m_rowFree[i] & s.m_colFree[j] & s.m_sqrFree[m] & r))	\
			return SolveResult::kInvalidBoard;							\
																		\
		s.m_rowFree[i] &= ~r;											\
		s.m_colFree[j] &= ~r;											\
		s.m_sqrFree[m] &= ~r;											\
	}

	ITERS();

	s.m_board = b;

	return SolveInternal(sol, s);
}
