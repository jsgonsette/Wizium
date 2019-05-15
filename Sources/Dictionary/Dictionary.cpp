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
/// \file	Dictionary.cpp
/// \author	Jean-Sebastien Gonsette
///
/// \brief	Management of a dictionary of words.
// ###########################################################################

#include "libWizium.h"
#include "Dictionary.h"

#include <stdlib.h>
#include <string.h>


// ===========================================================================
// D E F I N E
// ===========================================================================

#define WILDCARD		255


// ===========================================================================
// T Y P E S
// ===========================================================================

/// Trie (Prefix tree) node. Each node represents a letter and has x letters 
/// possible pointers to the next letter of a word. (x is configurable)
struct S_WordNode
{
	S_WordNode () = delete;
	int& operator [] (int index) { return *(reinterpret_cast<int*> (this) + index); }
};

/// Last node in the trie (leaf). Characteristics of the words are stored here
struct S_WordLeaf
{
	S_WordLeaf() { idxDeffinition = -1; }
	int idxDeffinition;
};



// ###########################################################################
//
// P U B L I C - Dictionary
//
// ###########################################################################

// ===========================================================================
/// \brief		Constructor
// ===========================================================================
Dictionary::Dictionary (int _alphabetSize, int _maxWordSize) : alphabetSize (_alphabetSize), maxWordSize (_maxWordSize)
{
	if (maxWordSize <= 0 || maxWordSize > MAX_WORD_LENGTH) maxWordSize = MAX_WORD_LENGTH;
	if (alphabetSize > 64) alphabetSize = 64;
	if (alphabetSize <= 0) alphabetSize = 26;

	// One root for each possible word length
	vRootNodes = new S_WordNode* [maxWordSize];
	vWordNodes = nullptr;
	vWordLeafs = nullptr;
	usedWordNodes = 0;
	usedWordLeafs = 0;
	numWordLeafs = 0;
	numWordNodes = 0;

	// Empty dictionary
	Clear ();
}


// ===========================================================================
/// \brief		Destructor
// ===========================================================================
Dictionary::~Dictionary ()
{
	Clean ();
	delete [] vRootNodes;
}


// ===========================================================================
// ===========================================================================
int Dictionary::Compare (const uint8_t word1 [], const uint8_t word2 []) const
{
	for (int i = 0; i < maxWordSize; i ++)
	{
		if (word1 [i] < word2 [i]) return -1;
		else if (word1 [i] > word2 [i]) return 1;
		else if (word1 [i] == 0) return 0;
	}
	return 0;
}


// ===========================================================================
/// \brief		Clear dictionary content and free all memory
// ===========================================================================
void Dictionary::Clear ()
{
	uint8_t w [2] = {0, 0};

	// Free everything
	Clean ();

	// One root for each possible word length
	for (int i = 0; i < this->maxWordSize; i ++) vRootNodes [i] = NewWordNode ();
	
	// Add all '1 letter' words
	for (uint8_t i = 1; i <= alphabetSize; i ++ )
	{
		w [0] = i;
		AddEntry (w);
	}
}


