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
/// \file		Library.cpp
/// \author		Jean-Sebastien Gonsette
///
/// \brief	Singleton holding all the Wizium instances
// ###########################################################################

#include <assert.h>
#include "library.h"
#include "library.Module.h"


// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

/// Library singleton instance
Library Library::s_library = Library ();


// ===========================================================================
/// \brief	Return Singleton unique instance
// ===========================================================================
Library& Library::GetInstance ()
{
	return s_library;
}


// ===========================================================================
/// \brief	Create and manage a new PPMM instance
///
/// \param	config	Instance configuration options
/// \return	New managed instance
// ===========================================================================
Library::Module* Library::CreateInstance (const Config& config)
{
	Module* module;

	// Create the module
	module = new Module (config);

	// Chaining
	module->next = this->modules;
	this->modules = module;

	return module;
}


// ===========================================================================
/// \brief	Destroy an existing instance
///
/// \param	instance	Instance to remove and destroy
// ===========================================================================
void Library::DestroyInstance (Module* module)
{
	const Module* p = this->modules;
	while (p && p != module) p = p->next;

	assert (p != nullptr);
	if (p) delete module;
}


// ===========================================================================
/// \brief	Destructor
// ===========================================================================
Library::~Library ()
{
	// delete all modules
	const Module* p = this->modules;
	while (p != nullptr)
	{
		const Module*pn = p->next;
		delete p;
		p = pn;
	}
}


// ===========================================================================
/// \brief	Flush dictionary content
// ===========================================================================
void Library::ClearDictionary (Module* module)
{
	module->GetDictionary ().Clear ();
}


// ===========================================================================
/// \brief	Add words to the dictionary
///
/// \param		module			Target module
/// \param		tabEntries		List of words. End of list when a null caracter is 
///								found instead of a new word or if 'numWords' is reached
///								Each word is either in the [A..Z] ASCII range or in the [1..alphabetSize] range
/// \param		entrySize		>0: (static) size of every word in the list
///								=0: (dynamic) every word is ZERO terminated. 
///								-1: Use the static value given in configuration
/// \param		numWords		Number of words in the list
///
/// \return		Number of words added to the dictionary
// ===========================================================================
int32_t Library::AddDictionaryEntries (Module* module, const uint8_t* tabEntries, int32_t entrySize, int32_t numWords)
{
	// Fall back on max word length if entry size is not specified
	if (entrySize < 0) entrySize = module->maxWordLength;

	return module->GetDictionary ().AddEntries (tabEntries, entrySize, numWords);
}


// ===========================================================================
/// \brief	Find a word matching a mask, with a given starting point.
///			This function can be called iteratively to enumerate all the words mathcing a mask.
///
/// \param		module		Target module
/// \param[out]	result		Array to write the matching word (must be as long as the mask length)
/// \param		mask		Mask enabling to force some letters. 
///							Each letter is either in the [A..Z] ASCII range or in the [1..alphabetSize] range
///							Exception for '*' which means any letter (e.g. "*A***I**)
///							Word length to retrieve is given implicitly by the mask length.
/// \param		startWord	Word to start searching from in the dictionary. If empty, start begins at the fist dictionary word.
///
/// \return		True if a match has been found
// ===========================================================================
bool Library::FindDictionaryEntry (Module* module, uint8_t* result, const uint8_t* mask, const uint8_t* startWord) const
{
	const Dictionary& dico = module->GetDictionary ();
	if (dico.FindEntry (result, mask, startWord))
	{
		if (dico.AlphabetSize () == 26)
		{
			int len = 0;
			while (result [len] != 0) result [len++] += 'A' - 1;
		}
		return true;
	}
	else return false;
}


// ===========================================================================
/// \brief	Find a random word matching a mask
///
/// \param		module		Target module
/// \param[out]	result		Array to write the matching word (must be as long as the mask length)
/// \param		mask		Mask enabling to force some letters. 
///							Each letter is either in the [A..Z] ASCII range or in the [1..alphabetSize] range
///							Exception for '*' which means any letter (e.g. "*A***I**)
///							Word length to retrieve is given implicitly by the mask length.
///
/// \return		True if a match has been found
// ===========================================================================
bool Library::FindRandomDictionaryEntry (Module* module, uint8_t* result, const uint8_t* mask) const
{
	const Dictionary& dico = module->GetDictionary ();
	if (dico.FindRandomEntry (result, mask))
	{
		if (dico.AlphabetSize () == 26)
		{
			int len = 0;
			while (result [len] != 0) result [len++] += 'A' - 1;
		}
		return true;
	}
	else return false;
}


// ===========================================================================
/// \brief	Return the number of words in the dictionary
///
/// \param		module		Target module
// ===========================================================================
uint32_t Library::GetNumDictionaryWords (Module* module) const
{
	return module->GetDictionary ().GetNumWords ();
}


// ===========================================================================
/// \brief	Change the size of the grid
///
/// \param		module				Target module
/// \param		width, height		New grid size
// ===========================================================================
void Library::SetGridSize (Module* module, uint8_t width, uint8_t height)
{
	return module->GetGrid ().Grow (width, height);
}


