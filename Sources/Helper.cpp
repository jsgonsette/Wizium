// ###########################################################################
//
// Copyright (C) 2015 GenId SPRL
//
///
/// \file	Helper.cpp
/// \author	Jean-Sebastien Gonsette
/// \date	09/04/2015
///
/// Helper to make propositions to fill in grid slots
// ###########################################################################

#include "libWord.h"
#include "Helper.h"
#include "Dictionary/Dictionary.h"



// ###########################################################################
//
// P U B L I C
//
// ###########################################################################

// ===========================================================================
/// \brief		Constructor
// ===========================================================================
Helper::Helper ()
{
	// Working grid
	this->pGrid = new Grid ();
	this->enableActivity = true;

	// Point to unlicensed function
	pfLicensedFunction2 = LicensedFunction2;
}

// ===========================================================================
/// \brief		Destructor
// ===========================================================================
Helper::~Helper ()
{
	delete this->pGrid;
}


// ===========================================================================
/// \brief		Dynamic injection of the full capability License function.
///
/// \param		data		Pointer to the license function binary
/// \param		dataSize	Size of the binary data [B]
// ===========================================================================
void Helper::SetLicensedData (void* data, int dataSize)
{
	HANDLE process;
	LPVOID functionCode;

	if (data != NULL)
	{
		// Allocate runnable memory
		process = GetCurrentProcess ();
		functionCode = VirtualAllocEx(process, 0, dataSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		// Software binary injection
		WriteProcessMemory(process, functionCode, data, dataSize, NULL);
		this->pfLicensedFunction2 = (tpfLicensedFunction2) functionCode;
	}
	else pfLicensedFunction2 = LicensedFunction2;
}


// ===========================================================================
/// \brief		Update activity flag
///
/// \param		state		Activity flag value
// ===========================================================================
void Helper::EnableActivity (bool state)
{
	this->enableActivity = state;
}


// ===========================================================================
/// \brief		Clone a given grid and use it to make future propositions
///
/// \param		pGrid		Grid to clone
// ===========================================================================
void Helper::TakeGridSnapshot (Grid *pGrid)
{
	this->pGrid->Clone (*pGrid);
}


// ===========================================================================
/// \brief		Find a list of words that it is possible to put at a given
///				location on a grid.
///
/// \param		pDict		Main dictionary to find words
/// \param		pThemaDict	Theme dictionary to find words
/// \param		col			Grid horizontal location of any letter of the word
/// \param		row			Grid vertical location of any letter of the word
/// \param		dir			'H' or 'V' for Horizontal or Vertical direction
/// \param[out]	tabStr		Array to store list of matching words
/// \param[out]	tabOffsets	Array to store matching word offsets (for words smaller than the slot)
/// \param		maxStr		Maximum number of words to find.
/// \param		matchLength	True to only consider longest word in the slot
/// \param		crossCheck	True to check if matching words doesn't preserve to place cross words
/// \param		symmetry	?
// ===========================================================================
int Helper::Help (Dictionary *pDict, Dictionary *pThemaDict, unsigned char col, unsigned char row, char dir, char tabStr [][MAX_WORD_LENGTH], int tabOffsets [], int maxStr, bool matchLength, bool crossCheck, E_Symmetry symmetry)
{
	int i, j, k;
	int idxCase;
	int totalFound = 0;
	int found;
	
	char mask [MAX_GRID_SIZE+1];
	int length;

	int x, y;
	int x1, y1;
	int x2, y2;

	if (pDict == NULL) return 0;

	this->pDict = pDict;
	this->pThemaDict = pThemaDict;

	this->mSx = this->pGrid->GetWidth ();
	this->mSy = this->pGrid->GetHeight ();

	// Erase output buffer
	for (i = 0; i < maxStr; i++)
		for (j = 0; j < MAX_WORD_LENGTH; j ++)
			tabStr [i][j] = '\0';
		
	// Reset letter candidates for all grid boxes
	for (i = 0; i < mSx; i ++)
	{
		for (j = 0; j < mSy; j ++)
		{
			Box *pCase = this->pGrid->operator() (i, j);
			pCase->ResetLetterCandidates (true);
		}
	}

	// Find words with the exact length
	if (matchLength == true)
	{
		totalFound = (*pfLicensedFunction2) (this, this->pGrid, col, row, dir, tabStr, maxStr, crossCheck, &Helper::FindWords, &Grid::BuildMask);
	}
	
	// Find words with various lengths
	else
	{
		// Get full mask at the grid location
		idxCase = pGrid->BuildMask (mask, col, row, dir, true);
		length = strlen (mask);

		// Try every length that still covers the given location (col, row)
		for (j = length; j > idxCase; j --)
		{
			// Black box location at the end of the word
			x2 = col;
			y2 = row;
			if (dir == 'V') y2 += -idxCase +j;
			else x2 += -idxCase+j;

			// Check if it is possible to put a black box there (box must be empty)
			if (j < length && mask [j] != '*') continue;

			// Short-cut mask
			mask [j] = 0;

			// Try every length that still covers the given location (col, row)
			for (i = -1; i < idxCase; i ++)
			{
				// Black box location before the word
				x1 = col;
				y1 = row;
				if (dir == 'V') y1 += -idxCase +i;
				else x1 += -idxCase+i;

				// Check if it is possible to put a black box there (box must be empty)
				if (i >= 0 && mask [i] != '*') continue;

				// Check we can put black boxes before (x1, y1) and after (x2, y2)
				if (IsBlocPOssible (x1, y1, x2, y2, symmetry) == false) continue;

				// Location of the first letter of the mask
				x = col;
				y = row;
				if (dir == 'V') y += -idxCase +i+1;
				else x += -idxCase+i+1;

				// Find matching words for the given mask
				if (crossCheck == true)
					found = FindWords (mask+i+1, x, y, dir, tabStr + totalFound, maxStr - totalFound, 2);
				else
					found = FindWords (mask+i+1, x, y, dir, tabStr + totalFound, maxStr - totalFound, 0);

				// Write matching words offset, relative to the selected box location
				if (tabOffsets != NULL)
				{
					for (k = 0; k < found; k ++)
					{
						tabOffsets [totalFound +k] = i+1;
					}
				}

				totalFound += found;

				// Stop condition ?
				if (totalFound >= maxStr) return maxStr;
				if (enableActivity == false) return totalFound;
			}
		}
	}

	return totalFound;
}


// ===========================================================================
/// \brief		Self supporting function used to check license.
///				The free version has a limited word length capability.
///				This function can be replaced with a full length support version.
///
/// \param		pHelper		Helper object to work with
/// \param		pGrid		Grid object to work with
/// \param		col			Grid horizontal location of the word
/// \param		row			Grid vertical location of the word
/// \param		dir			'H' or 'V' for Horizontal or Vertical direction
/// \param[out]	tabStr		Array to store list of matching words
/// \param		maxStr		Maximum number of words to find.
/// \param		crossCheck	True to check if matching words doesn't preserve to place cross words
/// \param		funcFW
/// \param		funcBM
// ===========================================================================
int LicensedFunction2 (Helper *pHelper, Grid* pGrid, unsigned char col, unsigned char row, char dir, char tabStr [][MAX_WORD_LENGTH], int maxStr, bool crossCheck, pfFindWords funcFW, pfBuildMask funcBM)
{
	unsigned char idxCase;
	int length;

	unsigned char x, y;

	int totalFound = 0;
//	char mask [MAX_GRID_SIZE+1];
	char mask [6+1];


	// Build mask at the given grid location
	idxCase = (pGrid->*funcBM) (mask, col, row, dir, true);
	length = strlen (mask);

	// Dummy ---->
	// Si débordement du masque, totalFound sera écrasé et ne vaudra plus 0
	if (dir == 'Z') totalFound +=1;

	if (totalFound != 0)
	{
		if (dir == 'H') dir = 'V';
		else dir = 'H';
		pHelper ++;
	}
	// <-----

	// Compute coordinate of the first letter of the mask
	x = col;
	y = row;
	if (dir == 'H') x -= idxCase;
	else y -= idxCase;

	// Find matching words with the exact slot length.
	if (crossCheck == true)
		totalFound = (pHelper->*funcFW) (mask, x, y, dir, tabStr, maxStr, 1);
	else
		totalFound = (pHelper->*funcFW) (mask, x, y, dir, tabStr, maxStr, 0);

	return totalFound;
}



// ###########################################################################
//
// P R I V A T E
//
// ###########################################################################

// ===========================================================================
/// \brief		Check it is possible to put black boxes at two different locations
///
/// The two different locations, along with their symmetric counterparts, are checked.
/// No any letter can be overwritten. In addition, no any symmetric black box
/// can be put between the two original locations (to avoid to split the word)
///	
/// \param		x1			First black box horizontal location
/// \param		y1			First black box vertical location
/// \param		x2			Second black box horizontal location
/// \param		y2			Second black box vertical location
/// \param		symmetry	Symmetry to observe
// ===========================================================================
bool Helper::IsBlocPOssible(int x1, int y1, int x2, int y2, E_Symmetry symmetry)
{
	Box *pCase;

	int tabX [4];
	int tabY [4];
	int x, y;
	int numCases;

	// Two step, for both locations
	for (int step = 0; step < 2 ; step++)
	{
		if (step == 0)
		{
			x = x1;
			y = y1;
		}
		else
		{
			x = x2;
			y = y2;
		}

		// If out of grid, nothing to test
		if (x >= 0 && x < mSx && y >= 0 && y < mSy)
		{
			// If black box already in place, nothing to test
			pCase = this->pGrid->operator() (x, y);
			if (pCase->IsBloc () == true) continue;

			// Get symmetrical boxes
			numCases = GetSymmetricCases (x, y, symmetry, tabX, tabY);

			// Test every boxes
			for (int i = 0; i < numCases; i ++)
			{
				// Box cannot be in between x1,y1 and x2,y2
				if (tabY [i] == y1 && tabY[i] == y2 && tabX[i] > x1 && tabX[i] < x2) return false;
				if (tabX [i] == x1 && tabX[i] == x2 && tabY[i] > y1 && tabY[i] < y2) return false;

				// Check there is no letter already in place
				pCase = this->pGrid->operator() (tabX [i], tabY [i]);
				if (pCase->IsLetter () == true && pCase->getLetter () != 0) return false;
			}
		}
	}

	return true;
}


// ===========================================================================
/// \brief		Returns all the symmetrical boxes of a given location
///	
/// \param		x			Target horizontal location
/// \param		y			Target vertical location
/// \param		symmetry	Symmetry to consider
/// \param[out]	tabX		X coordinates of all the symmetrical boxes
/// \param[out]	tabY		Y coordinates of all the symmetrical boxes
///
/// \return		Number of symmetrical boxes, original location included {1,2,4}
// ===========================================================================
int Helper::GetSymmetricCases (int x, int y, E_Symmetry symmetry, int tabX [4], int tabY [4])
{
	// Original location
	tabX [0] = x;
	tabY [0] = y;
	if (symmetry == NONE) return 1;

	if (symmetry == HORIZONTAL)
	{
		tabX [1] = mSx -1 -x;
		tabY [1] = y;
		return 2;
	}

	if (symmetry == VERTICAL)
	{
		tabX [1] = x;
		tabY [1] = mSy -1 -y;
		return 2;
	}

	if (symmetry == ORTHOGONAL)
	{
		tabX [1] = mSx -1 -x;
		tabY [1] = y;
		tabX [2] = x;
		tabY [2] = mSy -1 -y;
		tabX [3] = mSx -1 -x;
		tabY [3] = mSy -1 -y;
		return 4;
	}

	if (symmetry == DIA_NO)
	{
		tabX [1] = mSx -1 -y;
		tabY [1] = mSy -1 -x;
		return 2;
	}

	if (symmetry == DIA_NE)
	{
		tabX [1] = y;
		tabY [1] = x;
		return 2;
	}

	if (symmetry == DIAGONAL)
	{
		tabX [1] = mSx -1 -y;
		tabY [1] = mSy -1 -x;
		tabX [2] = y;
		tabY [2] = x;
		tabX [3] = mSy -1 -x;
		tabY [3] = mSx -1 -y;
		return 4;
	}

	if (symmetry == ROT_90)
	{
		tabX [1] = mSy -1 -y;
		tabY [1] = x;
		tabX [2] = mSy -1 -x;
		tabY [2] = mSx -1 -y;
		tabX [3] = y;
		tabY [3] = mSy -1 -x;
		return 4;
	}

	if (symmetry == ROT_180)
	{
		tabX [1] = mSx -1 -x;
		tabY [1] = mSy -1 -y;
		return 2;
	}

	return 0;
}


// ===========================================================================
/// \brief		Find words matching a given mask, with options for cross matching
///	
/// \param		mask		Mask to comply with
/// \param		x			Location of first letter of the mask
/// \param		y			Location of first letter of the mask
/// \param		dir			'V' or 'H' for placement direction (horizontal, vertical)
/// \param[out]	tabStr		Output array to store results
/// \param		maxWords	Maximum number of words we can return
/// \param		findMode	0: Words must only match the mask
///							1: In addition, crosswords must match with the mask
///							2: In addition, crosswords can be smaller than their slot
///
/// \return		Number of word written in 'tabStr'
// ===========================================================================
int Helper::FindWords (char* mask, unsigned char x, unsigned char y, char dir, 
					char tabStr [][MAX_WORD_LENGTH], int maxWords, int findMode)
{
	int i, back;
	bool r;
	bool matchLength;
	bool lookInMain = true;

	char word [MAX_WORD_LENGTH+1];
	char crossMask [MAX_GRID_SIZE+1];
	int found = 0;

	int length;

	LetterCandidates possibleLetters [MAX_GRID_SIZE];

	length = strlen (mask);

	// Get letter candidates a the word location
	GetPossibleLetters (x, y, dir, possibleLetters);
	
	// Cross words must match their slot in mode 1
	matchLength = (findMode == 1);

	// No start word at the beginning
	word [0] = 0;

	// Find loop
	do
	{
		// Look for a word matching the mask
		if (lookInMain)
			pDict->FindEntry (word, mask, word, possibleLetters);
		else
			pThemaDict->FindEntry (word, mask, word, possibleLetters);

		// Not found ?
		if (word [0] == 0) 
		{
			// Look in theme dictionary
			if (lookInMain) 
			{
				lookInMain = false;
				continue;
			}

			// Or stop if we did it already
			else break;
		}


		// If we only check the mask, it's done
		if (findMode == 0)
		{
			r = true;
		}

		// Or we also check for cross matching
		else
		{
			r = true;

			// Go through the word letters
			for (i = 0; i < length; i ++)
			{
				// We don't check letters already on the grid
				if (mask [i] != '*') continue;

				// Build cross word mask
				if (dir == 'H')
					back = pGrid->BuildMask (crossMask, x+i, y, 'V', true);
				else
					back = pGrid->BuildMask (crossMask, x, y+i, 'H', true);

				// Update mask with the letter at the intersection (because it is not on the grid)
				crossMask [back] = word [i];

				// Make the test for the cross word at this location
				r = TestMask (crossMask, back, matchLength);

				// If we detect a problem, we invalidate the letter at this location and stop the word verification
				if (r == false) 
				{
					Box *pCase;
					if (dir == 'H')	pCase = this->pGrid->operator() (x+i, y);
					else pCase = this->pGrid->operator() (x, y+i);
					
					// Invalidate the letter, both at the grid level and at our level
					pCase->SetLetterCandidate (word [i], false);
					possibleLetters [i].SetLetterCandidate (word [i], false);
					break;
				}
			}
		}

		// Keep the word if every test is OK
		if (r == true)
		{
			// Copy result
			strcpy_s (tabStr[found], MAX_WORD_LENGTH, word);

			found ++;
			if (found >= maxWords)  break;
		}

		// Asynchronous stop condition ?
		if (this->enableActivity == false) break;
	}
	while (true);
	
	return found;
}


// ===========================================================================
/// \brief		Check if we can find at least one word for a cross matching test
///	
/// \param		mask			Mask to comply with
/// \param		idxIntersection	Index of the letter in the mask we must absolutely cover
/// \param		matchLength		True if the cross word must exactly match its slot length
///
/// \return		True if we could find a match for the given mask.
// ===========================================================================
bool Helper::TestMask (char mask [], int idxIntersection, bool matchLength)
{
	char word [MAX_WORD_LENGTH+1];

	int i, j;
	int length = strlen (mask);

	// Single check in case of exact length
	if (matchLength == true)
	{
		pDict->FindEntry (word, mask, NULL, NULL);
		if (word [0] != 0) return true;

		pThemaDict->FindEntry (word, mask, NULL, NULL);
		if (word [0] != 0) return true;
		else return false;
	}

	// In the other case, we must try every length possibility covering 'idxIntersec'
	else
	{
		// Test every black box placement from the end of the mask, down to 'idxIntersection'
		for (j = length; j > idxIntersection; j --)
		{
			// Place black box in 'j' if possible
			if (j < length && mask [j] != '*') continue;
			mask [j] = 0;

			// Test every black box placement from the beginning of the mask, up to 'idxIntersection'
			for (i = -1; i < idxIntersection; i ++)
			{
				// Place black box in 'i' if possible
				if (i >= 0 && mask [i] != '*') continue;
				
				// If we find one entry it's OK
				pDict->FindEntry (word, mask+i+1, NULL, NULL);
				if (word [0] != 0) return true;

				pThemaDict->FindEntry (word, mask+i+1, NULL, NULL);
				if (word [0] != 0) return true;
			}
		}
	}

	return false;
}


// ===========================================================================
/// \brief		Extract letters candidates from the grid at a given location
///	
/// \param		x					Location of first letter candidates to extract
/// \param		y					Location of first letter candidates to extract
/// \param		dir					'V' or 'H' for extraction direction (horizontal, vertical)
/// \param[out]	pPossibleLetters	Letter candidates output vector
// ===========================================================================
void Helper::GetPossibleLetters (unsigned char x, unsigned char y, char dir, LetterCandidates* pPossibleLetters)
{
	Box *pCase;
	int i = 0;
	
	do
	{
		// Get target box
		pCase = this->pGrid->operator() (x, y);

		// Copy letter candidates from the box
		pPossibleLetters [i] = pCase->possibleLetters;

		// Move on next box
		if (dir == 'H') 
		{
			x ++;
			if (x >= mSx) break;
		}
		else
		{
			y ++;
			if (y >= mSy) break;
		}
	}
	while (true);	
}

// End
