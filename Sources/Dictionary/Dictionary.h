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
/// \file 	Dictionary.h
/// \author	Jean-Sebastien Gonsette
// ###########################################################################

#ifndef __DICTIONARY__H
#define __DICTIONARY__H

#include <stdint.h>

// ===========================================================================
// D E F I N E S
// ===========================================================================

// Longest possible word in the dictionary
constexpr auto MAX_WORD_LENGTH = 40;



// ###########################################################################
//
// T Y P E S
//
// ###########################################################################

struct S_WordNode;
struct S_WordLeaf;

/// Letter candidates for a given position in a word.
struct LetterCandidates
{
	LetterCandidates () : flags (0) { Reset (true); }
	LetterCandidates& operator = (const LetterCandidates& other) { flags = other.flags; return *this; }

	void Reset (bool state) { flags = state ? ((uint64_t) -1) : 0; }
	bool Query (uint8_t c) const { return (flags & (1ULL << c)) > 0; }
	void Set (uint8_t c, bool state)
	{
		if (state) flags = 1ULL << c;
		else flags &= ~(1ULL << c);
	}

	uint64_t flags;		///< Flags, one bit by letter in the alphabet (max alphabet size = 64)
};



// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

/// Dictionary
class Dictionary 
{

public :

	Dictionary (int alphabetSize, int maxWordSize);
	Dictionary () = delete;
	~Dictionary ();

	int Compare (const uint8_t word1 [], const uint8_t word2 []) const;

	void Clear ();
	int32_t AddEntries (const uint8_t* tabEntries, int32_t entrySize, int32_t numWords);
	
	bool FindEntry (uint8_t result [], const uint8_t mask [], const uint8_t startWord [] = nullptr, const LetterCandidates possibleLetters [] = nullptr) const;
	bool FindRandomEntry (uint8_t result [], const uint8_t mask [], const LetterCandidates possibleLetters [] = nullptr) const;

	uint32_t GetNumWords () const {return usedWordLeafs - alphabetSize;}
	uint8_t AlphabetSize () const { return alphabetSize; }
	uint8_t MaxWordSize () const { return maxWordSize; }

private :

	void Clean ();

	int ProcessEntry (const uint8_t* entry, uint8_t* out) const;
	bool AddEntry (const uint8_t* entry);

	S_WordNode* GetWordNode (int idx) const;
	S_WordLeaf* GetWordLeaf (int idx) const;

	S_WordNode* NewWordNode ();
	S_WordLeaf* NewWordLeaf ();

	int MakeIndex (S_WordNode* p) const;
	int MakeIndex (S_WordLeaf* p) const;


private :


	/// Trie root node for every possible word length
	struct S_WordNode **vRootNodes;

	/// Trie nodes pool (for fast allocation)
	int* vWordNodes;

	/// Trie leaves pool (for fast allocation)
	struct S_WordLeaf* vWordLeafs;

	/// Nodes pool size
	unsigned int numWordNodes;

	/// Leaves pool size
	unsigned int numWordLeafs;

	/// Number of nodes used in the pool
	unsigned int usedWordNodes;

	/// Number of leaves used in the pool
	unsigned int usedWordLeafs;

	/// Size of the alphabet, according to user config
	int alphabetSize;

	/// Max size of a word, according to user config
	int maxWordSize;
};


#endif