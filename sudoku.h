/*******************************************************************
*
*	Author: Kareem Omar
*	kareem.h.omar@gmail.com
*	https://github.com/komrad36
*
*	Last updated Nov 1, 2020
*******************************************************************/

#pragma once

#include <cstdint>

enum class SolveResult : uint8_t
{
    kNoSolution,
    kUniqueSolution,
    kMultipleSolutions,
    kInvalidBoard,
};

struct SudokuBoard
{
    uint8_t m_cells[96];
};

SolveResult Solve(SudokuBoard& sol, const SudokuBoard& b);