// ===========================================================================
/// \brief	Find a word randomly in the dictionary, 
///			on the basis of a mask and of letter candidates.
///
/// \param[out]	result		Array to write the matching word
///							Each letter is in the range [1..alphabetSize]
/// \param		mask		Mask enabling to force some letters. 
///							'*' means any letter (e.g. "*A***I**)
///							Word length is given implicitly by the mask length.
/// \param		candidates	Letter candidates. If not null, must be an array as long as the mask length.
///
/// \return		True if a match has been found
// ===========================================================================
bool Dictionary::FindRandomEntry (uint8_t result [], const uint8_t mask [], const LetterCandidates possibleLetters []) const
{
	int i;
	int idxSubNode;
	int depth;
	uint8_t idxLetter;
	uint8_t maskEntry [MAX_WORD_LENGTH+1];
	uint8_t first [MAX_WORD_LENGTH];
	S_WordNode* tabDepthNodes [MAX_WORD_LENGTH];
	S_WordNode *pNode;
	S_WordNode *pSubNode;
	S_WordLeaf *pLeaf;

	// Sanitize entries
	if (result == nullptr) return false;
	int maskLen = ProcessEntry (mask, maskEntry);
	if (maskLen == 0) return false;
		
	// Point on the trie root with the right length
	pNode = vRootNodes [maskLen -1];

	// Reset buffers
	for (i = 0; i < maskLen; i ++) {
		result [i] = 0;
		first [i] = 255;
	}
	
	// Go down in the trie until we find a matching word
	depth = 0;
	while (depth < maskLen)
	{
		// 1) Select a letter at current 'depth' level
		// if letter at this depth can be anything (wildcard) ...
		if (maskEntry [depth] == WILDCARD)
		{
			idxSubNode = -1;

			// The first time, take a letter at randow. After, we just move on.
			if (first [depth] == 255) idxLetter = rand () % this->alphabetSize;
			else idxLetter = (result [depth]-1) +1;

			// Search for a letter candidate (among the alphabet)
			for (i = 0; i < this->alphabetSize; i++)
			{
				// Check for alphabet loopback
				if (idxLetter >= this->alphabetSize) idxLetter = 0;

				// Check we didn't get our first try
				if (first [depth] == idxLetter) break;

				// Save first random letter to detect loopback later
				if (first [depth] == 255) first [depth] = idxLetter;

				// Find an existing letter
				if (pNode->operator [] (idxLetter) >= 0)
				{
					// Check this letter is acceptable
					if (possibleLetters == nullptr || possibleLetters [depth].Query (idxLetter))
					{
						idxSubNode = pNode->operator [] (idxLetter);
						break;
					}
				}

				// Move on
				idxLetter ++;
			}
		}
		// If letter at this level if forced ...
		else
		{
			// We follow the mask and go forward
			idxLetter = maskEntry [depth] -1;
			idxSubNode = pNode->operator [] (idxLetter);
		}

		// 2) Get corresponding next node (or leaf)
		if (depth < maskLen - 1) {
			pSubNode = GetWordNode (idxSubNode);
			pLeaf = nullptr;
		}
		else {
			pLeaf = GetWordLeaf (idxSubNode);
			pSubNode = nullptr;
		}

		// 3) If next node is valid, write our solution at this depth
		if (pSubNode != nullptr || pLeaf != nullptr) result [depth] = idxLetter + 1;
		else 
		{
			result [depth] = 0;
			first [depth] = 255;
		}

		// 4) Follow our subnode (or leaf) ...
		if (pSubNode != nullptr)
		{
			tabDepthNodes [depth] = pNode;
			pNode = pSubNode;
			depth ++;
		}
		else if (pLeaf != nullptr) depth ++;

		// or go backward if there is no corresponding word in the dictionary...
		else
		{
			// Skip mandatory letters when going backward
			do	{depth --;}  while (depth >= 0 && maskEntry [depth] != WILDCARD);

			// Not possible to go further backward ? -> Fail
			if (depth < 0)
			{
				if (result != nullptr) result [0] = 0;
				return false;
			}

			pNode = tabDepthNodes [depth];
		}
	}

	if (maskLen < this->maxWordSize) result [maskLen] = 0;
	return true;
}


