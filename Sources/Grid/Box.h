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
/// \file		Box.h
/// \author		Jean-Sebastien Gonsette
// ###########################################################################

#ifndef __BOX__H
#define __BOX__H

#include "Dictionary/Dictionary.h"


// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

/// Grid box
class Box
{

public :

	Box ();
	~Box () {}

	void MakeBloc ();
	void MakeLetter ();
	void MakeVoid ();

	bool IsBloc () const {return type == 'B';}
	bool IsLetter () const {return type == 'L';}
	bool IsVoid () const {return type == 'V';}
	bool IsLocked () const {return isLocked;}

	uint8_t GetLetter () const;
	void SetLetter (uint8_t c);

	int8_t GetCounter () const { return this->counter; }
	void ResetCounter (int8_t v) { this->counter = v; }
	int8_t IncrementCounter () { return ++this->counter; }
	int8_t DecrementCounter() { return --this->counter; }

	int GetFailCounter () const { return this->failCounter; }
	void ResetFailCounter () { this->failCounter = 0; }
	int IncrementFailCounter () { return ++this->failCounter; }

	uint8_t GetBlocDensity () const { if (type != 'B') return 0; else return value; }
	void SetBlocDensity (uint8_t density) { if (type != 'B') return; else value = density; }

	void Lock (bool state) {isLocked = state;};

	void ResetCandidates (bool state) { candidates.Reset (state); }
	void SetCandidate (uint8_t c, bool state) { candidates.Set (c, state); }
	void SetCandidate (const LetterCandidates& other) { candidates = other; }
	bool QueryCandidate (uint8_t c) { return candidates.Query (c); }
	const LetterCandidates& GetCandidate () { return candidates; }

	int tag;

private :

	char type;				///< Box type (letter, void, block)
	uint8_t value;			///< Letter in this box or block local densisty (depending on box purpose)
	int8_t counter;			///< Counter used to track how many time the same content has been written in this box
	int failCounter;		///< Counter used to track how many time this box is implicated in a failure

	bool isLocked;			///< Content is locked

	LetterCandidates candidates;	///< Letter candidates for this box
};


#endif

