
#include <windows.h>
#include <fstream>

#include "Dictionary/Dictionary.h"
#include "Grid/Grid.h"
//#include "C_SolverDynamic.h"
#include "Solvers/SolverStatic.h"
#include "Helper.h"



// ================================================================================
// Constructiuon du dictionnaire su base d'une liste de mots contenus dans un 
// fichier text
// ================================================================================
bool BuildFromWordList (const char* path, Dictionary *pdict)
{
	FILE* file;
	
	int j = 0;
	int idx = 0;
	
	int t2, t1;

	char c;
	char words [20*100+1];
	int count = 0;

	fopen_s (&file, path, "rb");
	if (file == NULL) return false;



	while (!feof(file))
	{
		do
		{
			fread (&c, 1, 1, file);
			
			if (c == 13 || c == 10) 
			{
				if (idx % 20 != 0) 
				{
					words [idx] = 0;
					idx += 20 - (idx%20);
				}
				continue;
			}

			if (c == ' ' || c == '-' || c == '\'') continue;
			if (c == 'ç') c = 'c';
			if (c == 'à') c = 'a';
			if (c == 'ä') c = 'a';
			if (c == 'â') c = 'a';
			if (c == 'é') c = 'e';
			if (c == 'è') c = 'e';
			if (c == 'ê') c = 'e';
			if (c == 'ë') c = 'e';
			if (c == 'ï') c = 'i';
			if (c == 'î') c = 'i';
			if (c == 'ì') c = 'i';
			if (c == 'ô') c = 'o';
			if (c == 'ö') c = 'o';
			if (c == 'û') c = 'u';
			if (c == 'ü') c = 'u';
			if (c == 'ù') c = 'u';
			if (c == 'ÿ') c = 'y';
			if (c == 'ñ') c = 'n';
			words [idx] = toupper (c);
			if (words [idx] < 'A' || words [idx] > 'Z') printf ("\nCaractere inconnu : %c", c);
			
			idx ++;
			if ((idx % 20) == 19)
			{
				idx --;
			}
		}
		while (!feof(file) && idx < 2000);

		if (idx % 20 > 0) idx += 20 - (idx%20);
		if (idx <= 20*100) words [idx] = 0;

		t1 = GetTickCount ();
		pdict->AddEntries (words, 20, 100);
		t2 = GetTickCount ();

		count += 100;
		idx = 0;

		//		j ++;
//		if (j > 100) break;
	}

	fclose (file);

	printf ("\n%d words loaded", count);
	return true;
}

Grid grid;
//SolverDynamic solver;
SolverStatic solverStatic;
Dictionary dict;
Dictionary dictThema;
Helper helper;

//extern unsigned long numCallFindEntry;
//extern LONGLONG spentFindEntry;
//extern LARGE_INTEGER freq2;


