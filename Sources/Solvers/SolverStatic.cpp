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
/// \file		SolverStatic.cpp
/// \author		Jean-Sebastien Gonsette
///
/// \brief		Solver with fixed grid layout
// ###########################################################################

#include <stdlib.h>
#include <string.h>
#include <chrono>

#include "SolverStatic.h"
#include "SolverStatic.StaticItem.h"
#include "Dictionary/Dictionary.h"


// ===========================================================================
// D E F I N E
// ===========================================================================


// ===========================================================================
// T Y P E S
// ===========================================================================


int Len2 (uint8_t* s)
{
	int len = -1;
	while (s[++len] != 0);
	return len;
}


// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief		Constructor
// ===========================================================================
SolverStatic::SolverStatic ()
{
	seed = 0;
	m_pItemList = nullptr;
	m_numItems = -1;		
	m_idxCurrentItem = -1;

	m_heurestic = true;
	m_backTreshold = 0;
}


// ===========================================================================
/// \brief		Destructor
// ===========================================================================
SolverStatic::~SolverStatic ()
{
	if (m_pItemList != nullptr) delete [] m_pItemList;
}


// ===========================================================================
/// \brief		Set the heuristic to use during the grid generation
///
/// An heuristic is an empirical method enabling to reduce problem complexity
/// by removing problem sub spaces that are likely to be unsuccessful.
////
/// \param	state			True to activate the heuristic
/// \param	backTreshold	Heuristic level (not used)
// ===========================================================================
void SolverStatic::SetHeurestic (bool state, int backTreshold)
{
	this->m_heurestic = state;
	this->m_backTreshold = backTreshold;
}


// ===========================================================================
/// \brief		Stop searching for a solution
// ===========================================================================
void SolverStatic::Solve_Stop ()
{
	// Unlock all grid boxes	
	if (pGrid != nullptr)
	{
		for (int y = 0; y < mSy; y ++)
		{
			for (int x = 0; x < mSx; x ++)
			{
				Box* box = pGrid->operator ()(x, y);
				box->Lock (false);
			}
		}
	}

	this->pDict = nullptr;
	this->pGrid = nullptr;
	this->steps = 0;
	m_numItems = -1;
	m_idxCurrentItem = -1;
}


// ===========================================================================
/// \brief		Start searching for a solution to fill in a grid.
///
/// \param		pGrid		Grid to solve
/// \param		pDict		Main dictionary to fill in the grid
// ===========================================================================
void SolverStatic::Solve_Start (Grid &grid, const Dictionary &dico)
{
	int x, y;
	int count = 0;
	Box *box = nullptr;

	Solve_Stop ();

	// Save dictionary and grid
	this->pDict = &dico;
	this->pGrid = &grid;
	this->mSx = pGrid->GetWidth ();
	this->mSy = pGrid->GetHeight ();

	// This solver doesn't add any block
	this->pGrid->SetDensityMode (Grid::BlocDensityMode::NONE);

	// Lock non empty boxes
	for (y = 0; y < mSy; y ++)
	{
		for (x = 0; x < mSx; x ++)
		{
			box = pGrid->operator ()(x,y);

			// Lock the box if not empty
			if (box->IsLetter () == false || box->GetLetter() != 0) 
				box->Lock (true);

			// If empty, store the number of preceding unlocked boxes
			else
			{
				box->tag = count;
				box->Lock (false);
				count ++;
			}
		}
	}

	// Erase the grid, except the locked boxes (why ?)
	pGrid->Erase ();

	// Establish a static ordered list of word slots for whose we must find a solution
	if (m_pItemList != nullptr) delete [] m_pItemList;
	BuildWordList ();
	m_idxCurrentItem = 0;

	// Init step counter and rng
	this->steps = 0;
	srand (static_cast<uint32_t> (this->seed));
}


