/*******************************************************************
*
*	Author: Kareem Omar
*	kareem.h.omar@gmail.com
*	https://github.com/komrad36
*
*	Last updated June 7, 2020
*******************************************************************/

#include "sudoku.h"

#include <iostream>

static void PrintBoard(const Board& b)
{
	std::cout << "\xda\xc4\xc4\xc4\xc4\xc4\xc2\xc4\xc4\xc4\xc4\xc4\xc2\xc4\xc4\xc4\xc4\xc4\xbf" << std::endl;

	for (int i = 0; i < 9; ++i)
	{
		std::cout << "\xb3";
		for (int j = 0; j < 9; ++j)
		{
			const int c = b.Get(9 * i + j);
			if (j % 3 == 2)
				std::cout << ' ';
			if (c)
				std::cout << c;
			else
				std::cout << '-';
			if (j % 3 == 0)
				std::cout << ' ';
			if (j == 2 || j == 5)
				std::cout << "\xb3";
		}
		std::cout << "\xb3" << std::endl;
		if (i == 2 || i == 5)
			std::cout << "\xc3\xc4\xc4\xc4\xc4\xc4\xc5\xc4\xc4\xc4\xc4\xc4\xc5\xc4\xc4\xc4\xc4\xc4\xb4" << std::endl;
	}

	std::cout << "\xc0\xc4\xc4\xc4\xc4\xc4\xc1\xc4\xc4\xc4\xc4\xc4\xc1\xc4\xc4\xc4\xc4\xc4\xd9" << std::endl;
}

int main()
{
	constexpr U8 cells[] =
	{
		8, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 3, 6, 0, 0, 0, 0, 0,
		0, 7, 0, 0, 9, 0, 2, 0, 0,
		0, 5, 0, 0, 0, 7, 0, 0, 0,
		0, 0, 0, 0, 4, 5, 7, 0, 0,
		0, 0, 0, 1, 0, 0, 0, 3, 0,
		0, 0, 1, 0, 0, 0, 0, 6, 8,
		0, 0, 8, 5, 0, 0, 0, 1, 0,
		0, 9, 0, 0, 0, 0, 4, 0, 0,
	};

	//constexpr U8 cells[81] = { 0 };

	Board b;
	for (int i = 0; i < 81; ++i)
		b.Set(i, cells[i]);

	std::cout << "Board before solving:" << std::endl;
	PrintBoard(b);


	Board sol;
	SolveResult res = Solve(sol, b);

	std::cout << "\nSolve result: " << SolveResultToString(res) << std::endl;

	if (res == SolveResult::kUniqueSolution || res == SolveResult::kMultipleSolutions)
	{
		std::cout << "\nSolved board:" << std::endl;
		PrintBoard(sol);
	}

	std::cout << std::endl;
}
