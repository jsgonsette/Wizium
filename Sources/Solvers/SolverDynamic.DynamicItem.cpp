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

#include "Solvers/SolverDynamic.DynamicItem.h"
#include <assert.h> 



// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief		Constructor
// ===========================================================================
SolverDynamic::DynamicItem::DynamicItem ()
{
	Reset ();
	pNext = nullptr;
}


// ===========================================================================
/// \brief		Reset this element
// ===========================================================================
void SolverDynamic::DynamicItem::Reset ()
{
	ResetCandidates ();

	word [0] = 0;
	firstWord [0] = 0;

	length = 0;
	lengthFirstWord = 0;
	bestPos = 0;
	isBlock = false;

	posX = 0;
	posY = 0;

	bestPos = -1;
}


// ===========================================================================
/// \brief		Reset letter candidates
// ===========================================================================
void SolverDynamic::DynamicItem::ResetCandidates ()
{
	for (int i = 0; i < MAX_WORD_LENGTH; i ++) {
		candidates [i].Reset (true);
	}
}


// ===========================================================================
/// \brief		Set letter candidate at a given grid position
///
/// \param	pos		Index of target letter
/// \param	c		Target Letter (1..alphabetsize)
/// \param	state	New value
// ===========================================================================
void SolverDynamic::DynamicItem::SetCandidate (int pos, uint8_t c, bool state)
{
	assert (c > 0);

	if (pos < 0 || pos >= MAX_WORD_LENGTH) return;
	candidates [pos].Set (c-1, state);
}


// ===========================================================================
/// \brief	Query letter possibility at a given grid position
///
/// \param	pos		Index of target letter
/// \param	c		Letter to test (1..alphabetsize)
// ===========================================================================
bool SolverDynamic::DynamicItem::IsCandidate (int pos, uint8_t c)
{
	assert (c > 0);

	if (pos < 0 || pos >= MAX_WORD_LENGTH) return false;
	return candidates [pos].Query (c-1);
}


// ===========================================================================
/// \brief		Save the letter candidates of a word on a grid
///
/// \param	grid	Target grid
// ===========================================================================
void SolverDynamic::DynamicItem::SaveCandidatesToGrid (Grid& grid) const
{
	Box *box;
	unsigned char x = posX;
	int i = 0;

	while (x < grid.GetWidth ())
	{
		box = grid.operator ()(x ++, posY);
		box->SetCandidate (candidates [i ++]);
	}
}


// ===========================================================================
/// \brief		Pop letter candidates from a grid
///
/// \param	grid	Target grid
// ===========================================================================
void SolverDynamic::DynamicItem::LoadCandidatesFromGrid (Grid& grid)
{
	Box *box;
	unsigned char x = posX;
	int i = 0;

	ResetCandidates ();

	while (x < grid.GetWidth ())
	{
		box = grid.operator ()(x ++, posY);
		candidates [i ++] = box->GetCandidate ();
	}
}


// ===========================================================================
/// \brief		Reset all the letter candidates possibilities below a word 
///				placed on a grid
///
/// \param	grid	Target grid
// ===========================================================================
void SolverDynamic::DynamicItem::ResetCandidatesBelowItem (Grid& grid) const
{
	int i, j;
	uint8_t x = posX;
	uint8_t y = posY + 1;

	Box *box;

	for (i = 0; i <= length; i ++)
	{
		for (j = y; j < grid.GetHeight (); j ++)
		{
			box = grid.operator ()(x, j);
			if (box != nullptr) box->ResetCandidates (true);
		}

		x ++;
	}
}


// ===========================================================================
/// \brief		Write content on the grid
///
/// \param	grid	Target grid
// ===========================================================================
void SolverDynamic::DynamicItem::AddToGrid (Grid& grid) const
{
	if (isBlock == false)
		grid.AddWord (posX, posY, 'H', word);
	else
		grid.AddBloc (posX, posY);

}

// ===========================================================================
/// \brief		Remove content from the grid
///
/// \param	grid	Target grid
// ===========================================================================
void SolverDynamic::DynamicItem::RemoveFromGrid (Grid& grid) const
{
	if (isBlock == false)
		grid.RemoveWord (posX, posY, 'H');
	else
		grid.RemoveBloc (posX, posY);
}


// End
