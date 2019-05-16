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
/// \file		Grid.cpp
/// \author		Jean-Sebastien Gonsette
///
/// \brief		Crossword grid
// ###########################################################################

#include "Grid.h"
#include <stdio.h>


// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief		Initialize an empty, null size grid
// ===========================================================================
Grid::Grid ()
{
	mpTabCases = nullptr;
	mSx = mSy = 0;
	densityMode = DIAG;
	numBlackCases = 0;
}


// ===========================================================================
/// \brief		Destructor
// ===========================================================================
Grid::~Grid ()
{
	if (mpTabCases != nullptr) delete [] mpTabCases;
}


// ===========================================================================
/// \brief		Return the content of the grid at a given location
///
/// \param		x	Grid horizontal index
/// \param		y	Grid vertical index
///
/// \return		Pointer to the grid box
// ===========================================================================
inline Box* Grid::operator () (int x, int y)
{
	if (y < 0 || y >= mSy) return nullptr;
	if (x < 0 || x >= mSx) return nullptr;

	return &mpTabCases [y*mSx + x];
}


// ===========================================================================
/// \brief		Return the content of the grid at a given location
///
/// \param		x	Grid horizontal index
/// \param		y	Grid vertical index
///
/// \return		Pointer to the grid box
// ===========================================================================
inline const Box* Grid::operator () (int x, int y) const
{
	if (y < 0 || y >= mSy) return nullptr;
	if (x < 0 || x >= mSx) return nullptr;

	return &mpTabCases [y*mSx + x];
}


// ===========================================================================
/// \brief		Change the size of the grid. Content is lost.
///
/// \param		sx	New grid horizontal size
/// \param		sy	New grid vertical size

// ===========================================================================
void Grid::Grow (uint8_t sx, uint8_t sy)
{
	if (mpTabCases != nullptr) delete [] mpTabCases;
	mpTabCases = new Box [sx*sy];

	mSx = sx;
	mSy = sy;
}


// ===========================================================================
/// \brief		Erase the grid content, but the protected boxes
// ===========================================================================
void Grid::Erase ()
{
	int i, j;
	numBlackCases = 0;
	numVoidBoxes = 0;

	for (j = 0; j < mSy; j ++)
	{
		for (i = 0; i < mSx; i ++)
		{
			Box* box = this->operator ()(i,j);
			box->MakeLetter ();
			if (box->IsBloc ()) numBlackCases ++;
			if (box->IsVoid ()) numVoidBoxes ++;
		}
	}
}


// ===========================================================================
/// \brief		Lock the current content of the grid
// ===========================================================================
void Grid::LockContent ()
{
	int x, y;
	int count = 0;
	numBlackCases = 0;
	numVoidBoxes = 0;

	for (y = 0; y < mSy; y ++)
	{
		for (x = 0; x < mSx; x ++)
		{
			Box* box = this->operator ()(x, y);

			// Lock box if it contains something
			if (box->IsLetter () == false || box->GetLetter () != 0)
			{
				box->Lock (true);
				if (box->IsBloc ()) numBlackCases ++;
				if (box->IsVoid ()) numVoidBoxes ++;
			}

			// Otherwise, put number of non locked previous boxes in the 'tag' field
			else
			{
				box->Lock (false);
				box->tag = count;
				count ++;
			}
		}
	}
}



// ===========================================================================
/// \brief		Unlock every box
// ===========================================================================
void Grid::Unlock ()
{
	int x, y;

	for (y = 0; y < mSy; y ++)
	{
		for (x = 0; x < mSx; x ++)
		{
			Box* box = this->operator ()(x, y);
			box->Lock (false);
		}
	}
}


// ===========================================================================
/// \brief		Debug print the grid content in the console
// ===========================================================================
void Grid::Draw ()
{
	printf ("\n\nGrille : \n\n ");

	for (int j = 0; j < mSy; j ++)
	{
		for (int i = 0; i < mSx; i ++)
		{
			if ( this->operator ()(i,j)->IsBloc ()) printf ("#  ");
			if ( this->operator ()(i,j)->IsVoid ()) printf ("   ");
			if ( this->operator ()(i,j)->IsLetter ())
			{
				uint8_t c = (this->operator ()(i,j))->GetLetter ();
				if (c != 0) printf ("%c  ", c + 'A' - 1);
				else printf (".  ");
			}
		}

		printf ("\n\n ");
	}
}


