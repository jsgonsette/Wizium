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
/// \file		Grid.h
/// \author		Jean-Sebastien Gonsette
// ###########################################################################

#ifndef __GRID__H
#define __GRID__H

#include "Box.h"


// ===========================================================================
// D E F I N E S
// ===========================================================================

constexpr auto MAX_GRID_SIZE = 256;


// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

/// Crossword grid
class Grid
{
public:
	
	/// Available free space around a position
	struct Space
	{
		int left, bottom;
		int right, top;
	};

public :

	/// Type of block density allowed in this grid
	enum BlocDensityMode {NONE, SINGLE, DIAG, TWO, ANY};

	Grid ();
	~Grid ();

	Box* operator () (int x, int y);
	const Box* operator () (int x, int y) const;

	void Grow (uint8_t sx, uint8_t sy);
	void Draw ();
	void Erase ();
	void LockContent ();
	void Unlock ();

	uint8_t GetWidth () const {return static_cast<uint8_t>(mSx);}
	uint8_t GetHeight () const { return static_cast<uint8_t>(mSy); }
	void SetDensityMode (BlocDensityMode density) { this->densityMode = density; }

	int GetNumBlackCases () const {return numBlackCases;}
	int GetNumVoidBoxes () const { return numVoidBoxes; }
	int GetFillRate () const;

	void AddBloc (uint8_t x, uint8_t y);
	void RemoveBloc (uint8_t x, uint8_t y);
	void AddWord (uint8_t x, uint8_t y, char dir, const uint8_t* word);
	void RemoveWord (uint8_t x, uint8_t y, char dir);
    
	bool CheckBlocDensity (uint8_t x, uint8_t y) const;
	unsigned char BuildMask (uint8_t mask [], uint8_t x, uint8_t y, char dir, bool goBack) const;	
	Space GetSpace (int x, int y) const;

private :
				

	int mSx, mSy;						///< Grid dimensions
	Box *mpTabCases;					///< Array of boxes
	
	enum BlocDensityMode densityMode;	///< Allowed bloc density
	int numBlackCases;					///< Total number of black boxes
	int numVoidBoxes;					///< Total number of void boxes
};


#endif
