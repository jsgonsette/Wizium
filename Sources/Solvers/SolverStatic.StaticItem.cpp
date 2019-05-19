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
/// \file		DynamicItem.cpp
/// \author		Jean-Sebastien Gonsette
///
/// \brief		A word element put on the grid, used by the dynamic solver
// ###########################################################################

#include "Solvers/SolverStatic.StaticItem.h"
#include <assert.h> 


// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief		Constructor
// ===========================================================================
SolverStatic::StaticItem::StaticItem ()
{
	word [0] = 0;
	prevWord [0] = 0;
	firstWord [0] = 0;

	processOrder = -1;
	connectionStrength = 0;

	length = 0;
	posX = 0;
	posY = 0;
	bestPos = -1;

	failCounter = 0;
	failTotCounter = 0;
}


// ===========================================================================
/// \brief		Destructor
// ===========================================================================
SolverStatic::StaticItem::~StaticItem ()
{
}


// ===========================================================================
/// \brief		Reset this element
// ===========================================================================
void SolverStatic::StaticItem::Reset ()
{
	word [0] = 0;
	prevWord [0] = 0;
	firstWord [0] = 0;
	failCounter = 0;
	failTotCounter = 0;
	ResetCrossCandidates ();
}


// ===========================================================================
/// \brief		Reset letter candidates
// ===========================================================================
void SolverStatic::StaticItem::ResetCandidates ()
{
	// All letter boxes in the slot are valid candidates
	for (int i = 0; i < MAX_GRID_SIZE; i++) {
		possibleLetters [i].Reset (true);
	}
}


// ===========================================================================
/// \brief		Set letter candidate at a given grid position
///
/// \param	pos		Position, along the grid (not along the word)
/// \param	c		Target Letter (1..alphabetsize)
/// \param	state	New value
// ===========================================================================
void SolverStatic::StaticItem::SetCandidate (int pos, uint8_t c, bool state)
{
	assert (c > 0);

	if (pos < 0 || pos >= MAX_GRID_SIZE) return;
	possibleLetters [pos].Set (c-1, state);
}


// ===========================================================================
/// \brief	Query letter possibility at a given grid position
///
/// \param	pos		Position, along the grid (not along the word)
/// \param	c		Letter to test (1..alphabetsize)
// ===========================================================================
bool SolverStatic::StaticItem::IsCandidate (int pos, uint8_t c)
{
	assert (c > 0);

	if (pos < 0 || pos >= MAX_GRID_SIZE) return false;
	return possibleLetters [pos].Query (c-1);
}


// ===========================================================================
// ===========================================================================
void SolverStatic::StaticItem::ResetCrossCandidates ()
{
	// Non any letter boxes in the slot have been cross tested
	for (int i = 0; i < MAX_GRID_SIZE; i++) {
		crossTestedCandidates [i].Reset (false);
	}
}


// ===========================================================================
// ===========================================================================
void SolverStatic::StaticItem::SetCrossCandidate (int pos, uint8_t c, bool state)
{
	assert (c > 0);

	if (pos < 0 || pos >= MAX_GRID_SIZE) return;
	crossTestedCandidates [pos].Set (c-1, state);
}


// ===========================================================================
// ===========================================================================
bool SolverStatic::StaticItem::IsCrossTested (int pos, uint8_t c)
{
	assert (c > 0);

	if (pos < 0 || pos >= MAX_GRID_SIZE) return false;
	return crossTestedCandidates [pos].Query (c-1);
}


// End
