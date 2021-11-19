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
/// \file		SolverDynamic.cpp
/// \author		Jean-Sebastien Gonsette
///
/// \brief		Solver with dynamic black boxes placement
// ###########################################################################

#include <stdlib.h>
#include <string.h>
#include <chrono>
#include "Solvers/SolverDynamic.h"
#include "Solvers/SolverDynamic.DynamicItem.h"


// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief		Constructor
// ===========================================================================
SolverDynamic::SolverDynamic ()
{
	pItemList = nullptr;
	pItemUnused = nullptr;

	heurestic = true;
	stepBack = 3;
	maxBlackCases = -1;
	initialBlackCases = 0;

	seed = 0;
	densityMode = Grid::DIAG;
}


// ===========================================================================
/// \brief		Destructor
// ===========================================================================
SolverDynamic::~SolverDynamic ()
{
	FreeItems ();
}


// ===========================================================================
/// \brief		Start the grid generation process
///
/// \param		grid	Grid to solve
/// \param		dico	Dictionary to use
// ===========================================================================
void SolverDynamic::Solve_Start (Grid &grid, const Dictionary &dico)
{
	Solve_Stop ();

	// Use given grid and dictionary
	this->pDict = &dico;
	this->pGrid = &grid;
	this->mSx = pGrid->GetWidth ();
	this->mSy = pGrid->GetHeight ();
	this->pGrid->SetDensityMode (this->densityMode);

	// Lock non empty boxes
	pGrid->LockContent ();

	// Get initial number of black boxes
	initialBlackCases = pGrid->GetNumBlackCases ();

	// Init step counter and rng
	this->steps = 0;
	srand (static_cast<uint32_t> (this->seed));
}