// ===========================================================================
/// \brief	Move on in the grid generation process
///
/// \param		maxTimeMs:		>=0: Maximum time to spend [ms] before returning.
///								-1: No stop criteria
/// \param		maxSteps:		>=0: Maximum word tries before returning
///								-1: No stop criteria
///
/// \return		Generation status.
// ===========================================================================
Status SolverStatic::Solve_Step (int32_t maxTimeMs, int32_t maxSteps)
{
	uint64_t initCounter = this->steps;
	Status status;

	// Get time in
	auto start = std::chrono::system_clock::now ();

	if (pGrid == nullptr || pDict == nullptr)
	{
		status.counter = this->steps;
		status.fillRate = 0;
		return status;
	}

	// Main search loop, we have finished when we have found something for every slots in our list
	while (m_idxCurrentItem < m_numItems)
	{		
		// Skip slots tagged so
		//if (m_pItemList [m_idxCurrentItem].dictStep == C_StaticItem2::Ce_Skip)
		//{
		//	m_idxCurrentItem ++;
		//	continue;
		//}

		// Get item to solve during this iteration
		StaticItem *pItem = &m_pItemList [m_idxCurrentItem];

		// Reset 
		pItem->word [0] = 0;
		pItem->prevWord [0] = 0;
		pItem->firstWord [0] = 0;
		pItem->ResetCrossCandidates ();
		LoadCandidatesFromGrid (*pItem);

		// Find a first solution for this item
		unsigned int subCounter;
		int validatedCol;
		bool result = ChangeItem (*pItem, -1, &validatedCol, &subCounter);
		
		this->steps += subCounter;
		SaveCandidatesToGrid (*pItem);

		// If no solution, backtrack up to a previous item
		if (result == false) BackTrack ();

		// Backtracking failed ?
		if (m_idxCurrentItem < 0)
		{
			pDict = nullptr;
			break;
		}

		// Add current item to the grid
		AddCurrentItem ();
		
		// Check time / counter criteria
		if (maxTimeMs >= 0)
		{
			auto stop = std::chrono::system_clock::now ();
			std::chrono::duration<double> duration = stop - start;
			if (duration.count () >= maxTimeMs / 1000.0) break;
		}
		if (maxSteps >= 0)
		{
			int64_t delta = this->steps - initCounter;
			if (delta >= maxSteps) break;
		}
	}
		
	status.counter = this->steps;
	status.fillRate = pGrid ? pGrid->GetFillRate () : 0;
	return status;
}



// ###########################################################################
//
// P R I V A T E
//
// ###########################################################################

// ===========================================================================
/// \brief	Find a backtrack point and restore the grid at that point
///
/// \param		bestPos			Best letter index achieved for the current item before failing
/// \param[out]	pIdxToChange	Letter index of the new current item that must change
/// \param[out]	pmask			New current item mask
///
///	\return		New current item (Null if we cannot backtrack any further)
// ===========================================================================
void SolverStatic::BackTrack ()
{
	uint8_t mask [MAX_GRID_SIZE + 1];

	StaticItem *next = nullptr;
	StaticItem *target = &m_pItemList [m_idxCurrentItem];
	int targetCol = target->posX + target->bestPos + 1;

	while (next == nullptr)
	{
		// Remove items up to a given point where we can interact with a given target item
		do
		{
			m_idxCurrentItem --;
			if (m_idxCurrentItem < 0) break;
			pGrid->RemoveWord (m_pItemList [m_idxCurrentItem].posX, m_pItemList [m_idxCurrentItem].posY, 'H');

			// Does this new word interact with our target ?
			// - build interaction mask. Skip directly if no interaction at all
			next = &m_pItemList [m_idxCurrentItem];
			if (AreDependant (*next, *target, mask) == 0) continue;

			// - Check case where we want direct connection with 'targetCol'
			if (targetCol >= 0)
			{
				// Scan positions up to the target column. Strong interaction
				// if we have visibility for any one of them.
				bool strongInteraction = false;
				for (int i = targetCol; i >= next->posX; i --)
				{
					if (mask [i - next->posX] == '*')
					{
						strongInteraction = true;
						targetCol = i;
						break;
					}
				}
				if (strongInteraction) break;
			}

			// - otherwise we are happy with only weak interaction. 
			else
			{
				// Ensure the part of the next item we update still interact
				targetCol = next->length;
				while (mask [--targetCol] == '.');
				targetCol += next->posX;
				break;
			}
		} 
		while (true);
		if (m_idxCurrentItem < 0) break;

		// Now try to change the word. We can force to change length
		int valCol;
		unsigned int counter;
		bool r = ChangeItem (*next, targetCol, &valCol, &counter);

		// Update counter and candidates
		this->steps += counter;
		SaveCandidatesToGrid (*next);

		// Failed ?
		if (r == false)
		{
			target = next;
			next = nullptr;

			if (target->word [0] == 0) targetCol = -1;
			else targetCol = valCol +1;
		}
	}

}


