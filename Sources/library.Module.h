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
/// \file Library.Module.h
///
/// \brief An independent module for solving grids
// ###########################################################################

#ifndef LIBRARY_MODULE_H
#define LIBRARY_MODULEH

#include "library.h"
#include "Grid/Grid.h"
#include "Dictionary/Dictionary.h"
#include "Solvers/SolverDynamic.h"
#include "Solvers/SolverStatic.h"


// ###########################################################################
//
// T Y P E S
//
// ###########################################################################

class Library::Module
{
	friend class Library;

public:

	Module (const Config& config);
	~Module ();

	Module (const Module&) = delete;
	Module& operator = (const Module&) = delete;

	Dictionary& GetDictionary () { return dictionary; }
	const Dictionary& GetDictionary () const { return dictionary; }

	Grid& GetGrid () { return grid; }
	const Grid& GetGrid () const { return grid; }

	ISolver& GetSolver (const SolverConfig& config);
	ISolver& GetSolver () { return *currentSolver; }

private:

	Grid grid;					///< The grid we work on
	Dictionary dictionary;		///< The dictionary we work with
	SolverDynamic solverDyn;	///< The dynamic solver (can add black boxes)
	SolverStatic solverStat;	///< The static solver (no black box addition)
	
	int32_t alphabetSize;		///< Alphabet size we work with
	int32_t maxWordLength;		///< Max word length

	ISolver* currentSolver;		///< Pointer to current solver

	mutable const Module *next;			///< For modules chaining

private:


};

#endif

