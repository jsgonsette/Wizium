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
/// \file		SOlverStatic.StaticItem.h
/// \author		Jean-Sebastien Gonsette
// ###########################################################################

#ifndef __CSOLVERSTAT_ITEM__H
#define __CSOLVERSTAT_ITEM__H

#include "Grid/Grid.h"
#include "Dictionary/Dictionary.h"
#include "Solvers/SolverStatic.h"


// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

/// Word placed on grid during the backracking process
class SolverStatic::StaticItem
{
public:

	StaticItem ();
	~StaticItem ();

	void Reset ();

	void ResetCrossCandidates ();
	void SetCrossCandidate (int pos, uint8_t c, bool state);
	bool IsCrossTested (int pos, uint8_t c);

	void ResetCandidates ();
	void SetCandidate (int pos, uint8_t c, bool state);
	bool IsCandidate (int pos, uint8_t c);
	
public:

	// Dynamic info used when solving
	uint8_t word [MAX_WORD_LENGTH + 1];			///< Current value for the word in the slot	
	uint8_t prevWord [MAX_WORD_LENGTH + 1];		///< Previous word we could successfully put on the grid	
	uint8_t firstWord [MAX_WORD_LENGTH + 1];	///< First word when we start searching a new value, to detect we went around the dictionary
	
	LetterCandidates possibleLetters[MAX_GRID_SIZE];		///< Letter candidates for each item box
	LetterCandidates crossTestedCandidates[MAX_GRID_SIZE];	///< Cross-tested letters for each item box

	// Dynamic info used to backtrack
	int bestPos;					///< Best letter we could cross validate, in case of failure when searching a word.
	bool visibility;				///< Is this word visible to any following word impacted by a failure

	// Static info set once before solving
	int connectionStrength;			///< How much this word connects with previous words in the resolution list
	int processOrder;				///< Backtracking processing order
	uint8_t posX, posY;				///< Item location on grid	
	uint8_t length;					///< Item length

};

#endif


