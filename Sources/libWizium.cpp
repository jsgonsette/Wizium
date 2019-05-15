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
/// \file		libWizium.cpp
/// \author		Jean-Sebastien Gonsette
///
/// \brief		libWizium API entry points
// ###########################################################################

#include <stdlib.h>
#include <time.h> 
#include "libWizium.h"
#include "library.h"


// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief	Initialize this library
///
/// \param[out]	version	Version of this library
// ===========================================================================
void WIZ_Init (Version &version)
{
	version.major = VER_MAJOR;
	version.minor = VER_MINOR;
	version.release = VER_RELEASE;
}


// ===========================================================================
/// \brief	Create a sperate application instance
///
/// \param	config		Instance configuration options
/// \return	LibHandle on the instance
// ===========================================================================
LibHandle WIZ_CreateInstance (const Config& config)
{
	Library::Module *module;

	module = Library::GetInstance ().CreateInstance (config);
	return reinterpret_cast<LibHandle> (module);
}


// ===========================================================================
/// \brief	Destroy an application instance
///
/// \param	instance	Instance to destroy
// ===========================================================================
void WIZ_DestroyInstance (LibHandle instance)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);

	Library::GetInstance ().DestroyInstance (module);
}


// ===========================================================================
/// \brief	Flush the dictionary content
///
/// \param	instance		Instance to flush
// ===========================================================================
void DIC_Clear (LibHandle instance)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);

	Library::GetInstance ().ClearDictionary (module);
}


// ===========================================================================
/// \brief	Add words to the dictionary
///
/// \param		instance		Target module
/// \param		entries			Array of word entries to add to the dictionary
/// \param		numEntries		Number of words in the list
///
/// \return		Number of words added to the dictionary
// ===========================================================================
int32_t DIC_AddEntries (LibHandle instance, const uint8_t entries [], int32_t numEntries)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);

	return Library::GetInstance ().AddDictionaryEntries (module, entries, -1, numEntries);
}


// ===========================================================================
/// \brief	Find a word matching a mask, as part of an interative procedure
///
/// \param		instance	Target Instance
/// \param[out]	result		Array to write the matching word (must be as long as the mask length)
/// \param		mask		Mask enabling to force some letters. 
///							Each letter is either in the [A..Z] ASCII range or in the [1..alphabetSize] range
///							Exception for '*' which means any letter (e.g. "*A***I**)
///							Word length to retrieve is given implicitly by the mask length.
/// \param		startWord	Word to start searching from in the dictionary. If empty, start begins at the fist dictionary word.
///
/// \return		True if a match has been found
// ===========================================================================
bool DIC_FindEntry (LibHandle instance, uint8_t result [], const uint8_t mask [], const uint8_t startWord [])
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);

	return Library::GetInstance ().FindDictionaryEntry (module, result, mask, startWord);
}


// ===========================================================================
/// \brief	Find a random word matching a mask
///
/// \param		instance	Target Instance
/// \param[out]	result		Array to write the matching word (must be as long as the mask length)
/// \param		mask		Mask enabling to force some letters. 
///							Each letter is either in the [A..Z] ASCII range or in the [1..alphabetSize] range
///							Exception for '*' which means any letter (e.g. "*A***I**)
///							Word length to retrieve is given implicitly by the mask length.
///
/// \return		True if a match has been found
// ===========================================================================
bool DIC_FindRandomEntry (LibHandle instance, uint8_t result [], const uint8_t mask [])
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);

	return Library::GetInstance ().FindRandomDictionaryEntry (module, result, mask);
}


// ===========================================================================
/// \brief	Return the number of words in the dictionary
///
/// \param		instance	Target Instance
// ===========================================================================
uint32_t DIC_GetNumWords (LibHandle instance)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	return Library::GetInstance ().GetNumDictionaryWords (module);
}


// ===========================================================================
/// \brief	Change the size of the grid
///
/// \param	instance			Target Instance
/// \param	width, height		New grid size
// ===========================================================================
void GRID_SetSize (LibHandle instance, uint8_t width, uint8_t height)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	Library::GetInstance ().SetGridSize (module, width, height);
}


// ===========================================================================
/// \brief	Change one grid box type
///
/// \param	instance	Target Instance
/// \param	x, y		Grid box coordinates
/// \param	type		New box type
// ===========================================================================
void GRID_SetBox (LibHandle instance, uint8_t x, uint8_t y, BoxType type)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	Library::GetInstance ().SetGridBox (module, x, y, type);
}


// ===========================================================================
/// \brief	Write a word on the grid
///
/// \param	instance			Target Instance
/// \param	x, y				Box coordinate of the word first letter
/// \param	entry				Word to write on the grid
/// \param	dir					'H' or 'V' to layout the word Horizontally or Vertically
/// \param	terminator			True to add a black box at the end of the word
// ===========================================================================
void GRID_Write (LibHandle instance, uint8_t x, uint8_t y, const uint8_t entry[], char dir, bool terminator)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	Library::GetInstance ().WriteGrid (module, x, y, entry, dir, terminator);
}


// ===========================================================================
/// \brief	Read the whole grid content
///
/// When using the standard 26-letters alphabet, the word content
///	is returned in ASCII; '.' for empty box. 
/// Otherwise, the letters are in the range [1..alphabetSize]; ZERO for empty box.
/// In any case, black boxes and void boxes are given the '*' and '-' values.
///
/// \param		instance	Target Instance
/// \param[out]	grid		Buffer to get the grid content. MUST be big enough
///							to hold (WIDTH x HEIGHT) values.
// ===========================================================================
void GRID_Read (LibHandle instance, uint8_t grid[])
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	Library::GetInstance ().ReadGrid (module, grid);
}


// ===========================================================================
/// \brief	Erase the whole grid content
///
/// \param	instance			Target Instance
// ===========================================================================
void GRID_Erase (LibHandle instance)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	Library::GetInstance ().EraseGrid (module);
}


// ===========================================================================
/// \brief	Start the grid generation process
///
/// \param	instance			Target Instance
/// \param	sovlerConfg			Solver configuration parameters
// ===========================================================================
void SOLVER_Start (LibHandle instance, const SolverConfig& solverConfig)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	Library::GetInstance ().SolverStart (module, solverConfig);
}


// ===========================================================================
/// \brief	Continue the grid generation process.
///
/// This function can be called iteratively until the generation fail or succeed.
/// Time and step criterias enable to limit the time spent in the function during a single call.
///
/// \param	instance			Target Instance
/// \param		maxTimeMs:		>=0: Maximum time to spend [ms] before returning.
///								-1: No stop criteria
/// \param		maxSteps:		>=0: Maximum word tries before returning
///								-1: No stop criteria
/// \param[out]	status:			Generation status
// ===========================================================================
void SOLVER_Step (LibHandle instance, int32_t maxTimeMs, int32_t maxSteps, Status& status)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	status = Library::GetInstance ().SolverStep (module, maxTimeMs, maxSteps);
}


// ===========================================================================
/// \brief	Stop the grid generation process
// ===========================================================================
void SOLVER_Stop (LibHandle instance)
{
	Library::Module *module;
	module = reinterpret_cast<Library::Module*> (instance);
	Library::GetInstance ().SolverStop (module);
}

// End
