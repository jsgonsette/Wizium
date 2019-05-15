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
/// \file		SolverDynamic.h
/// \author		Jean-Sebastien Gonsette
// ###########################################################################

#ifndef __CSOLVERDYN__H
#define __CSOLVERDYN__H

#include "Grid/Grid.h"
#include "Dictionary/Dictionary.h"
#include "ISolver.h"


// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

class SolverDynamic: public ISolver
{

private:
	class DynamicItem;

public:

	SolverDynamic ();
	~SolverDynamic ();

	void Solve_Start (Grid &grid, const Dictionary &dico);
	Status Solve_Step (int32_t maxTimeMs, int32_t maxSteps);
	void Solve_Stop ();

	void SetHeurestic (bool state, int stepBack);
	void SetMaxBlackCases (int maxBlackCases) { this->maxBlackCases = maxBlackCases; }
	void SetBlackCasesDensity (Grid::BlocDensityMode density) { this->densityMode = density; }

private:

	DynamicItem* Backtrack (int valRow, int valCol);
	bool ChangeItem (DynamicItem *pItem, bool changeLength, int posToChange, int *pValidatedPos, unsigned int* pNumAttempts);
	bool ChangeItemWord (DynamicItem *pItem, uint8_t mask [], int unvalidatedIdx);
	bool ChangeItemLength (DynamicItem *pItem, int lengthMax);
	int GetInitialLength (int row, int maxLength);

	Grid::Space CheckGridBlock (int x, int y);
	bool CheckItemCross (DynamicItem *pItem, int *pBestPos);
	bool CheckItemLength (const DynamicItem *pItem);

	void FreeItems ();
	bool FindFreeBox (uint8_t *px, uint8_t *py) const;

	void PushUnusedItem (DynamicItem* pItem);
	DynamicItem* PopUnusedItem ();

	DynamicItem* RemoveLastItem ();
	DynamicItem* GetLastItem ();
	void AddItem (DynamicItem* pItem);


private:

	// List of words that are on the grid
	DynamicItem* pItemList;

	// List of unused words (pool)
	DynamicItem* pItemUnused;

	// Heurestic
	bool heurestic;
	int stepBack;

	// Number of black boxes limitation
	int maxBlackCases;
	int initialBlackCases;

	// Type of bloc density
	Grid::BlocDensityMode densityMode;
};




#endif