void MainMenu ()
{
	char c;

	do
	{
		for (int i = 0; i < 50; i ++) {
			printf ("\n");
		}

		printf  ("\n");
		printf ("\n\n            1) Load dictionary");
		printf ("\n\n            2) Set Grid Size");
		printf ("\n\n            3) Erase Grid");
		printf ("\n\n            4) Define one case");
		printf ("\n\n            5) Draw grid");
		printf ("\n\n            6) Create crossword");
		printf ("\n\n");
		printf ("\n\n            7) Exit");
		printf ("\n\n\n\n\n");
		printf ("\n            > ");

		fflush (stdin);
		c = getchar ();
		if( c < '1' || c > '6') continue;

		//
		if (c == '1') {
			
			printf ("\n\nPlease wait ...");

			BuildFromWordList ("J:\\Mes Projets\\QWord\\Dictionaries\\Languages\\French_Void.txt", &dict);

			printf ("Done !");
			printf ("\n\nPress enter to come back in the menu");
	
			fflush (stdin);
			getchar ();
		}
		
		//
		else if (c == '2') {

			int w, h;

			printf ("\n\nEnter grid width [1-50]: ");
			scanf_s ("%d", &w);

			if (w < 1 || w > 50) printf ("\nWrong size");
			else
			{
				printf ("\nEnter grid height [1-50] : ");
				scanf_s ("%d", &h);
				if (w < 1 || w > 50) printf ("\nWrong size");
				else
				{
					grid.Grow (w, h);
					printf ("\nDone !");
				}
			}
			printf ("\n\nPress enter to come back in the menu");
	
			fflush (stdin);
			getchar ();
		}

		//
		else if (c == '3') {
			
			grid.Erase ();
			printf ("\nDone");
			printf ("\n\nPress enter to come back in the menu");
	
			fflush (stdin);
			getchar ();
		}


		//
		else if (c == '4') {

			int w, h;
			char c;

			printf ("\n\nEnter case position [x,y] : ");
			scanf_s ("%d,%d", &w, &h);

			Box* pCase = grid.operator () (w,h);
			if (pCase == NULL)
			{
				printf ("\nBad position !!");
			}
			else
			{
				printf ("\nThis case is currently ");
				if (pCase->IsBloc ())
					printf ("a BLOC");
				else if (pCase->IsVoid ())
					printf ("VOID");
				else if (pCase->IsLetter ()) {
					printf ("a LETTER, value = ");

					if (pCase->getLetter () != 0)
						printf ("%c", pCase->getLetter ());
					else
						printf ("Undefined");
				}

				printf ("\nNew state ? (B) for BLOC, (L) for LETTER, (V) for VOID : ");
	
				fflush (stdin);
				c = getchar ();

				if (c == 'B' || c == 'b')
					pCase->MakeBloc ();
				else if (c == 'V' || c == 'v')
					pCase->MakeVoid ();
				else if (c == 'L' || c == 'l') {

					printf ("\n\nWhat letter for this case (0=undefined) : ");
					fflush (stdin);
					c = getchar ();

					if( (c < 'a' || c > 'z') && (c < 'A' || c > 'Z') && c != '0')
						printf ("\nIncorrect choice !!");
					else
					{
						pCase->MakeLetter ();
						pCase->setLetter (c);
					}

				}
				else printf ("\nIncorrect choice !!");

				if (pCase->IsLetter () == false || pCase->getLetter () != 0)
					pCase->Lock (true);
				else
					pCase->Lock (false);
			}

			printf ("\n\nPress enter to come back in the menu");
	
			fflush (stdin);
			getchar ();
		}

		//
		else if (c == '5')
		{			
			grid.Draw ();
			printf ("\n\nPress enter to come back in the menu");
	
			fflush (stdin);
			getchar ();
		}


		//
		else if (c == '6')
		{/*
			int t2, t1;
			char c;
			
			//srand (1);
			printf ("\n\nBloc density (D=Diag, S=Single, N=None) ? : ");
			fflush (stdin);
			c = getchar ();

			if (c== 'N' || c == 'n')
			{
				printf ("\nNo Block allowed");
				grid.SetBlocDensity (C_Grid::NONE);
			}
			else if (c== 'S' || c == 's')
			{
				printf ("\nBlocks can't touch each other");
				grid.SetBlocDensity (C_Grid::SINGLE);
			}
			else
			{
				grid.SetBlocDensity (C_Grid::DIAG);
			}


			printf ("\n\nUse heuristic (O/N) ? : ");
			fflush (stdin);
			c = getchar ();

			if (c == 'O' || c == 'o')
			{
				printf ("\n\nStepBack parameter (0-5) ? : ");
				fflush (stdin);
				c = getchar ();

				if (c < '0' || c > '5') c = '0';

				solver.SetHeurestic (true,c-'0');
				printf ("\nHeuristic with stepBack equal to %c", c);
			}
			else {
				solver.SetHeurestic (false, 0);
				printf ("\nNo heuristic");
			}

			solver.SetMaxBlackCases (1000);

			t1 = GetTickCount ();

			solver.Solve_Start (&grid, &dict, &dict);

			int fillrate;
			unsigned int numattempts_High, numattempts_Low;
			solver.Solve_Step (1000000, &fillrate, &numattempts_High, &numattempts_Low);
			
			t2 = GetTickCount ();

			solver.Solve_Stop ();

			grid.Draw ();
			printf ("\n\nTemps : %d ms", (t2-t1));

			C_Case *pcase;
			int bloc = 0;
			int tot = 0;

			for (int x = 0; x < grid.GetWidth (); x ++)
			{
				printf ("\n\n");
				for (int y = 0; y < grid.GetHeight (); y ++)
				{
					pcase = grid.operator ()(x,y);
					if (pcase->IsBloc ()) bloc ++;

					tot ++;
				}
			}

			printf ("\nNb Cases noires : %d (%.2f%%)", bloc, ((float) 100*bloc/tot));
			printf ("\n\nPress enter to come back in the menu");
			
		//	double time = ((double) spentFindEntry / (double) freq2.LowPart);
		//	printf ("NumCall : %d, Time : %f", numCallFindEntry, time);
	
			fflush (stdin);
			getchar ();*/
		}

	}
	while (c != '7');
}

