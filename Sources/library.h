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
/// \file Library.h
///
/// \brief	Singleton holding all the Wizium instances
// ###########################################################################

#ifndef LIBRARY_H
#define LIBRARY_H

#include "libWizium.h"


// ###########################################################################
//
// T Y P E S
//
// ###########################################################################

/// Library management
class Library
{
public:

	/// Independent Map Matching module
	class Module;


public:

	static Library& GetInstance ();
	~Library ();

	Module* CreateInstance (const Config& config);
	void DestroyInstance (Module*);

	void ClearDictionary (Module* module);
	int32_t AddDictionaryEntries (Module* module, const uint8_t* tabEntries, int32_t entrySize, int32_t numWords);
	bool FindDictionaryEntry (Module* module, uint8_t* result, const uint8_t* mask, const uint8_t* startWord) const;
	bool FindRandomDictionaryEntry (Module* module, uint8_t* result, const uint8_t* mask) const;
	uint32_t GetNumDictionaryWords (Module* module) const;

	void SetGridSize (Module* module, uint8_t width, uint8_t height);
	void SetGridBox (Module* module, uint8_t x, uint8_t y, BoxType type);
	void WriteGrid (Module* module, uint8_t x, uint8_t y, const uint8_t entry[], char dir, bool terminator);
	void ReadGrid (Module* module, uint8_t grid[]);
	void EraseGrid (Module* module);

	void SolverStart (Module* module, const SolverConfig& solver);
	Status SolverStep (Module* module, int32_t maxTimeMs, int32_t maxSteps);
	void SolverStop (Module* module);


private:

	Library ();


private:

	/// Unique instance of this singleton
	static Library s_library;

	/// All independant modules
	const Module* modules;
};


#endif
