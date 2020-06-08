/*******************************************************************
*
*	Author: Kareem Omar
*	kareem.h.omar@gmail.com
*	https://github.com/komrad36
*
*	Last updated June 7, 2020
*******************************************************************/

#pragma once

#if defined(_MSC_VER)
#define ASSUME(x) __assume(x)
#elif defined(__clang__)
#define ASSUME(x) __builtin_assume(x)
#elif defined(__GNUC__)
#define ASSUME(x) do { if (!(x)) __builtin_unreachable(); } while (0)
#else
#define ASSUME(x)
#endif

#include <cstdint>

using U8 = uint8_t;
using U32 = uint32_t;

enum class SolveResult : U8
{
	kNoSolution,
	kUniqueSolution,
	kMultipleSolutions,
	kInvalidBoard,
};

static const char* SolveResultToString(const SolveResult r)
{
	switch (r)
	{
	case SolveResult::kNoSolution:
		return "kNoSolution";
	case SolveResult::kUniqueSolution:
		return "kUniqueSolution";
	case SolveResult::kMultipleSolutions:
		return "kMultipleSolutions";
	case SolveResult::kInvalidBoard:
		return "kInvalidBoard";
	}
	return "kUnknown";
}

class Board
{
public:
	inline void Set(U32 i, U32 x)
	{
		ASSUME(i < 81);
		ASSUME(x < 10);
		m_cells[i] = x;
	}

	inline U8 Get(U32 i) const
	{
		ASSUME(i < 81);
		return m_cells[i];
	}

private:
	U8 m_cells[81];
};

SolveResult Solve(Board& sol, const Board& b);