// ===========================================================================
/// \brief	Change one grid box type
///
/// \param	module		Target module
/// \param	x, y		Grid box coordinates
/// \param	type		New box type
// ===========================================================================
void Library::SetGridBox (Module* module, uint8_t x, uint8_t y, BoxType type)
{
	if (x >= module->GetGrid ().GetWidth ()) return;
	if (y >= module->GetGrid ().GetHeight ()) return;

	switch (type)
	{
	case BoxType::LETTER:
		module->GetGrid () (x, y)->MakeLetter ();
		break;
	
	case BoxType::BLACK:
		module->GetGrid () (x, y)->MakeBloc ();
		break;
	
	case BoxType::VOID:
		module->GetGrid () (x, y)->MakeVoid ();
		break;
	}	
}


// ===========================================================================
/// \brief	Write a word on the grid
///
/// \param	module				Target module
/// \param	x, y				Box coordinate of the word first letter
/// \param	entry				Word to write on the grid
/// \param	dir					'H' or 'V' to layout the word Horizontally or Vertically
// ===========================================================================
void Library::WriteGrid (Module* module, uint8_t x, uint8_t y, const uint8_t entry[], char dir, bool terminator)
{
	const Dictionary& dico = module->GetDictionary ();
	Grid& mgrid = module->GetGrid ();
	int stepx = 0;
	int stepy = 0;
	int lim;

	// Fix limit and orientation
	if (dir == 'H')
	{
		stepx = 1;
		lim = mgrid.GetWidth () - x;
	}
	else
	{
		stepy = 1;
		lim = mgrid.GetHeight () - y;
	}

	// Write word
	int idx = -1;
	while (++idx < lim && entry [idx] != 0)
	{
		// Optional translation
		uint8_t val = entry [idx];
		if (dico.AlphabetSize () == 26)
		{
			if (val >= 'A' && val <= 'Z') val += 1 - 'A';
			if (val >= 'a' && val <= 'z') val += 1 - 'a';
		}

		if (val > dico.AlphabetSize ()) break;
		mgrid (x, y)->MakeLetter ();
		mgrid (x, y)->SetLetter (val);
		x += stepx;
		y += stepy;
	}

	// Optional black box
	if (terminator && idx < lim) mgrid (x, y)->MakeBloc ();
}


// ===========================================================================
/// \brief	Read the whole grid content
///
/// When using the standard 26-letters alphabet, the word content
///	is returned in ASCII; '.' for empty box. 
/// Otherwise, the letters are in the range [1..alphabetSize]; ZERO for empty box.
/// In any case, black boxes and void boxes are given the '*' and '-' values.
/// 
///
/// \param		module		Target module
/// \param[out]	grid		Buffer to get the grid content. MUST be big enough
///							to hold (WIDTH x HEIGHT) values.
// ===========================================================================
void Library::ReadGrid (Module* module, uint8_t grid[])
{
	const Grid& mgrid = module->GetGrid ();
	int w = mgrid.GetWidth ();
	int h = mgrid.GetHeight ();
	uint8_t alphaSize = module->GetDictionary ().AlphabetSize ();

	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			const Box* box = mgrid (i, j);
			
			if (box->IsLetter ())
			{
				uint8_t val = box->GetLetter ();
				if (alphaSize == 26) grid [j * w + i] = val > 0 ? (val + 'A' - 1) : '.';
				else grid [j * w + i] = val;
			}
			else if (box->IsBloc ()) grid [j * w + i] = '#';
			else if (box->IsVoid ()) grid [j * w + i] = '-';
		}
	}
}


// ===========================================================================
/// \brief	Erase the whole grid content
///
/// \param		module		Target module
// ===========================================================================
void Library::EraseGrid (Module* module)
{
	module->GetGrid ().Erase ();
}


// ===========================================================================
/// \brief	Start the grid generation process
///
/// \param	module				Target module
/// \param	sovlerConfg			Solver configuration parameters
// ===========================================================================
void Library::SolverStart (Module* module, const SolverConfig& solverConfig)
{
	ISolver& solver = module->GetSolver (solverConfig);	
	solver.Solve_Start (module->GetGrid (), module->GetDictionary ());
}


// ===========================================================================
/// \brief	Continue the grid generation process.
///
/// This function can be called iteratively until the generation fail or succeed.
/// Time and step criterias enable to limit the time spent in the function during a single call.
///
/// \param		module			Target module
/// \param		maxTimeMs:		>=0: Maximum time to spend [ms] before returning.
///								-1: No stop criteria
/// \param		maxSteps:		>=0: Maximum word tries before returning
///								-1: No stop criteria
///
/// \return		Generation status.
// ===========================================================================
Status Library::SolverStep (Module* module, int32_t maxTimeMs, int32_t maxSteps)
{
	ISolver& solver = module->GetSolver ();
	Status status = solver.Solve_Step (maxTimeMs, maxSteps);
	return status;
}


// ===========================================================================
/// \brief	Terminate the grid generation process
// ===========================================================================
void Library::SolverStop (Module* module)
{
	ISolver& solver = module->GetSolver ();
	solver.Solve_Stop ();
}



// ###########################################################################
//
// P R I V A T E
//
// ###########################################################################

// ===========================================================================
/// \brief	Private Constructor
// ===========================================================================
Library::Library ()
{
	modules = nullptr;
}