// ===========================================================================
/// \brief	Change the content of an item in the backtracking list.
///
/// \param	item			Item to change
/// \param	colToChange		Absolute position (along the row) of the letter that must change. (-1 if not needed)
/// \param	pValidatedCol	Best absolute position that we could reach while trying to choose a word to put on the grid.
///							Letter of a word are checked from the left to right. 
///							The check consits in to be sure a cross word can still be layout on the grid at each letter position.
/// \param	pNumAttempts	Counter giving the number of words we tried before finding a suitable solution.
///
///	Return:					True if we could find a suitable solution.
// ===========================================================================
bool SolverStatic::ChangeItem (StaticItem &item, int colToChange, int *pValidatedCol, unsigned int* pCounter)
{
	bool result;

	uint32_t stepCounter = 0;				///< Step counter
	int unvalidatedIdx = -1;				///< Word letter to force to change
	uint8_t mask[MAX_GRID_SIZE + 1];		///< Grid content at the item location

	// Get mask from the grid for this item
	// Build all cross masks for this item
	pGrid->BuildMask (mask, item.posX, item.posY, 'H', false);
	BuildCrossMasks (item);

	// Take count of the request to change the word at a given ABSOLUTE position
	if (colToChange >= 0) unvalidatedIdx = colToChange - item.posX;

	// Search loop
	do
	{
		stepCounter++;

		// Find a possible word, given the mask and the letter candidates.
		// We can force a given letter to change
		result = ChangeItemWord (item, mask, unvalidatedIdx);
		unvalidatedIdx = -1;

		// We found a candidate
		if (result)
		{
			// If a word is found, cross check every letter. 'bestPos' memorize how far we went.
			result = CheckItemCross (item, &item.bestPos);
			if (result) break;
		}
		else break;
	} while (true);

	if (pValidatedCol != nullptr) *pValidatedCol = item.posX + item.bestPos;
	if (pCounter != nullptr) *pCounter = stepCounter;

	return result;
}


// ===========================================================================
/// \brief	Find next word candidate for a given grid slot.
///
/// Word must meet already placed letters (mask) and must not conflict invalid letters
///
/// \param[in,out]	item			Target item
/// \param			mask			Mask giving the grid content at its position
/// \param			unvalidatedIdx	Index of the letter we want to force a change (-1 if not used)
///
/// \return			True in case of success
// ===========================================================================
bool SolverStatic::ChangeItemWord (StaticItem &item, uint8_t mask [], int unvalidatedIdx)
{
	uint8_t letterToChange;
	bool loopStatus = false;

	// Enable to detect we looped completely over the dictionary
	if (item.word [0] != 0 && item.firstWord [0] != 0) {
		if (pDict->Compare (item.word, item.firstWord) < 0) loopStatus = true;
	}

	// If we force a given letter to change ?
	if (unvalidatedIdx >= 0)
	{
		letterToChange = item.word [unvalidatedIdx];

		// Force letters coming after to the last possibility -> next
		// word will have letter at the given index increased
		for (int i = unvalidatedIdx + 1; i < item.length; i++)
			item.word [i] = pDict->AlphabetSize ();
	}

	// Search loop
	while (true)
	{
		bool found;

		// Look for something in the dictionary
		// If it is the first time we try, choose begining at random
		if (item.word [0] == 0) found = pDict->FindRandomEntry (item.word, mask, item.possibleLetters);
		else found = pDict->FindEntry (item.word, mask, item.word, item.possibleLetters);

		// If nothing found, restart at the begining of the dictionary
		// (can only be done once)
		if (found == false)
		{
			if (loopStatus == true) return false;
			loopStatus = true;

			item.word [0] = 0;
			found = pDict->FindEntry (item.word, mask, item.word, item.possibleLetters);
		}

		// Could not find anything ?
		if (found == false) return false;

		// If we find back the starting word (or go beyond), we fail
		if (loopStatus && item.firstWord [0] != 0 &&
			(pDict->Compare (item.word, item.firstWord) >= 0))
		{
			item.word [0] = 0;
			return false;
		}

		// Check the letter to change actually changed.
		// There is no guarantee, for example: if we must change 'A' in 'SABLER', we could still get 'TABLER'
		if (unvalidatedIdx >= 0 && letterToChange == item.word [unvalidatedIdx]) continue;

		// Check the word is not already on the grid (TODO)
		//if (item.length > 1);

		// All conditions match
		break;
	}

	// Save first word if needed
	if (item.firstWord [0] == 0) memcpy (item.firstWord, item.word, (size_t) item.length + 1);
	return true;
}


