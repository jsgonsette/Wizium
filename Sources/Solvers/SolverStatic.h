// ###########################################################################
//
// This file is part of the Wizium distribution (https://github.com/jsgonsette/Wizium).
// Copyright (c) 2019 Jean-Sebastien Gonsette.
//
// This program is free software : you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 3.

// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.If not, see <http://www.gnu.org/licenses/>.
///
/// \file		SolverStatic.h
/// \author		Jean-Sebastien Gonsette
///
/// \brief		Solver with fixed grid layout
// ###########################################################################

#ifndef __CSOLVER_STAT__H
#define __CSOLVER_STAT__H

#include "ISolver.h"



// ###########################################################################
//
// T Y P E S
//
// ###########################################################################



struct CrossMask
{
	uint8_t mask [MAX_GRID_SIZE + 1];
	int len;
	int backOffset;
};


// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

class SolverStatic : public ISolver
{

private:

	class StaticItem;


public:

	SolverStatic ();
	~SolverStatic ();

	void SetHeurestic (bool state, int backTreshold);
		
	void Solve_Start (Grid &grid, const Dictionary &dico);
	Status Solve_Step (int32_t maxTimeMs, int32_t maxSteps);
	void Solve_Stop ();

private:

	void AddCurrentItem ();
	void BackTrack ();
	int BackTrackStep (int idxTarget, int& targetCol, int idx);
	
	bool ChangeItem (StaticItem &item, int colToChange, unsigned int* pNumAttempts);
	bool ChangeItemWord (StaticItem &item, uint8_t mask [], int unvalidatedIdx, bool strict);
	bool CheckItemCross (StaticItem &item, int *pBestPos);
	void BuildCrossMasks (StaticItem &item);

	void BuildWordList ();
	int BuildWords (StaticItem pList [], int listLength);
	void SortWordList (StaticItem pList [], int listLength);
	int AreDependant (const StaticItem &item1, const StaticItem &item2, uint8_t* dependencyMask);

	int FindWordToStart (StaticItem pList [], int listLength);
	int FindWordNext (StaticItem pList [], int listLength);
	
	void SaveCandidatesToGrid (const StaticItem &iItem);
	void LoadCandidatesFromGrid (StaticItem &item);
	void ResetCandidatesAround (const StaticItem &item);
	

private:

	StaticItem* items;						///< All the horizontal slots to resolve
	CrossMask crossMasks [MAX_GRID_SIZE];	///< Cross masks for the current item to solve
	
	int numItems;							///< Number of words to place on the grid
	int idxCurrentItem;						///< Current slot we are currently resolving											
	
	// Heurestic
	bool heurestic;
	int stepBack;
};



#endif