void Write (int x, int y, char* word)
{
	int i = 0;
	Box* pCase;
	
	while (word [i] != 0)
	{
		pCase = grid.operator () (x+i,y);
		pCase->MakeLetter ();
		pCase->setLetter (word [i]);

		i ++;
	}

	pCase = grid.operator () (x+i,y);
	if (pCase != NULL) pCase->MakeBloc ();
}

int main ()
{

unsigned char codeData1 [] = {
0x81, 0xec, 0x10, 0x01, 0x00, 0x00, /*0xa1, 0x00, 0x70, 0x40, 0x00,*/ 0x33, 0xc4, 0x89, 0x84, 0x24,
0x0c, 0x01, 0x00, 0x00, 0x8b, 0x84, 0x24, 0x28, 0x01, 0x00, 0x00, 0x8b, 0x94, 0x24, 0x2c, 0x01,
0x00, 0x00, 0x8b, 0x8c, 0x24, 0x20, 0x01, 0x00, 0x00, 0x53, 0x8b, 0x9c, 0x24, 0x20, 0x01, 0x00,
0x00, 0x55, 0x8b, 0xac, 0x24, 0x1c, 0x01, 0x00, 0x00, 0x56, 0x8b, 0xb4, 0x24, 0x24, 0x01, 0x00,
0x00, 0x57, 0x8b, 0xfb, 0x89, 0x44, 0x24, 0x10, 0x89, 0x54, 0x24, 0x14, 0x8b, 0xc6, 0x2b, 0xfe,
0x8a, 0x10, 0x88, 0x14, 0x07, 0x83, 0xc0, 0x01, 0x84, 0xd2, 0x75, 0xf4, 0x0f, 0xb6, 0x46, 0x2c,
0x0f, 0xb6, 0x56, 0x2a, 0x6a, 0x00, 0x50, 0x0f, 0xb6, 0x46, 0x29, 0x52, 0x50, 0x8d, 0x54, 0x24,
0x28, 0x52, 0xff, 0x54, 0x24, 0x28, 0x8b, 0x8c, 0x24, 0x34, 0x01, 0x00, 0x00, 0x0f, 0xb6, 0x46,
0x2b, 0x51, 0x53, 0x8d, 0x54, 0x24, 0x20, 0x52, 0x56, 0x8b, 0xcd, 0xc6, 0x44, 0x04, 0x28, 0x00,
0xff, 0x54, 0x24, 0x20, 0x8b, 0x8c, 0x24, 0x1c, 0x01, 0x00, 0x00, 0x5f, 0x5e, 0x5d, 0x5b, 0x33,
0xcc, /*0xe8, 0x6b, 0x1a, 0x00, 0x00,*/ 0x81, 0xc4, 0x10, 0x01, 0x00, 0x00, 0xc3, 0xcc, 0xcc, 0xcc
		};


unsigned char codeData2 [] = {
0x81, 0xec, 0x10, 0x01, 0x00, 0x00, /*0xa1, 0x00, 0x80, 0x40, 0x00,*/ 0x33, 0xc4, 0x89, 0x84, 0x24, 
0x0c, 0x01, 0x00, 0x00, 0x8b, 0x94, 0x24, 0x20, 0x01, 0x00, 0x00, 0x8b, 0x8c, 0x24, 0x18, 0x01,
0x00, 0x00, 0x8b, 0x84, 0x24, 0x38, 0x01, 0x00, 0x00, 0x53, 0x8b, 0x9c, 0x24, 0x28, 0x01, 0x00,
0x00, 0x55, 0x8b, 0xac, 0x24, 0x3c, 0x01, 0x00, 0x00, 0x56, 0x8b, 0xb4, 0x24, 0x20, 0x01, 0x00,
0x00, 0x57, 0x8b, 0xbc, 0x24, 0x38, 0x01, 0x00, 0x00, 0x6a, 0x01, 0x53, 0x52, 0x8b, 0x94, 0x24,
0x38, 0x01, 0x00, 0x00, 0x52, 0x8d, 0x54, 0x24, 0x28, 0x52, 0xff, 0xd0, 0x80, 0xfb, 0x48, 0x8a,
0x94, 0x24, 0x2c, 0x01, 0x00, 0x00, 0x8a, 0x8c, 0x24, 0x30, 0x01, 0x00, 0x00, 0x88, 0x54, 0x24,
0x10, 0x88, 0x4c, 0x24, 0x14, 0x75, 0x08, 0x2a, 0xd0, 0x88, 0x54, 0x24, 0x10, 0xeb, 0x06, 0x2a,
0xc8, 0x88, 0x4c, 0x24, 0x14, 0x80, 0xbc, 0x24, 0x40, 0x01, 0x00, 0x00, 0x01, 0x75, 0x1d, 0x8b,
0x84, 0x24, 0x3c, 0x01, 0x00, 0x00, 0x8b, 0x4c, 0x24, 0x14, 0x8b, 0x54, 0x24, 0x10, 0x6a, 0x01,
0x50, 0x57, 0x53, 0x51, 0x52, 0x8d, 0x44, 0x24, 0x30, 0x50, 0xeb, 0x1b, 0x8b, 0x8c, 0x24, 0x3c,
0x01, 0x00, 0x00, 0x8b, 0x54, 0x24, 0x14, 0x8b, 0x44, 0x24, 0x10, 0x6a, 0x00, 0x51, 0x57, 0x53,
0x52, 0x50, 0x8d, 0x4c, 0x24, 0x30, 0x51, 0x8b, 0xce, 0xff, 0xd5, 0x8b, 0x8c, 0x24, 0x1c, 0x01,
0x00, 0x00, 0x5f, 0x5e, 0x5d, 0x5b, 0x33, 0xcc, /*0xe8, 0xd4, 0x2b, 0x00, 0x00,*/ 0x81, 0xc4, 0x10,
0x01, 0x00, 0x00, 0xc3, 0xcc, 0xcc, 0xcc, 0xcc};


// ---------------------------------------------------------------------------------------


/*
	grid.Grow (7, 7);
	
	grid (3,1)->MakeBloc ();
	grid (2,2)->MakeBloc ();
	grid (2,3)->MakeBloc ();
	grid (2,4)->MakeBloc ();
	grid (3,5)->MakeBloc ();

	grid (0,3)->MakeBloc ();
	grid (6,3)->MakeBloc ();

*/
// ---------------------------------------------------------------------------------------


/*
	grid.Grow (11, 11);

	grid (9,0)->MakeBloc ();
	grid (0,1)->MakeBloc ();
	grid (6,1)->MakeBloc ();
	grid (3,2)->MakeBloc ();
	grid (8,3)->MakeBloc ();
	grid (1,4)->MakeBloc ();
	grid (5,5)->MakeBloc ();
	grid (9,6)->MakeBloc ();
	grid (2,7)->MakeBloc ();
	grid (7,8)->MakeBloc ();
	grid (4,9)->MakeBloc ();
	grid (10,9)->MakeBloc ();
	grid (1,10)->MakeBloc ();
*/
// ---------------------------------------------------------------------------------------

/*
	grid.Grow (11, 11);

	grid (4,0)->MakeBloc ();
	grid (4,1)->MakeBloc ();
	grid (4,2)->MakeBloc ();
	grid (6,0)->MakeBloc ();
	grid (6,1)->MakeBloc ();
	grid (6,2)->MakeBloc ();
	grid (4,8)->MakeBloc ();
	grid (4,9)->MakeBloc ();
	grid (4,10)->MakeBloc ();
	grid (6,8)->MakeBloc ();
	grid (6,9)->MakeBloc ();
	grid (6,10)->MakeBloc ();

	grid (0,4)->MakeBloc ();
	grid (1,4)->MakeBloc ();
	grid (2,4)->MakeBloc ();
	grid (3,4)->MakeBloc ();
	grid (7,4)->MakeBloc ();
	grid (8,4)->MakeBloc ();
	grid (9,4)->MakeBloc ();
	grid (10,4)->MakeBloc ();
	grid (0,6)->MakeBloc ();
	grid (1,6)->MakeBloc ();
	grid (2,6)->MakeBloc ();
	grid (3,6)->MakeBloc ();
	grid (7,6)->MakeBloc ();
	grid (8,6)->MakeBloc ();
	grid (9,6)->MakeBloc ();
	grid (10,6)->MakeBloc ();

	grid (5,5)->MakeBloc ();
*/
// ---------------------------------------------------------------------------------------


/*
	grid.Grow (11, 11);
	
	grid (5,0)->MakeBloc ();
	grid (3,2)->MakeBloc ();
	grid (7,2)->MakeBloc ();
	grid (2,3)->MakeBloc ();
	grid (8,3)->MakeBloc ();

	grid (0,5)->MakeBloc ();
	grid (5,5)->MakeBloc ();
	grid (10,5)->MakeBloc ();

	grid (2,7)->MakeBloc ();
	grid (8,7)->MakeBloc ();
	grid (3,8)->MakeBloc ();
	grid (7,8)->MakeBloc ();
	grid (5,10)->MakeBloc ();
*/
//	Write (0,0, "FIAIS");
//	Write (6,0, "FALLU");
//	Write (0,1, "UTILISASSES");
//	Write (0,2, "MER");
//	Write (4,2, "GIN");
//	Write (8,2, "DUE");
//	Write (0,3, "ER");
//	Write (3,3, "RIMAS");
//	Write (9,3, "RE");
//	Write (0,4, "REVELATEURS");
//	Write (1,5, "RAIL");
//	Write (6,5, "IVRE");
//	Write (0,6, "CARNASSIERE");
//	Write (0,7, "AI");
//	Write (3,7, "SITAR");
//	Write (9,7, "IL");
//	Write (0,8, "LES");
//	Write (4,8, "RAT");
//	Write (8,8, "FOI");
//	Write (0,9, "ENUMERERONS");


// ===========================================================================
// 15x15 38b 78w

	grid.Grow (15, 15);

	grid (5, 0)->MakeBloc ();
	grid (5, 1)->MakeBloc ();
	grid (10, 0)->MakeBloc ();
	grid (10, 1)->MakeBloc ();
	grid (10, 2)->MakeBloc ();
	grid (6, 3)->MakeBloc ();
	grid (0, 4)->MakeBloc ();
	grid (1, 4)->MakeBloc ();
	grid (2, 4)->MakeBloc ();
	grid (3, 4)->MakeBloc ();
	grid (7, 4)->MakeBloc ();
	grid (8, 4)->MakeBloc ();
	grid (12, 4)->MakeBloc ();
	grid (13, 4)->MakeBloc ();
	grid (14, 4)->MakeBloc ();
	grid (4, 6)->MakeBloc ();
	grid (5, 7)->MakeBloc ();
	grid (5, 8)->MakeBloc ();
	grid (3, 9)->MakeBloc ();
	grid (11, 5)->MakeBloc ();
	grid (9, 6)->MakeBloc ();
	grid (9, 7)->MakeBloc ();
	grid (10, 8)->MakeBloc ();
	grid (0, 10)->MakeBloc ();
	grid (1, 10)->MakeBloc ();
	grid (2, 10)->MakeBloc ();
	grid (6, 10)->MakeBloc ();
	grid (7, 10)->MakeBloc ();
	grid (11, 10)->MakeBloc ();
	grid (12, 10)->MakeBloc ();
	grid (13, 10)->MakeBloc ();
	grid (14, 10)->MakeBloc ();
	grid (8, 11)->MakeBloc ();
	grid (4, 12)->MakeBloc ();
	grid (4, 13)->MakeBloc ();
	grid (4, 14)->MakeBloc ();
	grid (9, 13)->MakeBloc ();
	grid (9, 14)->MakeBloc ();


// ===========================================================================
// 15x15 34b 76w

	grid.Grow (15, 15);

	grid (5, 0)->MakeBloc ();
	grid (11, 0)->MakeBloc ();
	grid (5, 1)->MakeBloc ();
	grid (11, 1)->MakeBloc ();
	grid (11, 2)->MakeBloc ();
	grid (3, 3)->MakeBloc ();
	grid (8, 3)->MakeBloc ();
	grid (0, 4)->MakeBloc ();
	grid (1, 4)->MakeBloc ();
	grid (2, 4)->MakeBloc ();
	grid (7, 4)->MakeBloc ();
	grid (13, 4)->MakeBloc ();
	grid (14, 4)->MakeBloc ();
	grid (6, 5)->MakeBloc ();
	grid (4, 6)->MakeBloc ();
	grid (11, 6)->MakeBloc ();
	grid (5, 7)->MakeBloc ();
	grid (9, 7)->MakeBloc ();
	grid (3, 8)->MakeBloc ();
	grid (10, 8)->MakeBloc ();
	grid (8, 9)->MakeBloc ();
	grid (0, 10)->MakeBloc ();
	grid (1, 10)->MakeBloc ();
	grid (7, 10)->MakeBloc ();
	grid (12, 10)->MakeBloc ();
	grid (13, 10)->MakeBloc ();
	grid (14, 10)->MakeBloc ();
	grid (6, 11)->MakeBloc ();
	grid (11, 11)->MakeBloc ();
	grid (3, 12)->MakeBloc ();
	grid (3, 13)->MakeBloc ();
	grid (9, 13)->MakeBloc ();
	grid (3, 14)->MakeBloc ();
	grid (9, 14)->MakeBloc ();



	/*
	grid.Grow (15, 15);

	grid (7, 0)->MakeBloc ();
	grid (7, 1)->MakeBloc ();
	grid (7, 2)->MakeBloc ();
	grid (4, 3)->MakeBloc ();
	grid (9, 3)->MakeBloc ();
	grid (10, 3)->MakeBloc ();

	grid (3, 4)->MakeBloc ();
	grid (5, 5)->MakeBloc ();
	grid (12, 5)->MakeBloc ();
	grid (13, 5)->MakeBloc ();
	grid (14, 5)->MakeBloc ();
	grid (8, 6)->MakeBloc ();
	grid (7, 7)->MakeBloc ();
	grid (6, 8)->MakeBloc ();
	grid (0, 9)->MakeBloc ();
	grid (1, 9)->MakeBloc ();
	grid (2, 9)->MakeBloc ();
	grid (9, 9)->MakeBloc ();

	grid (11, 10)->MakeBloc ();
	grid (10, 11)->MakeBloc ();
	grid (4, 11)->MakeBloc ();
	grid (5, 11)->MakeBloc ();

	grid (7, 12)->MakeBloc ();
	grid (7, 13)->MakeBloc ();
	grid (7, 14)->MakeBloc ();
	*/
// ---------------------------------------------------------------------------------------
/*

	grid.Grow (17, 17);

	grid (7, 0)->MakeBloc ();
	grid (7, 1)->MakeBloc ();
	grid (13, 1)->MakeBloc ();
	grid (13, 2)->MakeBloc ();
	grid (5, 2)->MakeBloc ();
	grid (2, 3)->MakeBloc ();
	grid (3, 3)->MakeBloc ();
	grid (9, 3)->MakeBloc ();
	grid (10, 3)->MakeBloc ();
	grid (1, 4)->MakeBloc ();
	grid (5, 5)->MakeBloc ();
	grid (6, 5)->MakeBloc ();
	grid (7, 5)->MakeBloc ();
	grid (8, 5)->MakeBloc ();
	grid (12, 5)->MakeBloc ();
	grid (13, 6)->MakeBloc ();
	grid (3, 7)->MakeBloc ();
	grid (4, 7)->MakeBloc ();
	grid (10, 7)->MakeBloc ();
	grid (14, 7)->MakeBloc ();
	grid (15, 7)->MakeBloc ();
	grid (16, 7)->MakeBloc ();

	grid (8, 8)->MakeBloc ();

	grid (13, 9)->MakeBloc ();
	grid (12, 9)->MakeBloc ();
	grid (6, 9)->MakeBloc ();
	grid (2, 9)->MakeBloc ();
	grid (1, 9)->MakeBloc ();
	grid (0, 9)->MakeBloc ();

	grid (3, 10)->MakeBloc ();
	
	grid (11, 11)->MakeBloc ();
	grid (10, 11)->MakeBloc ();
	grid (9, 11)->MakeBloc ();
	grid (8, 11)->MakeBloc ();
	grid (4, 11)->MakeBloc ();

	grid (15, 12)->MakeBloc ();

	grid (14, 13)->MakeBloc ();
	grid (13, 13)->MakeBloc ();
	grid (7, 13)->MakeBloc ();
	grid (6, 13)->MakeBloc ();

	grid (3, 14)->MakeBloc ();
	grid (11, 14)->MakeBloc ();

	grid (9, 15)->MakeBloc ();
	grid (3, 15)->MakeBloc ();
	
	grid (9, 16)->MakeBloc ();
*/
//	Write (0,0,"MASURES");
//	Write (8,0,"PASSANTES");
//	Write (0,1,"EVASERA");
//	Write (8,1,"INOUI");
//	Write (14,1,"OTA");
//	Write (0,2,"CASES");
//	Write (6,2,"LOGEURS");
//	Write (14,2,"REG");

//	Write (0,3,"AL");
//	Write (4,3,"USINE");
//	Write (11,3,"VEGETA");

//	Write (0,4,"N");
//	Write (2,4,"DECENTRALISERAI");

//	Write (0,5,"IMITE");
//	Write (9,5,"VIE");
//	Write (13,5,"LOGE");

//	Write (0,6,"SUCCEDERAIENT");
//	Write (14,6,"SES");

//	Write (0,7,"EST");
//	Write (5,7,"OTONS");
//	Write (11,7,"DUR");

//	Write (0,8,"SCEPTRES");
//	Write (9,8,"ENRAIERA");

//	Write (3,9,"PAL");
//	Write (7,9,"ECRIA");
//	Write (14,9,"MOU");

//	Write (0,10,"FAT");
//	Write (4,10,"CONTRADICTEUR");

//	Write (0,11,"ACRE");
//	Write (5,11,"TUT");
//	Write (12,11,"ROUEE");

//	Write (0,12,"SCANDALEUSEMENT");
//	Write (16,12,"O");

//	Write (0,13,"CONVOI");
//	Write (8,13,"RUGIT");
//	Write (15,13,"ML");

//	Write (0,14,"ILS");
//	Write (4,14,"REOPERA");
//	Write (12,14,"ALLEE");

//	Write (0,15,"NEE");
//	Write (4,15,"ENTRE");
//	Write (10,15,"RACIALE");

	//Write (0,16,"ASSISTEES");
	//Write (10,16,"EVENTES");

	// ---------------------------------------------------------------------------------------



/*
	grid.Grow (17, 17);

	grid (8, 0)->MakeBloc ();
	grid (13, 0)->MakeBloc ();
	grid (8, 1)->MakeBloc ();
	grid (13, 1)->MakeBloc ();
	grid (13, 2)->MakeBloc ();
	grid (13, 3)->MakeBloc ();
	grid (9, 3)->MakeBloc ();
	grid (3, 3)->MakeBloc ();
	grid (4, 3)->MakeBloc ();
	grid (5, 4)->MakeBloc ();
	grid (6, 4)->MakeBloc ();
	grid (7, 4)->MakeBloc ();
	grid (0, 5)->MakeBloc ();
	grid (1, 5)->MakeBloc ();
	grid (2, 5)->MakeBloc ();
	grid (7, 5)->MakeBloc ();
	grid (8, 5)->MakeBloc ();
	grid (12, 5)->MakeBloc ();
	grid (12, 6)->MakeBloc ();
	grid (3, 7)->MakeBloc ();
	grid (9, 7)->MakeBloc ();
	grid (10, 7)->MakeBloc ();
	grid (11, 7)->MakeBloc ();
	grid (15, 7)->MakeBloc ();
	grid (16, 7)->MakeBloc ();

	grid (8, 8)->MakeBloc ();


	grid (0, 9)->MakeBloc ();
	grid (1, 9)->MakeBloc ();
	grid (5, 9)->MakeBloc ();
	grid (6, 9)->MakeBloc ();
	grid (7, 9)->MakeBloc ();
	grid (13, 9)->MakeBloc ();
	grid (4, 10)->MakeBloc ();
	grid (4, 11)->MakeBloc ();
	grid (8, 11)->MakeBloc ();
	grid (9, 11)->MakeBloc ();
	grid (14, 11)->MakeBloc ();
	grid (15, 11)->MakeBloc ();
	grid (16, 11)->MakeBloc ();
	grid (9, 12)->MakeBloc ();
	grid (10, 12)->MakeBloc ();
	grid (11, 12)->MakeBloc ();
	grid (3, 13)->MakeBloc ();
	grid (3, 14)->MakeBloc ();
	grid (3, 15)->MakeBloc ();
	grid (3, 16)->MakeBloc ();
	grid (7, 13)->MakeBloc ();
	grid (12, 13)->MakeBloc ();
	grid (13, 13)->MakeBloc ();
	grid (8, 15)->MakeBloc ();
	grid (8, 16)->MakeBloc ();
*/
  
	// ---------------------------------------------------------------------------------------

/*	dict.AddEntries ("ER", -1, 1);
	dict.AddEntries ("LSD", -1, 1);
	dict.AddEntries ("SUE", -1, 1);
	dict.AddEntries ("MM", -1, 1);
	dict.AddEntries ("AL", -1, 1);
	dict.AddEntries ("ETC", -1, 1);
	dict.AddEntries ("PP", -1, 1);
	dict.AddEntries ("ENV", -1, 1);
	dict.AddEntries ("CR", -1, 1);
	dict.AddEntries ("ML", -1, 1);
	dict.AddEntries ("LAT", -1, 1);
	dict.AddEntries ("AV", -1, 1);
*/

/*
	grid.Grow (15, 15);
	solver.SetSeed (69);
	solver.SetMaxBlackCases (30);
	solver.SetHeurestic (true, 3);
	solver.SetBlackCasesDensity (C_Grid::DIAG);
	solver.SetLicensedData (codeData1);

	solver.Solve_Start (&grid, &dict, &dictThema);
	solver.Solve_Step (1000000, NULL, NULL, NULL);
*/

	BuildFromWordList("X:\\GenId\\4) Products\\Wizium\\Engineering\\Dictionaries\\Raw\\Fr_ODS4.txt", &dict);


	
//	grid.Grow(9,8);
//	Write (0, 0, "STOPPERA");

//	Write(0, 0, "DECROCHES");
//	Write(0, 1, "ECOEURANT");
//	Write(0, 2, "RONFLANTE");
//	Write(0, 3, "ATTRISTER");
//	Write(0, 4, "PARAPHERA");
//	Write(0, 5, "EMACIERAS");

	solverStatic.SetSeed (0);
	solverStatic.SetHeurestic (false, 69);
	solverStatic.Solve_Start (&grid, &dict, &dictThema);
	solverStatic.Solve_Step (100000000, NULL, NULL, NULL);

	MainMenu ();

	return 0;
}