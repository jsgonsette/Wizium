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
	type = 'L';
	value = 0;
	counter = 0;
	failCounter = 0;
	isLocked = false;
	tag = 0;

	// All letters are valid candidates
	ResetCandidates (true);
}


// ===========================================================================
/// \brief		Turn the box into a bloc
// ===========================================================================
void Box::MakeBloc ()
{
	if (isLocked == true) return;
	type = 'B';
	value = 0;
	counter = 0;
}


// ===========================================================================
/// \brief		Turn the box into a letter
// ===========================================================================
void Box::MakeLetter()
{
	if (isLocked == true) return;
	type = 'L';
	value = 0;
	counter = 0;
}


// ===========================================================================
/// \brief		Turn the box into void
// ===========================================================================
void Box::MakeVoid ()
{
	if (isLocked == true) return;
	type = 'V';
	value = 0;
	counter = 0;
}


// ===========================================================================
/// \brief		Return the letter stored in the box.
///
/// \return		Embedded letter. 0 if box is not a letter box.
// ===========================================================================
uint8_t Box::GetLetter () const
{
	if (type != 'L') return 0;
	return value;
}


// ===========================================================================
/// \brief		Write a letter in the box
///
/// \param		c	>0: Letter to write in the box. 0: To erase the letter.
// ===========================================================================
void Box::SetLetter (uint8_t c)
{
	if (isLocked == true) return;
	if (type != 'L') return;
	
	value = c;
}


// End
