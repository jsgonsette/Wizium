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
	items = nullptr;
	numItems = -1;		
	idxCurrentItem = -1;

	heurestic = true;
	stepBack = 0;
}


// ===========================================================================
/// \brief		Destructor
// ===========================================================================
SolverStatic::~SolverStatic ()
{
	if (items != nullptr) delete [] items;
}


// ===========================================================================
/// \brief		Set the heuristic to use during the grid generation
///
/// An heuristic is an empirical method enabling to reduce problem complexity
/// by removing problem sub spaces that are likely to be unsuccessful.
////
/// \param	state			True to activate the heuristic
/// \param	stepBack		Heuristic level
// ===========================================================================
void SolverStatic::SetHeurestic (bool state, int stepBack)
{
	this->heurestic = state;
	this->stepBack = stepBack;
}


// ===========================================================================
/// \brief		Stop searching for a solution
// ===========================================================================
void SolverStatic::Solve_Stop ()
{
	// Unlock all grid boxes	
	if (pGrid != nullptr) pGrid->Unlock ();

	this->pDict = nullptr;
	this->pGrid = nullptr;
	this->steps = 0;
	numItems = -1;
	idxCurrentItem = -1;
}


// ===========================================================================
/// \brief		Start searching for a solution to fill in a grid.
///
/// \param		pGrid		Grid to solve
/// \param		pDict		Main dictionary to fill in the grid
// ===========================================================================
void SolverStatic::Solve_Start (Grid &grid, const Dictionary &dico)
{
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
	this->pGrid->LockContent ();

	// Establish a static ordered list of word slots for whose we must find a solution
	if (items != nullptr) delete [] items;
	BuildWordList ();
	idxCurrentItem = 0;

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

	// If we failed or stopped previously 
	if (pGrid == nullptr || pDict == nullptr)
	{
		status.counter = this->steps;
		status.fillRate = 0;
		return status;
	}

	// Main search loop, we have finished when we have found something for every slots in our list
	while (this->idxCurrentItem < this->numItems)
	{		
		// Get item to solve during this iteration
		StaticItem *pItem = &items [idxCurrentItem];

		// Reset 
		pItem->Reset ();
		LoadCandidatesFromGrid (*pItem);

		// Find a first solution for this item
		unsigned int subCounter;
		bool result = ChangeItem (*pItem, -1, &subCounter);
		
		this->steps += subCounter;
		SaveCandidatesToGrid (*pItem);

		// If no solution, backtrack up to a previous item
		if (result == false) BackTrack ();

		// Backtracking failed ?
		if (this->idxCurrentItem < 0)
		{
			pDict = nullptr;
			pGrid->Erase ();
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
// ===========================================================================
void SolverStatic::BackTrack ()
{
	unsigned int counter;
	uint8_t mask[MAX_GRID_SIZE + 1];

	// The first word we change must have direct interaction with the point of failure
	int targetCol = items [idxCurrentItem].posX + items [idxCurrentItem].bestPos + 1;
	int idxTarget = this->idxCurrentItem;

	// Reset visibility with the failing word
	for (int i = 0; i < idxCurrentItem; i++) items[i].visibility = false;
	items [idxCurrentItem].visibility = true;

	// Look for the next word to change
	int idx = this->idxCurrentItem;
	StaticItem *next = nullptr;

	while (next == nullptr)
	{
		// Try to backtrack 
		idx = BackTrackStep (idxTarget, targetCol, idx);
		if (idx < 0)
		{
			idxCurrentItem = -1;
			break;
		}
		next = &items [idx];

		// If no target and heuristic activated, determine the column we force to change
		if (this->heurestic == true && targetCol == -1)
		{
			// Take the column that collected the higest rate of failure
			targetCol = next->posX;
			for (int i = next->posX; i < next->posX + next->length; i++)
			{
				if ((*pGrid) (i, next->posY)->GetFailCounter () >
					(*pGrid) (targetCol, next->posY)->GetFailCounter ()) targetCol = i;
			}

			// Reset counter for this word
			for (int i = next->posX; i < next->posX + next->length; i++)
				(*pGrid) (i, next->posY)->ResetFailCounter ();

			// Push a bit more
			targetCol -= this->stepBack;
			if (targetCol < next->posX) targetCol = next->posX;
		}

		// Now try to change the item we went back to. 
		bool r = ChangeItem (*next, targetCol, &counter);
		
		// No more target
		idxTarget = targetCol = -1;

		// Update counter and candidates
		this->steps += counter;
		SaveCandidatesToGrid (*next);

		// Failed ?
		if (r == false) next = nullptr;
		else idxCurrentItem = idx;
	}
}


// ===========================================================================
/// \brief	Backtrack until we remove a word that has visibility, either
///			on a given target (idxTarget, targetCol), or on any other items
///			that has weak visibility on 'idxCurrentItem'
///
/// \param	idxTarget		Index of the element we want direct visibility to
///							-1: if no direct visibility wanted. In that case
///							we just check the 'visibility' flag of any 
/// \param	targetCol		If 'idxTarget' defined, column that must be visible
///							At return, actual value of the column to consider (maybe lesser in case of occlusion)
/// \param	idx				Current item to start from 
///
/// \return	Index of next item to change
// ===========================================================================
int SolverStatic::BackTrackStep (int idxTarget, int& targetCol, int idx)
{
	uint8_t mask[MAX_GRID_SIZE + 1];
	StaticItem *target = (idxTarget >= 0) ? &items[idxTarget] : nullptr;

	// Remove items until we meet our interaction criteria
	while (--idx >= 0)
	{
		// Remove word from grid and prepare to use it
		pGrid->RemoveWord (items[idx].posX, items[idx].posY, 'H');
		StaticItem *next = &items[idx];

		// If we look for strong interaction with a target word
		if (target != nullptr)
		{
			AreDependant (*next, *target, mask);

			// Strong interaction if any column up to 'targetCol' is visible
			bool strongInteraction = false;
			for (int i = targetCol; i >= next->posX; i--)
			{
				if (mask[i - next->posX] == '*')
				{
					strongInteraction = true;
					targetCol = i;
					break;
				}
			}
			if (strongInteraction) break;
		}

		// Otherwise, we just seek for weak interaction with any visible item
		else
		{
			// Check if 'idx' see any of the following visible items
			int i;
			for (i = idx + 1; i <= idxCurrentItem && items [i].visibility; i++)
				if (AreDependant (*next, items [i], mask)) break;
			if (i <= idxCurrentItem) break;
		}
	}

	// Set the visibility flag of the item we have found
	if (idx >= 0) items [idx].visibility = true;
	return idx;
}


// ===========================================================================
/// \brief	Change the content of an item in the backtracking list.
///
/// \param	item			Item to change
/// \param	colToChange		Absolute position (along the row) of the letter that must change. (-1 if not needed)
/// \param	pCounter		Counter giving the number of words we tried before finding a suitable solution.
///
///	Return:					True if we could find a suitable solution.
// ===========================================================================
bool SolverStatic::ChangeItem (StaticItem &item, int colToChange, unsigned int* pCounter)
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
		result = ChangeItemWord (item, mask, unvalidatedIdx, false);
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
bool SolverStatic::ChangeItemWord (StaticItem &item, uint8_t mask [], int unvalidatedIdx, bool strict)
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
		if (unvalidatedIdx >= 0 && strict && letterToChange == item.word [unvalidatedIdx]) continue;

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
	StaticItem *pItem = &items [idxCurrentItem];
	pGrid->AddWord (pItem->posX, pItem->posY, 'H', pItem->word);

	// For each updated letter, reset letter candidates on the cross boxes
	ResetCandidatesAround (items [idxCurrentItem]);

	// Save new reference to keep track of updated letters
	memcpy (pItem->prevWord, pItem->word, pItem->length+1);

	// This parameter is only valid in case of failure
	pItem->bestPos = -1;

	// Move on next item in our list, no more failing item
	idxCurrentItem ++;
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
		// ('item' is not on the grid, so the mask should be updated)
		if (crossMasks [i].len <= 1) continue;
		crossMasks [i].mask [crossMasks [i].backOffset] = item.word [i];

		// Can we find a word ?
		if (pDict->FindEntry (word, crossMasks [i].mask) == true)
		{
			item.SetCrossCandidate (i, item.word [i], true);
			continue;
		}			

		// Invalidate the failing letter if no possibility
		item.SetCandidate (i, item.word [i], false);

		// Propagate this failure through the column
		this->pGrid->FailAtColumn (item.posX + i, item.posY);
	
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
		crossMasks [i].backOffset = pGrid->BuildMask (crossMasks [i].mask, x, y, 'V', true);
		int len = -1;
		while (crossMasks [i].mask [++len]);
		crossMasks [i].len = len;

		// Check the crossword is not already completely defined, if so skip it
		int j = 0;
		while (crossMasks [i].mask [j] != 0 && crossMasks [i].mask [j] != '*') j ++;
		if (crossMasks [i].mask [j] == 0) crossMasks [i].len = 0;
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
	numItems = BuildWords (nullptr, 0);

	// Allocate memory and actually build the list
	items = new StaticItem [numItems];
	BuildWords (items, numItems);

	// Compute list order
	int idxLast = -1;
	int processOrder = 0;
	while (true)
	{
		// Find a wor to start if needed
		if (idxLast < 0)
		{
			idxLast = FindWordToStart (items, numItems);
			if (idxLast < 0) break;
		}

		// Or find a next one
		else idxLast = FindWordNext (items, numItems);		
		if (idxLast < 0) break;

		// Set processing order
		items [idxLast].processOrder = processOrder ++;

		// Update strength of all the words not already selected
		for (int i = 0; i < numItems; i++)
		{
			if (items [i].processOrder >= 0) continue;
			items [i].connectionStrength += AreDependant (items [idxLast], items [i], nullptr);
		}
	} 

	// Sort word list according to their process order
	SortWordList (items, numItems);
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
		for (; i < item2.posX + item2.length - item1.posX; i++)
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


// End
