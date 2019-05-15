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

	bool IsBloc () const {return mType == 'B';}
	bool IsLetter () const {return mType == 'L';}
	bool IsVoid () const {return mType == 'V';}
	bool IsLocked () const {return mLocked;}

	uint8_t GetLetter () const;
	void SetLetter (uint8_t c);

	int8_t GetCounter () const { return this->mCounter; }
	void ResetCounter (int8_t v) { this->mCounter = v; }
	int8_t IncrementCounter () { return ++this->mCounter; }
	int8_t DecrementCounter() { return --this->mCounter; }

	uint8_t GetBlocDensity () const { if (mType != 'B') return 0; else return mValue; }
	void SetBlocDensity (uint8_t density) { if (mType != 'B') return; else mValue = density; }

	void Lock (bool state) {mLocked = state;};

	void ResetCandidates (bool state) { candidates.Reset (state); }
	void SetCandidate (uint8_t c, bool state) { candidates.Set (c, state); }
	void SetCandidate (const LetterCandidates& other) { candidates = other; }
	bool QueryCandidate (uint8_t c) { return candidates.Query (c); }
	const LetterCandidates& GetCandidate () { return candidates; }

	int tag;

private :

	char mType;				///< Box type (letter, void, block)
	uint8_t mValue;			///< Letter in this box or block local densisty (depending on box purpose)
	int8_t mCounter;		///< General purpose counter

	bool mLocked;			///< Content is locked

	LetterCandidates candidates;	///< Letter candidates for this box
};


#endif