// ===========================================================================
/// \brief	Place the current item solution on the grid
// ===========================================================================
void SolverStatic::AddCurrentItem ()
{	
	// Put the word on the grid
	StaticItem *pItem = &m_pItemList [m_idxCurrentItem];
	pGrid->AddWord (pItem->posX, pItem->posY, 'H', pItem->word);

	// For each updated letter, reset letter candidates on the cross boxes
	ResetCandidatesAround (m_pItemList [m_idxCurrentItem]);

	// Save new reference to keep track of updated letters
	memcpy (pItem->prevWord, pItem->word, pItem->length+1);

	// This parameter is only valid in case of failure
	pItem->bestPos = -1;

	// Move on next item in our list, no more failing item
	m_idxCurrentItem ++;
}


// ===========================================================================
/// \brief	Save letter candidates of a given word in the grid
///
/// \param	item	Target item
// ===========================================================================
void SolverStatic::SaveCandidatesToGrid (const StaticItem &item)
{
	for (int i = 0; i < item.length; i++)
	{
		Box* box = pGrid->operator ()(item.posX +i, item.posY);
		box->SetCandidate (item.possibleLetters [i]);
	}
}


// ===========================================================================
/// \brief	Load letter candidates of a given word from the grid
///
/// \param[in,out]	item	Target item
// ===========================================================================
void SolverStatic::LoadCandidatesFromGrid (StaticItem &item)
{
	for (int i = 0; i < item.length; i++)
	{
		Box* box = pGrid->operator ()(item.posX + i, item.posY);
		item.possibleLetters [i] = box->GetCandidate ();
	}
}


// ===========================================================================
/// \brief	Reset letter candidates for all the word crossing a given word, but only
///			in unsolved boxes that have visibility with updated letters.
///
/// Word already in place are not affected. For our backtracking strategy, this means
/// that only upcoming items are affected.
///
/// \param	item	Target item
// ===========================================================================
void SolverStatic::ResetCandidatesAround (const StaticItem &item)
{
	int i;

	int x, y;
	Box *pCase;

	for (i = 0; i < item.length; i ++)
	{
		// Starting box
		x = item.posX;
		y = item.posY;
		x += i;

		// Do nothing for letters that didn't change
		if (item.prevWord [0] != 0 && item.prevWord [i] == item.word [i]) continue;

		// Move on the top 
		do
		{
			y --;
			if (y < 0 || x < 0) break;

			pCase = pGrid->operator ()(x, y);
			if (pCase->IsBloc () == true) break;
			if (pCase->GetLetter () == 0) pCase->ResetCandidates (true);
		}
		while (true);


		// Starting box
		x = item.posX;
		y = item.posY;

		x += i;
		
		// Move to the bottom
		do
		{
			y ++;
			if (y >= mSy || x >= mSx) break;

			pCase = pGrid->operator ()(x, y);
			if (pCase->IsBloc () == true) break;

			if (pCase->GetLetter () == 0) pCase->ResetCandidates (true);
		}
		while (true);
	}
}