// ===========================================================================
/// \brief		Stop the grid generation process
// ===========================================================================
void SolverDynamic::Solve_Stop ()
{
	// Unlock all grid boxes	
	if (pGrid != nullptr) pGrid->Unlock ();
	
	this->pDict = nullptr;
	this->pGrid = nullptr;
	this->steps = 0;
	FreeItems ();
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
Status SolverDynamic::Solve_Step (int32_t maxTimeMs, int32_t maxSteps)
{
	uint8_t x, y;
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

	// Processing loop
	while (true)
	{
		// Find out the next empty box to consider
		if (FindFreeBox (&x, &y) == false)
		{
			FreeItems ();
			break;
		}

		// Room around the next box to process
		Grid::Space space = pGrid->GetSpace (x, y);

		// Pick up a dynamic item to put on the grid
		DynamicItem* pItem = PopUnusedItem ();
		if (pItem == nullptr) pItem = new DynamicItem ();
		pItem->Reset ();

		// Item context: set position and get letter candidates from the grid
		pItem->posX = x - space.left;
		pItem->posY = y;
		pItem->LoadCandidatesFromGrid (*this->pGrid);

		// Find a first solution for this item
		unsigned int subCounter;
		int validatedCol;
		bool result = ChangeItem (pItem, false, -1, &validatedCol, &subCounter);

		this->steps += subCounter;
		pItem->SaveCandidatesToGrid (*this->pGrid);

		// If we have something, add the item to the backtrack list
		if (result) AddItem (pItem);

		// Otherwise, we need to backtrack
		else
		{
			// Get the row where we failed. The box that was a problem is at
			// coordiantes (validatedCol +1, validatedRow)
			int validatedRow = pItem->posY;

			// Put the item into the trash
			PushUnusedItem (pItem);

			// Backtracking to update a prior item
			pItem = Backtrack (validatedRow, validatedCol);
		}

		// Add current item to grid
		if (pItem != nullptr)
		{
			pItem->AddToGrid (*this->pGrid);
			pItem->ResetCandidatesBelowItem (*this->pGrid);
		}
		
		// If no item, it means the grid generation failed
		else
		{
			FreeItems ();
			pDict = nullptr;
			pGrid->Erase ();
			break;
		}

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


// ===========================================================================
/// \brief		Set the parameters of the heuristic mecanism used during the 
///				generation process
///
/// \param		state		True to activate the heuristic
/// \param		stepBack	Number of steps back when solver is blocked
// ===========================================================================
void SolverDynamic::SetHeurestic (bool state, int stepBack)
{
	this->heurestic = state;
	this->stepBack = stepBack;
}



// ###########################################################################
//
// P R I V A T E
//
// ###########################################################################

// ===========================================================================
/// \brief	Go back in the generation process by modifying items that are
///			already on the grid. We try to update an item that impact
///			a given location on the grid.
///
/// \param	valRow, valCol		Coordinate of the last box we reached. 
///								Try to impact next box on the right.
///
/// \return	Item in the backtracking list that has been updated, or
///			null in case of failure.
// ===========================================================================
SolverDynamic::DynamicItem* SolverDynamic::Backtrack (int valRow, int valCol)
{
	DynamicItem* pItem = nullptr;
	bool changeLength;
	int targetCol = -1;

	// Remove items from backtrack list until we can change one successfuly
	while (pItem == nullptr)
	{
		// Remove items up to a given point, as given by valRow and valCol
		do
		{
			// Remove last item from list and from grid
			pItem = GetLastItem ();
			if (pItem == nullptr) break;
			pItem->RemoveFromGrid (*this->pGrid);

			changeLength = false;
			targetCol = -1;

			// We need to remove one item only ?
			if (valRow == -1) break;

			// Item on the same target row ? low interaction -> stop only if there is no heuristic
			else if (pItem->posY == valRow && this->heurestic == false)
			{
				// Force to change length, otherwise it would not change anything
				changeLength = true;
				break;
			}

			// Strong interaction (item has incidence on 'valCol +1') ?
			else if (pItem->posY < valRow && pItem->posX <= (valCol + 1))
			{
				targetCol = valCol + 1;
				break;
			}

			pItem = RemoveLastItem ();
			PushUnusedItem (pItem);
		} while (true);
		if (pItem == nullptr) break;

		// Now try to change the word. We can force to change length
		unsigned int counter;
		bool r = ChangeItem (pItem, changeLength, targetCol, &valCol, &counter);

		// Update counter and candidates
		this->steps += counter;
		pItem->SaveCandidatesToGrid (*this->pGrid);

		// Failed ?
		if (r == false)
		{
			pItem = RemoveLastItem ();
			PushUnusedItem (pItem);

			// Manage the update marker 
			if (pItem->posY < valRow)
			{
				// When heuristic is activated, as soon as we reach our target column, we seek 
				// to reach the next validation point to accelerate backtracking
				if (this->heurestic)
				{
					valRow = pItem->posY;
					valCol -= this->stepBack;
					if (valCol < pItem->posX) valCol = pItem->posX;
					if (valCol < 0) valCol = -1;
				}

				// We have reached our target point, continue normally
				else valRow = -1;
			}

			pItem = nullptr;
		}
	}

	return pItem;
}


// ===========================================================================
/// \brief	Change the content of an item in the backtracking list.
///
/// The overal strategy to test all the possibilities of a word on the grid
/// is to choose an initial length, test all the words satisfying the constraints,
/// then to change the length and retry. If no any possibility is found, the last
/// option is to try to generate a black box instead of a word. 
///
/// \param	pItem			Item to change
/// \param	changeLength	True to force the length to change, 
///							instead of trying the next word of the current length
/// \param	colToChange		Absolute position (along the row) of the letter that must change. (-1 if not needed)
/// \param	pValidatedCol	Best absolute position that we could reach while trying to choose a word to put on the grid.
///							Letter of a word are checked from the left to right. 
///							The check consits in to be sure a cross word can still be layout on the grid at each letter position.
/// \param	pNumAttempts	Counter giving the number of words we tried before finding a suitable solution.
///
///	Return:					True if we could find a suitable solution.
// ===========================================================================
bool SolverDynamic::ChangeItem (DynamicItem *pItem, bool changeLength, int colToChange, int *pValidatedCol, unsigned int* pCounter)
{
	enum Steps { CHOOSE_LENGTH, CHANGE_WORD, CHANGE_LENGTH, CHANGE_BLOCK, DONE, FAILED };

	Grid::Space space;						///< Room around word origin
	uint32_t stepCounter = 0;				///< Step counter
	int unvalidatedIdx = -1;				///< Word letter to force to change
	uint8_t mask [MAX_GRID_SIZE + 1];		///< Grid content at the item location

	// Identify what to do
	Steps step = CHANGE_WORD;
	if (changeLength) step = CHANGE_LENGTH;
	if (pItem->length == 0) step = CHOOSE_LENGTH;

	// Cannot do anything else as the black box is the last thing we try
	if (pItem->isBlock) step = FAILED;

	// Get room around the box at the origin of this word
	space = pGrid->GetSpace (pItem->posX, pItem->posY);

	// Take count of the request to change the word at a given ABSOLUTE position
	if (colToChange >= 0 && pItem->word [0] != 0 && step == CHANGE_WORD)
	{
		unvalidatedIdx = colToChange - pItem->posX;

		// If this position is the black box at the end of the word, then change the length
		if (unvalidatedIdx == pItem->length)
		{
			step = CHANGE_LENGTH;
			unvalidatedIdx = -1;
		}
		// Otherwise, check the letter to change is not already invalide
		else if (unvalidatedIdx < pItem->length) {
			if (pItem->IsCandidate (unvalidatedIdx, pItem->word [unvalidatedIdx]) == false)
				unvalidatedIdx = -1;
		}
	}

	// Search loop
	while (step < DONE)
	{
		stepCounter ++;

		// If length not defined, choose one and ask for validation
		if (step == CHOOSE_LENGTH)
		{
			pItem->lengthFirstWord = 0;
			pItem->length = GetInitialLength (pItem->posY, space.left + 1 + space.right);

			// Request to change length (that will be shrinked, so the +1)
			pItem->length += 1;
			step = CHANGE_LENGTH;
		}

		// Try to change the word content. If it doesn't work, we have to change the length
		else if (step == CHANGE_WORD)
		{
			// Get the mask at the word position
			pGrid->BuildMask (mask, pItem->posX, pItem->posY, 'H', false);
			mask [pItem->length] = 0;

			// Try to find something matching the constraints
			bool result = ChangeItemWord (pItem, mask, unvalidatedIdx);
			unvalidatedIdx = -1;

			// If a word is found, cross check every letter. 'bestPos' memorize how far we went.
			if (result)
			{
				result = CheckItemCross (pItem, &pItem->bestPos);
				if (result) step = DONE;
			}
			// Otherwise this length has been dried out
			else step = CHANGE_LENGTH;
		}

		// Try to change the length
		else if (step == CHANGE_LENGTH)
		{
			bool result = ChangeItemLength (pItem, space.left + 1 + space.right);
			pItem->word [0] = 0;
			pItem->firstWord [0] = 0;

			// If we could not change the length, try with a block (last solution)
			if (result == false) step = CHANGE_BLOCK;
			else step = CHANGE_WORD;
		}

		// Try to put a black box at the word position
		else if (step == CHANGE_BLOCK)
		{
			pItem->length = 0;
			bool result = CheckItemLength (pItem);

			if (result)
			{
				pItem->isBlock = true;
				pItem->lengthFirstWord = 0;
				step = DONE;
			}
			else step = FAILED;
		}
	}

	if (pValidatedCol != nullptr) *pValidatedCol = pItem->posX + pItem->bestPos;
	if (pCounter != nullptr) *pCounter = stepCounter;

	if (step == DONE) return true;
	else return false;
}


// ===========================================================================
/// \brief	Find a new word solution for a given item on the grid. It must have the required
///			length, it must match the letter on the grid and respect the letter candiates
///
/// \param			pItem			Target item
/// \param			mask			Mask giving the grid content at the 'pItem' position. 
///									Implicitly gives the required word length
/// \param			unvalidatedIdx	Index of the first letter that must be mandatory changed
///
/// \return		True in case of success -> pItem->word contains a new solution.
// ===========================================================================
bool SolverDynamic::ChangeItemWord (DynamicItem *pItem, uint8_t mask [], int unvalidatedIdx)
{
	bool loopStatus = false;

	// Enable to detect we looped completely over the dictionary
	if (pItem->word [0] != 0 && pItem->firstWord [0] != 0) {
		if (pDict->Compare (pItem->word, pItem->firstWord) < 0) loopStatus = true;
	}

	// If we must change a given letter ?
	if (unvalidatedIdx >= 0)
	{
		// Force letters coming after to the last possibility -> next
		// word will have  letter at the given index increased
		for (int i = unvalidatedIdx + 1; i < pItem->length; i ++)
			pItem->word [i] = pDict->AlphabetSize ();
	}

	// Search loop
	while (true)
	{	
		// If it is the first time we try, choose begining at random
		if (pItem->word [0] == 0)
		{
			pItem->word [0] = 1 + rand () % pDict->AlphabetSize ();
			pItem->word [1] = 1 + rand () % pDict->AlphabetSize ();
			pItem->word [2] = 1 + rand () % pDict->AlphabetSize ();
			pItem->word [3] = 0;
			pItem->word [pItem->length] = 0;
		}

		// Look for something in the dictionary
		bool found = pDict->FindEntry (pItem->word, mask, pItem->word, pItem->candidates);

		// If nothing found, restart at the begining of the dictionary
		// (can only be done once)
		if (found == false)
		{
			if (loopStatus == true) return false;
			loopStatus = true;

			pItem->word [0] = 0;
			found = pDict->FindEntry (pItem->word, mask, pItem->word, pItem->candidates);
		}

		// Could not find anything ?
		if (found == false) return false;

		// If we find back the starting word (or go beyond), we fail
		if (loopStatus && pItem->firstWord [0] != 0 && 
			(pDict->Compare (pItem->word, pItem->firstWord) >= 0)) return false;

		// Check the word is not already on the grid
		/// \todo

		// All conditions match
		break;
	}

	// Memorize the first valid word we found, to detect when all dictionary words have been exhausted
	if (pItem->firstWord [0] == 0) memcpy (pItem->firstWord, pItem->word, (size_t) pItem->length + 1);
	return true;
}


// ===========================================================================
/// \brief	Reduce word length until it fits. If too short, loopback on
///			the maximum length. If we reach the first length we tried, fail.
///
/// \param	pItem		Target word
/// \param	lengthMax	Max length to try when looping back
///
/// \return	True in case of success -> pItem->length contains a new valid length
// ===========================================================================
bool SolverDynamic::ChangeItemLength (DynamicItem *pItem, int lengthMax)
{
	// Search loop
	while (true)
	{
		// Shrink length
		pItem->length --;

		// If too short, loop back
		if (pItem->length == 0) {
			if (lengthMax <= 0) return false;
			pItem->length = lengthMax;
		}

		// We already tried every length ?
		if (pItem->length == pItem->lengthFirstWord) return false;

		// Set 'lengthFirstWord' if not defined already
		if (pItem->lengthFirstWord == 0) pItem->lengthFirstWord = pItem->length;

		// Check this length is admissible
		if (CheckItemLength (pItem) == true) return true;
	} 
}


// ===========================================================================
/// \brief		Find out the size of the biggest compatible word next an hypothetic block, 
///				on its right, left, top and bottom sides
///
/// \param	x, y		Coordinate of the block to test. Can be out the grid on the border.
/// \return				Longest word on any side of the block.
///						0: not possible to put a block in x,y, 
///						-1: possible but no room
///						>0: length of the word
// ===========================================================================
Grid::Space SolverDynamic::CheckGridBlock (int x, int y)
{
	Grid::Space space = {-1, -1, -1, -1};
	int i;
	int back;								///< Where does the mask start (relative to x, y)
	uint8_t mask [MAX_GRID_SIZE + 1];		///< Buffer to extract the grid content
	uint8_t word [MAX_GRID_SIZE + 1];		///< Buffer to extract word possibility

	// Out of grid and not on the direct border ?
	if (x > mSx || x < -1) return space;
	if (y > mSy || y < -1) return space;

	// Already a block ?
	Box *box = pGrid->operator ()(x, y);
	if (box->IsBloc () == true || box->IsVoid ()) return space;

	// A letter is already here, we can't put a block
	if (box->IsLetter () && box->GetLetter () != 0)
	{
		space = {0, 0, 0, 0};
		return space;
	}

	// Test vertical and horizontal directions
	for (int dir = 0; dir < 2; dir ++)
	{
		// Skip if out of grid
		if (dir == 1 && (x >= mSx || x <= -1)) continue;
		if (dir == 0 && (y >= mSy || y <= -1)) continue;

		// Build a mask with the grid content, overlapping the x, y position. 
		// 'back' gives the relative offset of the begining of the mask
		if (dir == 0) 
		{
			if (x >= 0)	back = pGrid->BuildMask (mask, x, y, 'H', true);
			else {
				back = -1;
				pGrid->BuildMask (mask, 0, y, 'H', true);
			}
		}
		else 
		{
			if (y >= 0) back = pGrid->BuildMask (mask, x, y, 'V', true);
			else {
				back = -1;
				pGrid->BuildMask (mask, x, 0, 'V', true);
			}
		}
		int len = -1;
		while (mask [++len]);

		// Try various word length to put before (x, y)
		// - Put a block in (x, y)
		if (back < len && back >= 0) mask [back] = 0;
		for (i = 0; i < back; i++)
		{
			// If box in 'i'-1 contains a letter, not possible to START a new word in 'i'
			if (i > 0 && mask [i - 1] != '*') continue;

			// Word of a single letter, this is legal
			if ((back - i) <= 1) break;

			// Look for a word starting in 'i' and complying with the mask
			if (pDict->FindEntry (word, mask + i) == true) break;
		}

		// - Remove block in (x,y)
		if (back < len && back >= 0) mask [back] = '*';

		// - Write result
		if (dir == 0) space.left = (back == 0 ? -1 : back - i);
		if (dir == 1) space.top = (back == 0 ? -1 : back - i);

		// Try various word length after (x,y)
		for (i = len; i > back + 1; i --)
		{
			// If box in 'i' contains a letter, not possible in END in 'i'
			if (i != len && mask [i] != '*') continue;

			// Word of a single letter, this is legal
			if (i - back - 1 <= 1) break;

			// Write a block in 'i' and look for something
			mask [i] = 0;
			if (pDict->FindEntry (word, mask + back + 1) == true) break;
		}

		// - Write result
		if (dir == 0) space.right = (back >= len-1 ? -1 : i -back -1);
		if (dir == 1) space.bottom = (back >= len-1 ? -1 : i -back -1);
	}
	return space;
}


// ===========================================================================
/// \brief		Return the initial length to test for a given row
///
///				For the 2 first row we take it at random. But after
///				we always start with the maximum length
///
/// \param	row			Target row
/// \param	maxLength	Max. possible length
// ===========================================================================
int SolverDynamic::GetInitialLength (int row, int maxLength)
{
	if (row < 2)
	{
		if (maxLength > 8) maxLength = 8;
		int l = rand () % (maxLength);
		return l + 1;
	}
	else return maxLength;
}


// ===========================================================================
/// \brief	Verify pushing a word on the grid is possible, by checking
///			it is still possible to put at least one crossed-word at every
///			letter position
///
///	\param	pItem				Word to test (assumed to be an horizontal word)
/// \param	pBestPos[in, out]	Index of the best letter we could validate
// ===========================================================================
bool SolverDynamic::CheckItemCross (DynamicItem *pItem, int *pBestPos)
{
	uint8_t mask [MAX_GRID_SIZE + 1];
	uint8_t word [MAX_GRID_SIZE + 1];

	// Check every letter in order
	for (int i = 0; i < pItem->length; i ++)
	{
		// Position of the letter to check
		int x = pItem->posX + i; 
		int y = pItem->posY;

		// Mask in x,y, in the perpendicular orientation.
		// Update y to match the origin of the mask
		int back = pGrid->BuildMask (mask, x, y, 'V', true);
		y -= back;

		// Detect the case where all the letters are already defined.
		// In this case this is valid and there is nothing more to test
		int j = 0;
		while (mask [j] != 0 && mask [j] != '*') j ++;
		if (mask [j] == 0) continue;

		// pItem is not on the grid already -> add the letter under test to the mask
		mask [back] = pItem->word [i];
		int length = -1;
		while (mask [++length]);

		// Try different length, from the biggest (as given by the mask), to the 
		// shorter (as given by 'back' offset)
		for (j = length - 1; j >= back; j --)
		{
			// Skip if the box where we try to put a black box is already defined
			if (mask [j + 1] != '*' && (j + 1 < length)) continue;

			// Word of one letter is possible
			if (j + 1 <= 1) break;

			// Ensure it is possible to put a back box in 'j+1'
			if (j + 1 < length)
				if (pGrid->CheckBlocDensity (x, y + j + 1) == false) continue;

			// Truncate the mask to simulate the black box
			mask [j + 1] = 0;

			// Can we find somehting in the dictionary ?
			if (pDict->FindEntry (word, mask) == true) break;
		}

		// Failed ?
		if (j < back)
		{
			// Invalidate the 'i'e letter of pItem, as obviously it doesn't work
			pItem->SetCandidate (i, pItem->word [i], false);

			// How far could be go -> udpate bestPos
			if (pBestPos != nullptr && *pBestPos < i - 1) *pBestPos = i - 1;
			return false;
		}
	}

	return true;
}


// ===========================================================================
/// \brief	Check if the length of a word doesn't block the grid generation process
///
/// \param	pItem	Word to check, assuming horizontal orientation.
// ===========================================================================
bool SolverDynamic::CheckItemLength (const DynamicItem *pItem)
{
	uint8_t x = pItem->posX + pItem->length;
	uint8_t y = pItem->posY;

	// Always possible to push a black box out of the grid
	if (x >= mSx || y >= mSy) return true;

	// Also possible to push a black box if it already exists 
	Box *box = pGrid->operator ()(x, y);
	if (box->IsBloc () == true || box->IsVoid ()) return true;

	// Fail if not possible to add a single black box
	if (maxBlackCases == 0) return false;

	// Check about local density
	if (pGrid->CheckBlocDensity (x, y) == false) return false;

	// Check about global density
	if (this->maxBlackCases >= 0)
	{
		// - Curve giving a progression of the number of black boxes from
		//   the begining to the end of the grid. 
		//   Form: ax^2+bx, going through (0,0), (t,e) and (1,1)
		float t = 0.5f, e = 0.5f;				///< Linear curve with those values
		float b = (e + t * t) / (t*t + t);
		float a = 1 - b;

		// - retrieve the number of non-locked boxes up to (x, y)
		int boxNumber = box->tag;

		// - Black boxes fill rate at this point, according to our curve
		float fillrate = (float)boxNumber / 
			(float)(mSx*mSy - 1 - initialBlackCases - pGrid->GetNumVoidBoxes ());
		fillrate = a * fillrate*fillrate + b * fillrate;

		// - Derive max number of black cases up to (x, y)
		float maxBlack = 1.0f + (float)(maxBlackCases - 1) * fillrate;
		int limit = (int)maxBlack;
		if (maxBlack - limit >= 0.5f) limit++;

		// - Check if we go beyond this limit
		if ((pGrid->GetNumBlackCases () - initialBlackCases + 1) > limit) return false;
	}

	// Check now if the black box at the end of the word is not a problem
	Grid::Space space = CheckGridBlock (x, y);
	if (space.left == 0 || space.right == 0 || space.top == 0 || space.bottom == 0) return false;
	if (space.left >0 && space.left != pItem->length) return false;

	return true;
}


// ===========================================================================
/// \brief		Free linked list of DynamicItem instances created during the process
// ===========================================================================
void SolverDynamic::FreeItems ()
{
	DynamicItem* pDelete;
	
	DynamicItem* p = pItemUnused;
	while (p != nullptr)
	{
		pDelete = p;
		p = p->pNext;
		delete pDelete;
	}

	p = pItemList;
	while (p != nullptr)
	{
		pDelete = p;
		p = p->pNext;
		delete pDelete;
	}

	pItemUnused = nullptr;
	pItemList = nullptr;
}


// ===========================================================================
/// \brief	Find the first grid free box
///
/// \param	px, py		Coordinates of the box
/// \return	True in case of success
// ===========================================================================
bool SolverDynamic::FindFreeBox (uint8_t *px, uint8_t *py) const
{
	uint8_t x, y;
	const Box *box = nullptr;

	// Look from left to right, then from top to bottom
	for (y = 0; y < mSy; y ++)
	{
		for (x = 0; x < mSx; x ++)
		{
			box = pGrid->operator ()(x, y);
			if (box->IsLetter () && box->GetLetter () == 0) break;
		}

		if (x < mSx) break;
	}

	// Found nothing ?
	if (y >= mSy) return false;

	if (px != nullptr) *px = x;
	if (py != nullptr) *py = y;
	return true;
}


// ===========================================================================
/// \brief	Push a word in the list where we keep all the unused words
///	This mechanism avoid frequent new/delete operations
///
/// \param	pItem		Item to push in the list
// ===========================================================================
void SolverDynamic::PushUnusedItem (DynamicItem* pItem)
{
	pItem->pNext = pItemUnused;
	pItemUnused = pItem;
}


// ===========================================================================
/// \brief	Get a word from the list of unused words
///	This mechanism avoid frequent new/delete operations
///
/// \param	return	A word from the list
// ===========================================================================
SolverDynamic::DynamicItem* SolverDynamic::PopUnusedItem ()
{
	DynamicItem* p = pItemUnused;

	if (p != nullptr) pItemUnused = p->pNext;
	return p;
}


// ===========================================================================
/// \brief	Get a word from the list of unused words
///	This mechanism avoid frequent new/delete operations
///
/// \param	return	A word from the list
// ===========================================================================
SolverDynamic::DynamicItem* SolverDynamic::RemoveLastItem ()
{
	DynamicItem* p = pItemList;
	DynamicItem* p2 = pItemList;

	if (p == nullptr) return nullptr;

	if (p->pNext == nullptr) pItemList = nullptr;
	else
	{
		while (p->pNext->pNext != nullptr) p = p->pNext;

		p2 = p->pNext;
		p->pNext = nullptr;
	}

	return p2;
}


// ===========================================================================
/// \brief	Return the last item of the backtracking list
// ===========================================================================
SolverDynamic::DynamicItem* SolverDynamic::GetLastItem ()
{
	DynamicItem* p = pItemList;

	if (p != nullptr)
		while (p->pNext != nullptr) p = p->pNext;

	return p;
}


// ===========================================================================
/// \brief	Add a word to the backtracking list in case of successful placement
///
/// \param	pItem		Item to add to the list
// ===========================================================================
void SolverDynamic::AddItem (DynamicItem* pItem)
{
	DynamicItem* p = pItemList;
	pItem->pNext = nullptr;

	// List is empty ?
	if (p == nullptr) pItemList = pItem;

	else 
	{
		// Find end of list
		while (p->pNext != nullptr) p = p->pNext;
		p->pNext = pItem;
	}
}

// End