// ===========================================================================
/// \brief	Find a word in the dictionary, 
///			on the basis of a start point, of a mask and of letter candidates.
///
/// \param[out]	result		Array to write the matching word. 
///							Each letter is in the range [1..alphabetSize]
/// \param		mask		Mask enabling to force some letters. 
///							'*' means any letter (e.g. "*A***I**)
///							Word length is given implicitly by the mask length.
/// \param		start		Word to start the search in the dictionary
///							If null, search starts at the begining of the dictionary
/// \param		candidates	Letter candidates. If not null, must be an array as long as the mask length.
///
/// \return		True if a match has been found
// ===========================================================================
bool Dictionary::FindEntry (uint8_t result [], const uint8_t mask [], const uint8_t start [], const LetterCandidates possibleLetters []) const
{
	int i;
	int idxSubNode;
	int depth = 0;
	bool hotStart;
	uint8_t idxLetter = -1;
	uint8_t idx;
	uint8_t maskEntry [MAX_WORD_LENGTH];
	uint8_t startEntry [MAX_WORD_LENGTH];
	S_WordNode* tabDepthNodes [MAX_WORD_LENGTH];
	S_WordNode *pNode;
	S_WordNode *pSubNode;
	S_WordLeaf *pLeaf;

	// Sanitize mask
	if (result == nullptr) return false;
	int maskLen = ProcessEntry (mask, maskEntry);
	if (maskLen == 0) return false;

	// Sanitize and used start suggestion
	int startLen = 0;
	memset (startEntry, 0, maskLen);
	if (start != nullptr) startLen = ProcessEntry (start, startEntry);
	memcpy (result, startEntry, maskLen);

	// Point on the trie root with the right length
	pNode = vRootNodes [maskLen -1];

	// Init matching word with the start word
	if (startLen > 0) hotStart = true;
	else hotStart = false;

	// Go down in the trie until we find a matching word
	while (depth < maskLen)
	{
		// If we have a start proposition, we must first find it before changing anything
		if (depth == (maskLen - 1)) hotStart = false;

		// 1) Select a letter at current 'depth' level
		// Letter at this depth can be anything ...
		if (maskEntry [depth] == WILDCARD)
		{
			// Start from the current letter solution at this depth, if any,
			// and go one step further if possible
			idx = result [depth];
			if (idx != 0) 
			{
				idx = idx -1;
				if (hotStart == false) idx ++;
			}

			// Search for a letter candidate, starting from the previous solution
			idxSubNode = -1;
			for (idxLetter = idx; idxLetter < this->alphabetSize; idxLetter++)
			{
				// Find an existing letter
				if (pNode->operator [] (idxLetter) >= 0)	
				{
					// Check this letter is acceptable				
					if (possibleLetters == nullptr || possibleLetters [depth].Query (idxLetter) )
					{
						idxSubNode = pNode->operator [] (idxLetter);
						break;
					}
				}
			}
		}
		
		// Letter at this depth is given from the mask ...
		else
		{
			// If we don't follow the start word anymore and that the letter at this depth
			// is defined, this means that we already exausted all the possibilities
			// for the following letters. Then we must go backward
			if (result [depth] != 0 && hotStart == false) idxSubNode = -1;

			// We follow the mask and go forward
			else
			{
				idxLetter = maskEntry [depth] -1;
				idxSubNode = pNode->operator [] (idxLetter);
			}
		}

		// 2) Get corresponding next node (or leaf)
		if (depth < maskLen - 1) {
			pSubNode = GetWordNode (idxSubNode);
			pLeaf = nullptr;
		}
		else {
			pLeaf = GetWordLeaf (idxSubNode);
			pSubNode = nullptr;
		}

		// 3) If next node is valid, write our solution at this depth
		if (pSubNode != nullptr || pLeaf != nullptr) 
			result [depth] = idxLetter + 1;
		else result [depth] = 0;
	

		// 4) If we had to follow 'start' up to there and that we didn't, it means
		// one letter was forbidden. -> we stop following 'start'
		if (hotStart == true)
		{
			bool failFollow = idxSubNode == -1 || result [depth] != startEntry [depth];
			if (failFollow)
			{
				hotStart = false;
				for (i = depth + 1; i < maskLen; i++) result[i] = 0;

				// Force to go back if 'mask' is before 'start' letter
				if (result [depth] > 0 && startEntry [depth] > 0 && result [depth] < startEntry [depth])
				{
					pSubNode = nullptr;
					pLeaf = nullptr;
				}

			}
		}
		
		// 5) Follow our subnode (or leaf) ...
		if (pSubNode != nullptr)
		{
			tabDepthNodes [depth] = pNode;
			pNode = pSubNode;
			depth ++;
		}
		else if (pLeaf != nullptr) depth ++;

		// ... or go backward if there is no corresponding word in the dictionary...
		else
		{
			depth --;
			if (depth < 0)
			{
				if (result != nullptr) result [0] = 0;
				return false;
			}

			pNode = tabDepthNodes [depth];
		}
	}

	if (maskLen < this->maxWordSize) result [maskLen] = 0;
	return true;
}


