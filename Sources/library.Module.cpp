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
/// \file Library.Module.cpp
///
/// \brief An independent module for solving grids
// ###########################################################################

#include "library.Module.h"


// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief	Constructor
// ===========================================================================
Library::Module::Module (const Config& config) : dictionary (config.alphabetSize, config.maxWordLength)
{
	alphabetSize = config.alphabetSize;
	maxWordLength = config.maxWordLength;
	currentSolver = &solverDyn;
}


// ===========================================================================
/// \brief	Destructor 
// ===========================================================================
Library::Module::~Module ()
{

}


// ===========================================================================
/// \brief	Return a solver configured as requested
///
/// \parma	config		Desired configuration
///
/// \return	The corresponding solver
// ===========================================================================
ISolver& Library::Module::GetSolver (const SolverConfig& config)
{
	// Configure the static solver
	if (config.maxBlackBoxes == 0)
	{
		this->solverStat.SetSeed (config.seed);

		if (config.heuristicLevel >= 0)
			this->solverStat.SetHeurestic (true, config.heuristicLevel);
		else
			this->solverStat.SetHeurestic (false, 0);

		this->currentSolver = &this->solverStat;
		return this->solverStat;
	}

	// Configure the dynamic solver
	else
	{
		this->solverDyn.SetSeed (config.seed);

		if (config.heuristicLevel >= 0)
			this->solverDyn.SetHeurestic (true, config.heuristicLevel);
		else
			this->solverDyn.SetHeurestic (false, 0);

		this->solverDyn.SetMaxBlackCases (config.maxBlackBoxes);
		switch (config.blackMode)
		{
		default:
		case BlackMode::ANY:
			this->solverDyn.SetBlackCasesDensity (Grid::BlocDensityMode::ANY); break;
		case BlackMode::DIAGONAL:
			this->solverDyn.SetBlackCasesDensity (Grid::BlocDensityMode::DIAG); break;
		case BlackMode::SINGLE:
			this->solverDyn.SetBlackCasesDensity (Grid::BlocDensityMode::SINGLE); break;
		case BlackMode::TWO:
			this->solverDyn.SetBlackCasesDensity (Grid::BlocDensityMode::TWO); break;
		}

		this->currentSolver = &this->solverDyn;
		return this->solverDyn;
	}
}