// ===========================================================================
/// \brief		Add a black box to the grid.
///
/// A counter keeps track of the number of times a black box has been added
/// at a given location. 'RemoveBloc' must be called the same number of time
/// to indeed remove the black box.
///
/// \param		x		Black box horizontal coordinate
/// \param		y		Black box vertical coordinate
// ===========================================================================
void Grid::AddBloc (uint8_t x, uint8_t y)
{
	int i;
	unsigned char v;

	int tabX [] = {-1, 0, 1, -1, 1, -1,  0,  1};
	int tabY [] = { 1, 1, 1,  0, 0, -1, -1, -1};

	// Target box
	Box* box  = this->operator () (x, y);
	if (box->IsLocked ()) return;
		
	// Turn the box into a black box
	if (box->IsBloc () == false)
	{
		box->MakeBloc ();
		box->ResetCounter (1);
		numBlackCases ++;
		
		// Update neighbourhood density and count number of nieghboors
		v = 0;

		for (i = 0; i < 8; i ++)
		{
			Box* boxNext = this->operator () (x + tabX [i], y + tabY [i]);
			if (boxNext != nullptr && boxNext->IsBloc ())
			{
				boxNext->SetBlocDensity (boxNext->GetBlocDensity () +1);
				v ++; 
			}
		}

		// Update target box density
		box->SetBlocDensity (v);
	}

	// Or increment the box counter
	else box->IncrementCounter ();

}


// ===========================================================================
/// \brief		Remove a black box from the grid.
///
/// A counter keeps track of the number of times a black box has been added
/// at a given location. 'RemoveBloc' must be called the same number of time
/// to indeed remove the black box.
///
/// \param		x		Black box horizontal coordinate
/// \param		y		Black box vertical coordinate
// ===========================================================================
void Grid::RemoveBloc (uint8_t x, uint8_t y)
{
	int i;
	int tabX [] = {-1, 0, 1, -1, 1, -1,  0,  1};
	int tabY [] = { 1, 1, 1,  0, 0, -1, -1, -1};
	
	// Target box
	Box* box = this->operator () (x, y);
	if (box->IsLocked ()) return;

	// Untag the box
	if (box->GetCounter () > 1)
		box->DecrementCounter ();
	else
	{
		box->MakeLetter ();
		numBlackCases --;
				
		// Update neighbours density
		for (i = 0; i < 8; i ++)
		{
			Box* boxNext = this->operator () (x + tabX [i], y + tabY [i]);
			if (boxNext != nullptr && boxNext->IsBloc ())
				boxNext->SetBlocDensity (boxNext->GetBlocDensity () -1);
		}
	}	
}


// ===========================================================================
/// \brief		Place a word on the grid, along with a black box at the end.
///
/// \param		x		Word horizontal coordinate (first letter)
/// \param		y		Word vertical coordinate (first letter)
/// \param		dir		'H': Horizontal placement, 'V' vertical placement
/// \param		word	Pointer to the word
// ===========================================================================
void Grid::AddWord (uint8_t x, uint8_t y, char dir, const uint8_t* word)
{
	unsigned char i;
	Box *box;
	
	i = 0;
	do
	{
		if (dir == 'H') box = this->operator () (x + i, y);
		else box = this->operator () (x, y + i);

		// Out of grid
		if (box == nullptr) break;

		// Increment the box counter or write the letter
		if (word [i] != 0)
		{
			if (box->GetLetter () == word [i])
				box->IncrementCounter ();
			else 
			{
				box->SetLetter (word [i]);
				box->ResetCounter (1);
			}
		}
		// Put a black box
		else 
		{
			if (dir == 'H')	AddBloc (x + i, y);
			else AddBloc (x, y + i);
		}
	}
	while (word [i ++] != 0);
}