// ===========================================================================
/// \brief	Add a word list in the dictionary
///
/// \param		tabEntries		List of words. End of list when a null caracter is 
///								found instead of a new word or if 'numWords' is reached
/// \param		entrySize		> 0: size of every word in the list
///								<=0: every word is null terminated. 
/// \param		numWords		Number of words in the list
///
/// \return		True
// ===========================================================================
int32_t Dictionary::AddEntries (const uint8_t* tabEntries, int32_t entrySize, int32_t numWords)
{
	uint8_t word [MAX_WORD_LENGTH+1];
	const uint8_t* pChar = tabEntries;
	bool result;
	int idx;
	int count = 0;

	// Loop on word list
	while (true)
	{
		// Last word detection (double 0)
		if (pChar [0] == 0) break;
		idx = 0;

		do
		{
			word [idx] = pChar [idx];

			// Working with default 26 letters alphabet ? Then we accept ASCII letters
			if (alphabetSize == 26)
			{
				if (word [idx] >= 'A' && word [idx] <= 'Z') word [idx] += 1 - 'A';
				if (word [idx] >= 'a' && word [idx] <= 'z') word [idx] += 1 - 'a';
			}
			
			// Check we are in the range [1..alphabetSize], or stop
			if (word [idx] >= 1 && word [idx] <= this->alphabetSize && idx < MAX_WORD_LENGTH) idx ++;
			else if (word [idx] == 0) break;
			else
			{
				word [0] = 0;
				break;
			}

			// Check end of word if fixed length
			if (entrySize >= 0 && idx >= entrySize)
			{
				word [idx] = 0;
				break;
			}
		}
		while (true);
		if (word [0] == 0) break;

		// Add single word
		result = AddEntry (word);
		if (result == false) break;
		
		// Move to next word in the list
		if (entrySize > 0) pChar += entrySize;
		else pChar += idx;

		// End of list detection
		count ++;
		if (count >= numWords && numWords >= 0) break;
	}

	return count;
}



// ###########################################################################
//
// P R I V A T E
//
// ###########################################################################

// ===========================================================================
/// \brief	Remove everything and clean memory
// ===========================================================================
void Dictionary::Clean ()
{
	if (vWordNodes != nullptr) delete [] vWordNodes;
	if (vWordLeafs != nullptr) delete [] vWordLeafs;

	vWordNodes = nullptr;
	vWordLeafs = nullptr;
	
	numWordNodes = 0;
	numWordLeafs = 0;

	usedWordNodes = 0;
	usedWordLeafs = 0;
}


// ===========================================================================
/// \brief	Process a user word entry before further processing.
///
/// Its content must be in the range [1..alphabetSize], other values being
/// considered as wildcard.
///
/// \return	Entry length
// ===========================================================================
int Dictionary::ProcessEntry (const uint8_t* entry, uint8_t* out) const
{
	int idx;

	for (idx = 0; idx < this->maxWordSize; idx++)
	{
		if (entry [idx] == 0) break;
		out [idx] = entry [idx];

		if (alphabetSize == 26)
		{
			if (entry [idx] >= 'A' && entry [idx] <= 'Z') out [idx] += 1 - 'A';
			if (entry [idx] >= 'a' && entry [idx] <= 'z') out [idx] += 1 - 'a';
		}
		if (out [idx] > this->alphabetSize) out [idx] = WILDCARD;
	}
	
	return idx;
}



// ===========================================================================
/// \brief	Add a single word in the dictionary
///
/// \param	entry	Null terminated word
///
/// \return True
// ===========================================================================
bool Dictionary::AddEntry (const uint8_t* entry)
{
	int i, len=-1;
	int idxNode = -1, idxSubNode;
	int idxLetter;

	S_WordNode *pWordNode;
	S_WordNode *pSubNode;
	S_WordLeaf *pWordLeaf;

	while (entry [++len] != 0);
	if (len > this->maxWordSize || len == 0) 
		return false;

	// Get trie corresponding to the given word length
	pWordNode = vRootNodes [len-1];

	// Add letters, one by one
	for (i = 0; i < len-1; i ++)
	{
		// Letter index
		idxLetter = entry [i] -1;

		// Get sub node for this letter
		idxSubNode = pWordNode->operator[] (idxLetter);
		pSubNode = GetWordNode (idxSubNode);

		// If sub node not found, create it
		if (pSubNode == nullptr)
		{
			// Save node index
			if (i > 0) idxNode = MakeIndex (pWordNode);

			// Allocate new node (this can invalidate all node pointers)
			pSubNode = NewWordNode ();

			// Restore node from index
			if (i > 0) pWordNode = GetWordNode (idxNode);

			// Link to parent
			pWordNode->operator[] (idxLetter) = MakeIndex (pSubNode);
		}

		// Move on next node
		pWordNode = pSubNode;
	}

	// Leave for the final letter
	idxLetter = entry [i] -1;
	idxSubNode = pWordNode->operator[] (idxLetter);

	pWordLeaf = GetWordLeaf (idxSubNode);

	// Create leaf if it doesn't exist
	if (pWordLeaf == nullptr)
	{
		// Allocation and link it to its parent node
		pWordLeaf = NewWordLeaf ();
		pWordNode->operator[] (idxLetter) = MakeIndex (pWordLeaf);

		pWordLeaf->idxDeffinition = -1;
	}
		
	return true;
}