// ===========================================================================
/// \brief	Check if it is still possible to put words on the grid crossing a given word
///
/// This function enables to stop as soon as possible in case of dead lock and to return
/// at which position there is a conflict. The conflicting letter is then invalidated.
///
/// \param			pItem			Target item
/// \param[out]		pBestPos		Best letter index that still allows to put a crossword
/// \param			checkInMain		Use main dictionary
/// \param			checkInThema	Use theme dictionary
///
/// \return			True if every word letter fits.
// ===========================================================================
bool SolverStatic::CheckItemCross (StaticItem &item, int *pBestPos)
{
	uint8_t word [MAX_GRID_SIZE + 1];

	// Go through the whole word
	for (int i = 0; i < item.length; i ++)
	{
		// Check if we have already cross tested this letter
		if (item.IsCrossTested (i, item.word[i]) == true) continue;
		
		// Update mask with the letter that would be at the intersection
		// (pItem is not on the grid, so the mask should be updated)
		if (m_crossMasks [i].len <= 1) continue;
		m_crossMasks [i].mask [m_crossMasks [i].backOffset] = item.word [i];

		// Can we find a word ?
		if (pDict->FindEntry (word, m_crossMasks [i].mask) == true)
		{
			item.SetCrossCandidate (i, item.word [i], true);
			continue;
		}			

		// Invalidate the failing letter if no possibility
		item.SetCandidate (i, item.word [i], false);
	
		// Return best position
		if (pBestPos != nullptr && *pBestPos < i-1) *pBestPos = i-1;
		return false;

	} // for i

	return true;
}


// ===========================================================================
/// \brief	Build all the cross masks for a given item according to the current
///			grid content
///
/// \param		item			Target item
// ===========================================================================
void SolverStatic::BuildCrossMasks (StaticItem &item)
{
	// Go through the whole word
	for (int i = 0; i < item.length; i ++)
	{
		// Get the current letter position on the grid (x, y)
		int x = item.posX + i;
		int y = item.posY;

		// Compute mask for the crossword, along with its offset relative to 'item'
		m_crossMasks [i].backOffset = pGrid->BuildMask (m_crossMasks [i].mask, x, y, 'V', true);
		m_crossMasks [i].len = Len2 (m_crossMasks [i].mask);

		// Check the crossword is not already completely defined, if so skip it
		int j = 0;
		while (m_crossMasks [i].mask [j] != 0 && m_crossMasks [i].mask [j] != '*') j ++;
		if (m_crossMasks [i].mask [j] == 0) m_crossMasks [i].len = 0;
	}
}


// ===========================================================================
/// \brief	Pre build the list of all the grid word slots to solve and order them.
///
/// Slots are known in advance as this solver doesn't add black boxes. 
/// Ordering is done by maximazing the linkage between 2 successive words, 
/// in order to ease backtracking work.
// ===========================================================================
void SolverStatic::BuildWordList ()
{
	// Build horizontal slots list
	// First call enable to know the number of slots
	m_numItems = BuildWords (nullptr, 0);

	// Allocate memory and actually build the list
	m_pItemList = new StaticItem [m_numItems];
	BuildWords (m_pItemList, m_numItems);

	// Compute list order
	int idxLast = -1;
	int processOrder = 0;
	while (true)
	{
		// Find a wor to start if needed
		if (idxLast < 0)
		{
			idxLast = FindWordToStart (m_pItemList, m_numItems);
			if (idxLast < 0) break;
		}

		// Or find a next one
		else idxLast = FindWordNext (m_pItemList, m_numItems);		
		if (idxLast < 0) break;

		// Set processing order
		m_pItemList [idxLast].processOrder = processOrder ++;

		// Update strength of all the words not already selected
		for (int i = 0; i < m_numItems; i++)
		{
			if (m_pItemList [i].processOrder >= 0) continue;
			m_pItemList [i].connectionStrength += AreDependant (m_pItemList [idxLast], m_pItemList [i], nullptr);
		}
	} 

	// Sort word list according to their process order
	SortWordList (m_pItemList, m_numItems);

	// Exclude words already on the grid from the list
	/*ExcludeDefinedWords (m_pItemList, m_numItems); */
}