// ===========================================================================
/// \brief		Remove a word from the grid, along with the black box at the end.
///
/// \param		x		Word horizontal coordinate (first letter)
/// \param		y		Word vertical coordinate (first letter)
/// \param		dir		'H': Horizontal placement, 'V' vertical placement
// ===========================================================================
void Grid::RemoveWord (uint8_t x, uint8_t y, char dir)
{
	unsigned char i;
	Box *box;

	i = 0;
	do
	{
		// Get next letter
		if (dir == 'H')	box = this->operator () (x + i, y);
		else box = this->operator () (x, y + i);

		// Out of grid ?
		if (box == nullptr) break;

		// Decrement the box counter, or remove the letter
		if (box->IsLetter ())
		{
			if (box->GetCounter () > 1)
				box->DecrementCounter ();
			else 
			{
				box->SetLetter (0);
				box->ResetCounter (1);
			}
		}

		// Remove the ending bloc and stop
		else 
		{
			if (dir == 'H')	RemoveBloc (x + i, y);
			else RemoveBloc (x, y + i);
			break;
		}
		
		i ++;
	}
	while (true);
}


// ===========================================================================
/// \brief		Check if it is possible to add a black box at a given location
///				given the grid black box density
///
/// \param		x		Grid horizontal location
/// \param		y		Grid vertical location
///
/// \return		True if a black box can be added.
// ===========================================================================
bool Grid::CheckBlocDensity (uint8_t x, uint8_t y) const
{
	const Box *box = this->operator () (x, y);

	int tabX [] = {-1, 0, 1, -1, 1, -1,  0,  1};
	int tabY [] = { 1, 1, 1,  0, 0, -1, -1, -1};
	bool good = true;
	
	// Trivial cases if the box is not empty
	if (box == nullptr) return false;
	if (box->IsBloc () == true) return true;
	if (box->IsLetter () && box->GetLetter () != 0) return false;

	// No black box allowed:
	if (densityMode == NONE)
	{
		// OK if the black box was already there and is locked
		if (box->IsLocked ()) good = box->IsBloc ();
		else good = false;
	}

	// No neighbours:
	else if (densityMode == SINGLE)
	{
		// Check all neighbours
		for (int j = 0; j < 8; j ++)
		{
			box = this->operator () (x + tabX [j], y + tabY [j]);
			if (box != nullptr && box->IsBloc () && box->IsLocked () == false)
			{
				good = false;
				break;
			}
		}
	}

	// Neighbours must be in diagonal
	else if (densityMode == DIAG)
	{
		for (int j = 0; j < 8; j ++)
		{
			box = this->operator () (x+ tabX [j], y + tabY [j]);
			if (box != nullptr && box->IsBloc () && box->IsLocked () == false) 
			{
				if (j == 1 || j == 3 || j == 4 || j == 6) {
					good = false;
					break;
				}
			}
		}
	}

	// Only two neighbours
	else if (densityMode == TWO)
	{
		int c = 0;
		for (int j = 0; j < 8; j ++)
		{
			box = this->operator () (x + tabX [j], y + tabY [j]);
			if (box != nullptr && box->IsBloc () && box->IsLocked () == false) 
			{
				c ++;
				if (c > 2) {
					good = false;
					break;
				}

				if (box->GetBlocDensity () >= 2) {
					good = false;
					break;
				}
			}
		}
	}

	// Check for this unwanted diamond pattern:
	// . . * . .
	// . * . * .
	// . . * . .
	if (good)
	{
		// Black box at the four corners around current case
		bool p1, p2, p3, p4, pc;

		box = this->operator () (x-1, y-1 );
		p1 = (box == nullptr || box->IsBloc ());

		box = this->operator () (x+1, y-1 );
		p2 = (box == nullptr || box->IsBloc ());

		box = this->operator () (x+1, y+1 );
		p3 = (box == nullptr || box->IsBloc ());

		box = this->operator () (x-1, y+1 );
		p4 = (box == nullptr || box->IsBloc ());

		// Check we don't close the pattern from the bottom
		if (p1 && p2 && y >= 1) {
			box = this->operator () (x, y-1 );
			pc = (box == nullptr || box->IsBloc ());

			box = this->operator () (x, y-2 );
			if (pc == false && (box == nullptr || box->IsBloc ()) ) good = false;
		}
		// Check we don't close the pattern from the right
		if (p1 && p4 && x >= 1) {
			box = this->operator () (x-1, y );
			pc = (box == nullptr || box->IsBloc ());

			box = this->operator () (x-2, y );
			if (pc == false && (box == nullptr || box->IsBloc ()) ) good = false;
		}
		// Check we don't close the pattern from the left
		if (p2 && p3 && x < mSx -1) {
			box = this->operator () (x+1, y );
			pc = (box == nullptr || box->IsBloc ());

			box = this->operator () (x+2, y );
			if (pc == false && (box == nullptr || box->IsBloc ()) ) good = false;
		}
		// Check we don't close the pattern from the top
		if (p3 && p4 && y < mSy -1) {
			box = this->operator () (x, y+1 );
			pc = (box == nullptr || box->IsBloc ());

			box = this->operator () (x, y+2 );
			if (pc == false && (box == nullptr || box->IsBloc ()) ) good = false;
		}
	}

	return good;
}


