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
/// \file		SOlverDynamic.DynamicItem.h
/// \author		Jean-Sebastien Gonsette
// ###########################################################################

#ifndef __CSOLVERDYN_ITEM__H
#define __CSOLVERDYN_ITEM__H

#include "Grid/Grid.h"
#include "Dictionary/Dictionary.h"
#include "Solvers/SolverDynamic.h"


// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

/// Word placed on grid during the backracking process
class SolverDynamic::DynamicItem
{
public:

	DynamicItem ();
	void Reset ();
	void ResetCandidates ();
	void SetCandidate (int pos, uint8_t c, bool state);
	bool IsCandidate (int pos, uint8_t c);

	void SaveCandidatesToGrid (Grid& grid) const;
	void LoadCandidatesFromGrid (Grid& grid);
	void ResetCandidatesBelowItem (Grid& grid) const;
	void AddToGrid (Grid& grid) const;
	void RemoveFromGrid (Grid& grid) const;


public:

	uint8_t word [MAX_WORD_LENGTH + 1];		///< Word content	
	uint8_t firstWord [MAX_WORD_LENGTH + 1];///< Initial word content (firt try)
	int length;								///< Length
	int lengthFirstWord;					///< Length of the first word we tried
	int bestPos;							///< Index of the best letter that could be validated
	bool isBlock;							///< This object is a bloc
	uint8_t posX, posY;						///< Position on grid

	LetterCandidates candidates [MAX_WORD_LENGTH];	///< Possible letters at each position of the grid
	DynamicItem *pNext;								///< Linked list (housekeeping)
};

#endif