// ===========================================================================
/// \brief	Build the list of all the grid horizontal words/slots
/// 
/// \param[out]		pList			List of words/slots to fill in.
/// \param			listLength		Number of items in the list
///
/// Number of items detected in the grid. Can be bigger than 'listLength'
// ===========================================================================
int SolverStatic::BuildWords (StaticItem pList [], int listLength)
{
	int i = 0, j = 0;
	int idx = 0;
	bool openSlot = false;

	// Process each row
	for (j = 0; j < mSy; j ++)
	{
		// Move along the row
		for (i = 0; i < mSx; i ++)
		{
			Box *box = pGrid->operator ()(i, j);

			// Is it the begining of a word/slot ?
			if (openSlot == false && box->IsBloc () == false)
			{
				openSlot = true;

				// Add it to the list if possible
				if (idx < listLength)
				{
					pList [idx].posX = i;
					pList [idx].posY = j;
				}

				idx ++;
			}

			// Detect black boxes at the end of the current processed slot
			else if (box->IsBloc () == true && openSlot == true)
			{
				// Write slot length
				if (idx - 1 < listLength)	pList [idx - 1].length = i - pList [idx - 1].posX;
				openSlot = false;
			}
		}

		// Close last slot of this row, if any
		if (openSlot == true)
		{
			if (idx - 1 < listLength) pList [idx - 1].length = i - pList [idx - 1].posX;
			openSlot = false;
		}
	}

	return idx;
}


// ===========================================================================
/// \brief	Find next word to continue the processing ordering
///
/// \param			wordList		List of words/slots
/// \param			listLength		Number of items in the list
///
/// \return			Index of the slot to consider in our list.
// ===========================================================================
int SolverStatic::FindWordNext (StaticItem wordList [], int listLength)
{
	int best = -1;
	int bestScore = 1;

	for (int i = 0; i < listLength; i ++)
	{
		// Skip a word if it already has a process order
		if (wordList [i].processOrder >= 0) continue;

		// Take one with greatest length
		if (wordList [i].connectionStrength > bestScore)
		{
			bestScore = wordList [i].connectionStrength;
			best = i;
		}
	}

	return best;
}


// ===========================================================================
/// \brief	Find a first word to start the processing ordering
///
/// \param			wordList		List of words/slots
/// \param			listLength		Number of items in the list
///
/// \return			Index of the slot to consider in our list.
// ===========================================================================
int SolverStatic::FindWordToStart (StaticItem wordList [], int listLength)
{
	int best = -1;
	int bestLength = 0;

	for (int i = 0; i < listLength; i ++)
	{
		// Skip a word if it already has a process order
		if (wordList [i].processOrder >= 0) continue;

		// Take one with greatest length
		if (wordList [i].length > bestLength)
		{
			bestLength = wordList [i].length;
			best = i;
		}
	}

	return best;
}


// ===========================================================================
/// \brief	(Buble) Sort the word list according to the 'processOrder' field
/// 
/// \param[in,out]	wordList		List of words/slots
/// \param			listLength		Number of items in the list
// ===========================================================================
void SolverStatic::SortWordList (StaticItem wordList [], int listLength)
{
	int i,j;
	StaticItem swapItem;

	for (i = 0; i < listLength-1; i ++)
	{
		// Already in place ?
		if (wordList [i].processOrder == i) continue;

		// Else ...
		for (j = i+1; j < listLength; j ++)
		{
			if (wordList [j].processOrder == i)
			{
				// Swap i,j
				swapItem = wordList [j];
				wordList [j] = wordList [i];
				wordList [i] = swapItem;

				break;
			}
		}
	}
}