// ===========================================================================
/// \brief		Build a mask of the words that can be placed on a given
///				grid location.
///
/// \param[out]		mask		Word mask, '*' means any character (e.g. "AB*U*")
///								Must be as long a the max grid size +1
/// \param			x			Grid horizontal location for mask construction
/// \param			y			Grid vertical location for mask construction
/// \param			dir			'V': mask built vertically
///								'H': mask built horizontally
/// \param			goBack		True, go back (vertically or horizontally) as far
///								as possible before mask construction
///
/// \return		Number of boxes moved back before mask construction ('goBack' is True), or 0.
// ===========================================================================
unsigned char Grid::BuildMask (uint8_t mask [], uint8_t x, uint8_t y, char dir, bool goBack) const
{
	int i = 0;
	unsigned char offset = 0;
	const Box *box;

	// If we have to go back first
	if (goBack)
	{
		// Still possible to go back ?
		while (dir == 'H' && x > 0 || dir == 'V' && y > 0)
		{
			// Go back
			if (dir == 'H') x --;
			else y --;

			// We got too far ?
			box = this->operator ()(x, y);
			if (box->IsBloc () || box->IsVoid ())
			{
				if (dir == 'H') x ++;
				else y ++;
				break;
			}

			offset++;
		}
	}

	// Move on to build the mask
	while (true)
	{
		box = this->operator ()(x, y);
		if (box == nullptr) break;
		if (box->IsBloc () || box->IsVoid ()) break;

		mask [i] = box->GetLetter ();
		if (mask [i] == 0) mask[i] = '*';
		
		if (dir == 'V') y ++;
		else x ++;

		i ++;			
	}	

	mask [i] = 0;
	return offset;
}


// ===========================================================================
/// \brief		Return number of normal grid boxes (letters) around a given grid location
///
///				Grid location can be adjacent to the grid (e.g. x=-1 or x=grid width)
///
/// \param		x,y		Grid location
// ===========================================================================
Grid::Space Grid::GetSpace (int x, int y) const
{
	const Box *box;
	int tabSpace [] = {0,0,0,0};

	int px, py;
	int dirX [] = {0,1,0,-1};
	int dirY [] = {1,0,-1,0};
	
	// Check along 4 directions
	for (int i = 0; i < 4; i ++)
	{
		px = x;
		py = y;

		do
		{
			px += dirX [i];
			py += dirY [i];
			
			box = this->operator ()(px, py);
			if (box == nullptr || box->IsBloc () || box->IsVoid () ) break;

			tabSpace [i] ++;
		}
		while (true);
	}

	// Return result
	Space space;
	space.left = tabSpace [3];
	space.right = tabSpace [1];
	space.top = tabSpace [2];
	space.bottom = tabSpace [0];
	return space;
}


// ===========================================================================
/// \brief		Return number non empty boxes, as a percentage
///
/// \return		Fill rate [%]
// ===========================================================================
int Grid::GetFillRate () const
{
	int notVoid = 0;
	int numVoid = 0;

	for (int j = 0; j < mSy; j ++)
	{
		for (int i = 0; i < mSx; i ++)
		{
			if ( this->operator ()(i,j)->IsBloc ()) notVoid ++;
			else if ( this->operator ()(i,j)->IsLetter ())
			{
				uint8_t c = (this->operator ()(i,j))->GetLetter ();
				if (c != 0) notVoid ++;
			}
			else numVoid ++;
		}
	}

	return (int) (100 * notVoid / (mSx*mSy - numVoid));
}

// End
