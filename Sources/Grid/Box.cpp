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
/// \file		Box.cpp
/// \author		Jean-Sebastien Gonsette
///
/// \brief		A single box of a crossword grid
// ###########################################################################

#include "libWizium.h"
#include "Box.h"



// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief		Init the grid box as an empty letter
// ===========================================================================
Box::Box ()
{
	// Empty letter
	mType = 'L';
	mValue = 0;
	mCounter = 0;
	mLocked = false;
	tag = 0;

	// All letters are valid candidates
	ResetCandidates (true);
}


// ===========================================================================
/// \brief		Turn the box into a bloc
// ===========================================================================
void Box::MakeBloc ()
{
	if (mLocked == true) return;
	mType = 'B';
	mValue = 0;
	mCounter = 0;
}


// ===========================================================================
/// \brief		Turn the box into a letter
// ===========================================================================
void Box::MakeLetter()
{
	if (mLocked == true) return;
	mType = 'L';
	mValue = 0;
	mCounter = 0;
}


// ===========================================================================
/// \brief		Turn the box into void
// ===========================================================================
void Box::MakeVoid ()
{
	if (mLocked == true) return;
	mType = 'V';
	mValue = 0;
	mCounter = 0;
}


// ===========================================================================
/// \brief		Return the letter stored in the box.
///
/// \return		Embedded letter. 0 if box is not a letter box.
// ===========================================================================
uint8_t Box::GetLetter () const
{
	if (mType != 'L') return 0;
	return mValue;
}


// ===========================================================================
/// \brief		Write a letter in the box
///
/// \param		c	>0: Letter to write in the box. 0: To erase the letter.
// ===========================================================================
void Box::SetLetter (uint8_t c)
{
	if (mLocked == true) return;
	if (mType != 'L') return;
	
	mValue = c;
}


// End
