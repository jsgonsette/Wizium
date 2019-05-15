// ###########################################################################
//
// Copyright (C) 2015 GenId SPRL
//
///
/// \file 	Helper.h
/// \author	Jean-Sebastien Gonsette
/// \date	09/04/2015
///
// ###########################################################################

#ifndef __CHELPER__H
#define __CHELPER__H

#include "Grid/Grid.h"
#include "Dictionary/Dictionary.h"


// ###########################################################################
//
// T Y P E S
//
// ###########################################################################
class Helper;


// Typedef de la fonction FindWords
typedef	int (Helper::* pfFindWords) (char* mask, unsigned char x, unsigned char y, char dir, char tabStr [][MAX_WORD_LENGTH], int maxWords, int findMode);

// Typedef de la fonction pour la license
typedef int (*tpfLicensedFunction2) (Helper *pHelper, Grid* pGrid, unsigned char col, unsigned char row, char dir, char tabStr [][MAX_WORD_LENGTH], int maxStr, bool crossCheck, pfFindWords funcFW, pfBuildMask funcBM);

// Prototype de la fonction à utiliser selon la license
int LicensedFunction2 (Helper *pHelper, Grid* pGrid, unsigned char col, unsigned char row, char dir, char tabStr [][MAX_WORD_LENGTH], int maxStr, bool crossCheck, pfFindWords funcFW, pfBuildMask funcBM);



// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

/// Helper to make propositions to fill in the grid
class Helper
{
public:

	Helper ();
	~Helper ();


	enum E_Symmetry {NONE, HORIZONTAL, VERTICAL, ORTHOGONAL, ROT_180, ROT_90, DIA_NO, DIA_NE, DIAGONAL};

	void SetLicensedData (void* data, int dataSize);
	
	void TakeGridSnapshot (Grid *pGrid);
	int Help (Dictionary *pDict, Dictionary *pThemaDict, unsigned char col, unsigned char row, char dir, char tabStr [][MAX_WORD_LENGTH], int tabOffsets [], int maxStr, bool matchLength, bool crossCheck, E_Symmetry symmetry);

	void EnableActivity (bool state);
	bool IsEnabled () {return enableActivity;}

private:

	bool IsBlocPOssible (int x1, int y1, int x2, int y2, E_Symmetry symmetry);
	int FindWords (char* mask, unsigned char x, unsigned char y, char dir, char tabStr [][MAX_WORD_LENGTH], int maxWords, int findMode);
	bool TestMask (char mask [], int idxIntersection, bool matchLength);
	void GetPossibleLetters(unsigned char x, unsigned char y, char dir, LetterCandidates* pPossibleLetters);
	int GetSymmetricCases (int x, int y, E_Symmetry symmetry, int tabX [4], int tabY [4]);

	// Pointeur de fonction vers une version alternative de 'Help'
	tpfLicensedFunction2 pfLicensedFunction2;

private:

	/// Working grid
	Grid *pGrid;

	/// Dictionaries to use
	Dictionary *pDict;
	Dictionary *pThemaDict;

	/// Grid horizontal size
	unsigned char mSx;

	/// Grid vertical size
	unsigned char mSy;

	/// Activity flag
	bool enableActivity;
};



#endif


