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
/// \file		ISolver.h
/// \author		Jean-Sebastien Gonsette
// ###########################################################################

#ifndef __CSOLVER__H
#define __CSOLVER__H

#include "libWizium.h"
#include "Grid/Grid.h"
#include "Dictionary/Dictionary.h"


// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

class ISolver
{
public:

	ISolver() : pGrid (nullptr), pDict (nullptr), seed (0), mSx (0), mSy (0), steps (0) {}
	virtual ~ISolver() {}

	virtual void Solve_Start (Grid &grid, const Dictionary &dico) = 0;
	virtual Status Solve_Step (int32_t maxTimeMs, int32_t maxSteps) = 0;
	virtual void Solve_Stop () = 0;

	virtual void SetHeurestic (bool state, int param) = 0;
	virtual void SetSeed (uint64_t seed) {this->seed = seed;}

protected:

	
	Grid *pGrid;				///< Grid to solve
	const Dictionary *pDict;	///< Dictionary to use

	uint64_t seed;		///< Seed for the random generator	
	uint8_t mSx, mSy;	///< Grid size
	uint64_t steps;		///< Number of steps during the generation
};


#endif