// ===========================================================================
/// \brief	Convert a pool index into a Trie node
///
/// \param	idx		Index to convert
///
/// \return	Trie node. Null if index out of bounds.
// ===========================================================================
S_WordNode* Dictionary::GetWordNode (int idx) const
{
	if (idx < 0 || (unsigned int) idx >= numWordNodes) return nullptr;

	int* location = &vWordNodes [idx * this->alphabetSize];
	return reinterpret_cast<S_WordNode*> (location);
}


// ===========================================================================
/// \brief	Convert a pool index into a Trie leaf
///
/// \param	idx		Index to convert
///
/// \return	Trie leaf. Null if index out of bounds.
// ===========================================================================
S_WordLeaf* Dictionary::GetWordLeaf (int idx) const
{
	if (idx < 0 || (unsigned int) idx >= numWordLeafs) return nullptr;
	return (&vWordLeafs [idx]);
}


// ===========================================================================
/// \brief	Convert a Trie node pointer into a pool index (independent of pool location)
///
/// \param	p		Pointer to convert
///
/// \return	Trie node index. -1 if invalid pointer.
// ===========================================================================
int Dictionary::MakeIndex (S_WordNode* p) const
{
	uint64_t idx = reinterpret_cast<int*> (p) - vWordNodes;
	idx /= this->alphabetSize;

	if (idx >= numWordNodes) return -1;
	return (int) idx;
}


// ===========================================================================
/// \brief	Convert a Trie leaf pointer into a pool index (independent of pool location)
///
/// \param	p		Pointer to convert
///
/// \return	Trie leaf index. -1 if invalid pointer.
// ===========================================================================
int Dictionary::MakeIndex (S_WordLeaf* p) const
{
	uint64_t idx = p - vWordLeafs;

	if (idx >= numWordLeafs) return -1;
	return (int) idx;
}


// ===========================================================================
/// \brief	Allocate a new Trie node from the pool. 
///
/// If pool is not big enough, pool is resized. However it means that
/// pointer to nodes are not valid anymore.
///
/// \return	Trie node.
// ===========================================================================
S_WordNode* Dictionary::NewWordNode ()
{
	if (usedWordNodes >= numWordNodes)
	{
		unsigned int newSize;
		int *pNewTab;

		// New pool size
		newSize = (unsigned int) (numWordNodes * 1.4f);
		if (newSize == 0) newSize = 10000;

		// Create new pool (each S_WordNode size is (int x alphabetSize))
		pNewTab = new int [(size_t) newSize * this->alphabetSize];
		if (pNewTab == nullptr) return nullptr;

		// Copy old to new and init the rest to -1
		memcpy (pNewTab, vWordNodes, sizeof (int) * this->alphabetSize * usedWordNodes);
		memset (
			pNewTab + (size_t) this->alphabetSize * usedWordNodes,
			-1,
			sizeof (int) * this->alphabetSize  * (newSize -  usedWordNodes));

		delete [] vWordNodes;	
		vWordNodes = pNewTab;
		numWordNodes = newSize;

		// Update root nodes
		for (int i = 0; i < this->maxWordSize; i++) vRootNodes[i] = GetWordNode (i);
	}

	int* location = &vWordNodes [usedWordNodes * this->alphabetSize];
	usedWordNodes ++;

	return reinterpret_cast<S_WordNode*> (location);
}


// ===========================================================================
/// \brief	Allocate a new Trie leaf from the pool. 
///
/// If pool is not big enough, pool is resized. However it means that
/// pointer to leaves are not valid anymore.
///
/// \return	Trie leaf.
// ===========================================================================
S_WordLeaf* Dictionary::NewWordLeaf ()
{
	// No more room
	if (usedWordLeafs >= numWordLeafs)
	{
		int newSize;
		struct S_WordLeaf *pNewTab;

		// Allocate bigger pool
		newSize = (int) (numWordLeafs * 1.4f);
		if (newSize == 0) newSize = 10000;

		pNewTab = new S_WordLeaf [newSize];
		if (pNewTab == nullptr) return nullptr;

		// Copy old pool to new pool
		memcpy (pNewTab, vWordLeafs, sizeof (S_WordLeaf) * usedWordLeafs);
		delete [] vWordLeafs;
		
		vWordLeafs = pNewTab;
		numWordLeafs = newSize;
	}

	usedWordLeafs ++;
	return (&vWordLeafs [usedWordLeafs-1]);
}



// End
