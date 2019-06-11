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
/// \file libWizium.h
///
/// \brief	libWizium API definition.
///
// ###########################################################################

#ifndef LIBWIZIUM_H
#define LIBWIZIUM_H


constexpr auto VER_MAJOR = 1;
constexpr auto VER_MINOR = 0;
constexpr auto VER_RELEASE = 1;

// ###########################################################################
//
// C O N T E X T
//
// ###########################################################################

#if defined(WIN32)
#define PL_WIN32		///< Windows
#elif defined(__linux__)
#define PL_LINUX		///< Linux
#endif


#ifdef __cplusplus
#   define EXTERN_C    extern "C"
#else
#   define EXTERN_C    extern
#endif

#ifdef __linux__
#define LIB_API
#else
#ifdef LIBWIZIUM_EXPORTS
#define LIB_API __declspec(dllexport)
#else
#define LIB_API __declspec(dllimport)
#endif
#endif

#define API	EXTERN_C LIB_API

#include <stdint.h>

// ###########################################################################
//
// T Y P E S
//
// ###########################################################################

/// LibHandle on objects
typedef uint64_t LibHandle;

/// Version of this library
typedef struct
{
	int32_t major;				///< New feature breaking compatibilty
	int32_t minor;				///< New feature not breaking compatibility
	int32_t release;			///< Bux fix or enhancement
}
Version;

/// Instance configuration
typedef struct
{
	int32_t alphabetSize;		///< 0: use the 26 standard ASCII letters. It also means that any text entry is in ASCII
								///< Otherwise: fix the number of letters in the alphabet. Any text entry must contain
								///< sequence of letters that are in the range [1..alphabetSize].
	int32_t maxWordLength;		///< Max word length of any word in a grid. 
								///< This number fixes the size of any pointer to word entries
}
Config;

/// The different grid box states
typedef enum
{
	LETTER = 0,					///< The box holds a letter
	VOID = 1,					///< The box is not used (for non squared grids)
	BLACK = 2					///< The box is a black blox
}
BoxType;

/// Rule for generation of black boxes
typedef enum
{
	ANY = 0,				///< No rules to place black boxes
	SINGLE = 1,				///< Black boxes cannot touch together
	TWO = 2,				///< Two black boxes can touch, no more
	DIAGONAL = 3,			///< Black boxes can touch in diagonal only
}
BlackMode;

/// Solver configuration
typedef struct
{
	uint32_t seed;				///< RNG seed
	int32_t maxBlackBoxes;		///< Max number of black cases that can be added to the grid
	int32_t heuristicLevel;		///< Heurisitic level (see doc)
	BlackMode blackMode;		///< Rule for the generation of black boxes
}
SolverConfig;

/// Status of the grid generation process
typedef struct 
{
	uint64_t counter;		///< Total number of words tried
	int32_t fillRate;		///< Current fill rate [%]. 0: generation failed. 100: generation successful
}
Status;


// ###########################################################################
//
// P R O T O T Y P E S
//
// ###########################################################################

API void WIZ_Init (Version &version);

API LibHandle WIZ_CreateInstance (const Config& config);
API void WIZ_DestroyInstance (LibHandle instance);

API void DIC_Clear (LibHandle instance);
API uint32_t DIC_GetNumWords (LibHandle instance);
API int32_t DIC_AddEntries (LibHandle instance, const uint8_t entries [], int32_t numEntries);
API bool DIC_FindEntry (LibHandle instance, uint8_t result [], const uint8_t mask [], const uint8_t startWord []);
API bool DIC_FindRandomEntry (LibHandle instance, uint8_t result [], const uint8_t mask []);

API void GRID_SetSize (LibHandle instance, uint8_t width, uint8_t height);
API void GRID_SetBox (LibHandle instance, uint8_t x, uint8_t y, BoxType type);
API void GRID_Write (LibHandle instance, uint8_t x, uint8_t y, const uint8_t entry [], char dir, bool terminator);
API void GRID_Read (LibHandle instance, uint8_t grid []);
API void GRID_Erase (LibHandle instance);

API void SOLVER_Start (LibHandle instance, const SolverConfig& solver);
API void SOLVER_Step (LibHandle instance, int32_t maxTimeMs, int32_t maxSteps, Status& status);
API void SOLVER_Stop (LibHandle instance);

#endif