// ===========================================================================
/// \brief	Exclude words that already exist on the grid from our working list
///
/// \param[in,out]	pList			List of words/slots
/// \param			listLength		Number of items in the list
// ===========================================================================
void SolverStatic::ExcludeDefinedWords (StaticItem pList [], int listLength)
{
/*	int i, j;
	uint8_t pmask [MAX_GRID_SIZE+1];
	bool defined = true;
	
	// Process each slot
	for (i = 0; i < listLength-1; i ++)
	{
		// Build corresponding mask
		defined = true;
		pGrid->BuildMask (pmask, pList[i].posX, pList[i].posY, 'H', false);

		// Test mask completeness
		for (j = 0; j < Len2 (pmask); j ++)
		{
			if (pmask [j] == '*') defined = false;
		}

		// Tag the item if needed
		//if (defined == true) pList[i].dictStep = C_StaticItem2::Ce_Skip;
	}*/
}


// ===========================================================================
/// \brief	Detect if two words see each other through a common crossword
/// 
/// \param		item1			First word/slot to test
/// \param		item2			Second word/slot to test
/// \param[out]	dependencyMask	Mask (relative to item1), giving dependency
///								between both items. 
///								'*' / '.' means dependency / no dependency for this letter
///
/// \return		Number of dependent letters
// ===========================================================================
int SolverStatic::AreDependant (const StaticItem &item1, const StaticItem &item2, uint8_t* dependencyMask)
{
	unsigned char i1_xStart, i1_y, i1_xEnd;
	unsigned char i2_xStart, i2_y, i2_xEnd;
	int connect = 0;
	int i, j;

	// Start and end coordinates for item 1
	i1_xEnd = i1_xStart = item1.posX;
	i1_y = item1.posY;
	i1_xEnd += item1.length -1;

	// Start and end coordinates for item 2
	i2_xEnd = i2_xStart = item2.posX;
	i2_y = item2.posY;
	i2_xEnd += item2.length -1;

	// Clear mask
	if (dependencyMask != nullptr) 
	{
		for (i = 0; i < item1.length; i++)
			dependencyMask [i] = '.';
		dependencyMask [i] = 0;
	}

	// Assume words are horizontal and check overlapping
	if ( (i1_xEnd >= i2_xStart) && (i1_xStart <= i2_xEnd) ) 
	{	
		// Start and end Coordinates of the adjacent letters
		int s = (i1_xStart > i2_xStart) ? i1_xStart : i2_xStart;
		int e = (i1_xEnd < i2_xEnd) ? i1_xEnd : i2_xEnd;
		int step = (i1_y < i2_y) ? 1 : -1;

		// Check each boxes for the adjacent letters
		for (i = s; i <= e; i ++)
		{
			// Test the vertical path between both words
			for (j = i1_y +step; j != i2_y; j+= step)
			{
				if ( (*pGrid) (i, j)->IsBloc () ) break;
			}

			// If no block in between
			if (j == i2_y) 
			{
				// Mark the mask with a '*'
				if (dependencyMask != nullptr) dependencyMask [i - i1_xStart] = '*';
				connect ++;
			}		
		}
	} // if

	return connect;
}


// ===========================================================================
/// \brief	Convert letter index of a given word in grid coordinate
/// 
/// \param		item		Target word/slot
/// \param		letterIdx	Index of the letter in the word
///
/// \return		Grid coordinate (along word/slot direction)
// ===========================================================================
inline int SolverStatic::LetterIdx2Offset (const StaticItem &item, int letterIdx)
{
	return item.posX + letterIdx;
}


// ===========================================================================
/// \brief	Convert a grid coordinate into a letter index of a given word
/// 
/// \param		item		Target word/slot
/// \param		offset		Grid coordinate (along word/slot direction)
///
/// \return		Index of the corresponding letter in the word
// ===========================================================================
inline int SolverStatic::Offset2LetterIdx (const StaticItem &item, int offset)
{
	return offset - item.posX;
}


// End
