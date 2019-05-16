# ############################################################################	

__license__ = \
	"""This file is part of the Wizium distribution (https://github.com/jsgonsette/Wizium).
	Copyright (c) 2019 Jean-Sebastien Gonsette.

	This program is free software : you can redistribute it and / or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	This program is distributed in the hope that it will be useful, but
	WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
	General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.If not, see <http://www.gnu.org/licenses/>."""

__author__ = "Jean-Sebatien Gonsette"
__email__ = "jeansebastien.gonsette@gmail.com"
__version__ = "1.0"

# ############################################################################	

import ctypes
import platform
if platform.system()=='Linux':
	from ctypes import *
else:
	from ctypes import c_int, WINFUNCTYPE, windll
	from ctypes.wintypes import HWND, LPCSTR, UINT

# ############################################################################	

class Wizium:
	"Wrapper around the libWizium library"

	_init_done = False

	# ============================================================================
	class Version(ctypes.Structure):
		"""Library version"""
	# ============================================================================
		_fields_ = [("major", ctypes.c_int), 
					("minor", ctypes.c_int),
					("release", ctypes.c_int)]

		def __str__ (self):
			descr = "Version: {}.{}.{}".format (self.major, self.minor, self.release)
			return descr


	# ============================================================================
	class Config(ctypes.Structure):
		"""Description of the 'Segment' structure"""
	# ============================================================================
		_fields_ = [("alphabetSize", ctypes.c_int),
					("maxWordLength", ctypes.c_int)]

	# ============================================================================
	class SolverConfig(ctypes.Structure):
		"""Description of the 'Segment' structure"""
	# ============================================================================
		_fields_ = [("seed", ctypes.c_uint),
					("maxBlackBoxes", ctypes.c_int),
					("heuristicLevel", ctypes.c_int),
					("blackMode", ctypes.c_int)]

	# ============================================================================
	class Status(ctypes.Structure):
		"""Description of the 'Segment' structure"""
	# ============================================================================
		_fields_ = [("counter", ctypes.c_ulonglong),
					("fillRate", ctypes.c_int)]

		def __str__ (self):
			string = "Counter: {}\nFill rate: {}%".format (self.counter, self.fillRate)
			return string


	# ============================================================================
	def __init__ (self, dll_path):
		"""Constructor

		param	dll_path		path to the libWizium dll/so file to use"""
	# ============================================================================

		# Link to the lPPMM dll
		if platform.system()=='Linux':
			self._dll = ctypes.CDLL(dll_path)
		else:
			self._dll = ctypes.WinDLL (dll_path)	

		# Create a wrapper for each dll/so function
		self._api = {}
		self._api_def = {}
		self._api_def ["WIZ_Init"] = (ctypes.c_int, [ctypes.POINTER (Wizium.Version)])
		self._api_def ["WIZ_CreateInstance"] = (ctypes.c_ulonglong, [ctypes.POINTER (Wizium.Config)])
		self._api_def ["WIZ_DestroyInstance"] = (ctypes.c_int, [ctypes.c_ulonglong])
		self._api_def ["DIC_Clear"] = (ctypes.c_int, [ctypes.c_ulonglong])
		self._api_def ["DIC_AddEntries"] = (ctypes.c_int, [ctypes.c_ulonglong, ctypes.POINTER (ctypes.c_uint8), ctypes.c_int])
		self._api_def ["DIC_FindEntry"] = (ctypes.c_bool, [ctypes.c_ulonglong, ctypes.POINTER (ctypes.c_uint8), ctypes.POINTER (ctypes.c_uint8), ctypes.POINTER (ctypes.c_uint8)])
		self._api_def ["DIC_FindRandomEntry"] = (ctypes.c_bool, [ctypes.c_ulonglong, ctypes.POINTER (ctypes.c_uint8), ctypes.POINTER (ctypes.c_uint8)])
		self._api_def ["DIC_GetNumWords"] = (ctypes.c_uint, [ctypes.c_ulonglong])
		self._api_def ["GRID_SetSize"] = (ctypes.c_uint, [ctypes.c_ulonglong, ctypes.c_uint8, ctypes.c_uint8])
		self._api_def ["GRID_SetBox"] = (ctypes.c_uint, [ctypes.c_ulonglong, ctypes.c_uint8, ctypes.c_uint8, ctypes.c_int])
		self._api_def ["GRID_Write"] = (ctypes.c_uint, [ctypes.c_ulonglong, ctypes.c_uint8, ctypes.c_uint8, ctypes.POINTER (ctypes.c_uint8), ctypes.c_char, ctypes.c_bool])
		self._api_def ["GRID_Read"] = (ctypes.c_uint, [ctypes.c_ulonglong, ctypes.POINTER (ctypes.c_uint8)])
		self._api_def ["GRID_Erase"] = (ctypes.c_uint, [ctypes.c_ulonglong])
		self._api_def ["SOLVER_Start"] = (ctypes.c_uint, [ctypes.c_ulonglong, ctypes.POINTER (Wizium.SolverConfig)])
		self._api_def ["SOLVER_Step"] = (ctypes.c_uint, [ctypes.c_ulonglong, ctypes.c_int, ctypes.c_int, ctypes.POINTER (Wizium.Status)])
		self._api_def ["SOLVER_Stop"] = (ctypes.c_uint, [ctypes.c_ulonglong])

		for func_name in self._api_def:
			return_type = self._api_def [func_name][0]
			arg_types = self._api_def [func_name][1]
			if platform.system()=='Linux':
				proto = ctypes.CFUNCTYPE (return_type, *arg_types)
			else:
				proto = ctypes.WINFUNCTYPE (return_type, *arg_types)
			api = proto ((func_name, self._dll), None)

			self._api [func_name] = (api, proto)

		# Init library once and create one PPMM instance
		if Wizium._init_done == False:
			self.version = self._wiz_init ()
			print ("lPPMM library v{}.{}.{} loaded".format (self.version.major, self.version.minor, self.version.release))
			if str (self.version.major) + '.' + str (self.version.minor) != __version__:
				print ("[WARNING] Wrapper version doesn't match loaded library")
			Wizium._init_done = True

		# Create an instance
		self._wiz_create_instance ()
		if self._instance == 0:
			raise Exception ("lPPMM Library HANDLE could not be created")		


	# ============================================================================
	def __del__ (self):
		"""Destructor"""
	# ============================================================================

		# Destroy PPMM instance
		self._wiz_destroy_instance ()


	# ============================================================================
	def dic_clear (self):
		"""Flush the dictionary content"""
	# ============================================================================

		instance = ctypes.c_ulonglong (self._instance)

		(api, proto) = self._api ["DIC_Clear"]
		api (instance)


	# ============================================================================
	def dic_add_entries (self, entries):
		"""Add entries to the dictionary
		
		entries:		List of strings (must only contain ascii characters)
		return:			Number of words added to the dictionary
		"""
	# ============================================================================

		# Create an array to hold all the words
		m = self._max_word_length
		tab = bytearray (m * len (entries))
		ctab = (ctypes.c_uint8 * len (tab)).from_buffer (tab)

		# Put the list in the array
		skip = 0
		for idx, entry in enumerate (entries):
			be = bytearray (entry, 'ascii')
			if len (be) > self._max_word_length:
				skip += 1
				print ("Skip " + entry)
				continue

			ctab [(idx-skip)*m : (idx-skip)*m + len (be)] = be

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["DIC_AddEntries"]
		return api (instance, ctab, len (entries) - skip)


	# ============================================================================
	def dic_find_random_entry (self, mask):
		"""Find a random entry in the dictionary, matching a mask
		
		mask:		Mask to match. Each letter must either be in ['A'..'Z'] range
					or be a wildcard '*'. The wildcard means that any letter matches.
					Word length to retrieve is implicitly given by the mask length. (ex: '***' means any 3 letters word)

		return:		A matching word, or None
		"""
	# ============================================================================

		length = len (mask)
		tab = bytearray (length +1)
		ctab = (ctypes.c_uint8 * (length +1)).from_buffer (tab)

		tab_mask = bytearray (length +1)
		tab_mask [0:length] = bytearray (mask, 'ascii')
		tab_mask [length] = 0
		cmask = (ctypes.c_uint8 * (length +1)).from_buffer (tab_mask)

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["DIC_FindRandomEntry"]
		success = api (instance, ctab, cmask)

		if not success: return None
		else: return str (ctab, 'ascii')


	# ============================================================================
	def dic_find_entry (self, mask, start=None):
		"""Find a random entry in the dictionary, matching a mask
		
		mask:		Mask to match. Each letter must either be in ['A'..'Z'] range
					or be a wildcard '*'. The wildcard means that any letter matches.
					Word length to retrieve is implicitly given by the mask length. (ex: '***' means any 3 letters word)
		start:		A word to start searching in the dictionary. If None, search starts at the begining.		

		return:		A matching word, or None
		"""
	# ============================================================================

		length = len (mask)
		tab = bytearray (length +1)
		ctab = (ctypes.c_uint8 * (length +1)).from_buffer (tab)

		tab_mask = bytearray (length +1)
		tab_mask [0:length] = bytearray (mask, 'ascii')
		cmask = (ctypes.c_uint8 * (length +1)).from_buffer (tab_mask)
		
		if start:
			start_mask = bytearray (length +1)
			start_mask [0:len (start)] = bytearray (start, 'ascii')
			cstart = (ctypes.c_uint8 * (length +1)).from_buffer (start_mask)
		else:
			cstart = None

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["DIC_FindEntry"]
		success = api (instance, ctab, cmask, cstart)

		if not success: return None
		else: return str (ctab, 'ascii')			


	# ============================================================================
	def dic_gen_num_words (self):
		"""Return the number of words in the dictionary"""
	# ============================================================================

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["DIC_GetNumWords"]
		return api (instance)


	# ============================================================================
	def grid_erase (self):
		"""Erase the grid content"""
	# ============================================================================

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["GRID_Erase"]
		return api (instance)


	# ============================================================================
	def grid_set_size (self, width, height):
		"""Set the grid size. Content can be lost when shrinking."""
	# ============================================================================

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["GRID_SetSize"]
		self._width = width
		self._height = height		
		return api (instance, width, height)


	# ============================================================================
	def grid_set_box (self, x, y, type):
		"""Set the type of box at a given grid coordinate"""
	# ============================================================================

		assert type in ('LETTER', 'VOID', 'BLACK')

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["GRID_SetBox"]

		if type == 'LETTER': type_int = 0
		elif type == 'VOID': type_int = 1
		elif type == 'BLACK': type_int = 2

		api (instance, x, y, type_int)
		

	# ============================================================================
	def grid_write (self, x, y, word, dir='H', add_block=False):
		"""Write a word on the grid
		
		x, y		Location of the first letter
		word		Word to write
		dir			'V' or 'H' to selection orientation
		add_block	Optional black box at the end of the word
		"""
	# ============================================================================
		
		ba = bytearray (len (word) +1)
		ba [0:len (word)] = bytearray (word, 'ascii')
		bword = (ctypes.c_uint8 * (len (word) +1)).from_buffer (ba)

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["GRID_Write"]
		
		api (instance, x, y, bword, ord (dir [0]), add_block)


	# ============================================================================
	def grid_read (self):
		"""Read the whole content of the grid"""
	# ============================================================================

		size = self._width * self._height
		if not size: return None

		tab = bytearray (size)
		ctab = (ctypes.c_uint8 * size).from_buffer (tab)

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["GRID_Read"]
		api (instance, ctab)

		grid = [''] * self._height
		for i in range (self._height):
			grid [i] = str (tab [self._width*i: self._width*(i+1)], 'ascii') + '\n'

		return grid


	# ============================================================================
	def solver_start (self, seed=0, black_mode='DIAG', max_black=0, heuristic_level=-1):
		"""Start the grid generation process
		
		seed			Custom seed for the generation process
		black_mode		'DIAG':
						'ANY':
						'TWO':
						'SINGLE':
		max_black		Max. number of black boxes that can be added to the grid
		heuristic_level	Heuristic strength. -1: no heuristic
		"""
	# ============================================================================
		
		assert black_mode in ('DIAG', 'ANY', 'TWO', 'SINGLE')
		
		config = Wizium.SolverConfig ()
		config.seed = seed
		config.heuristicLevel = heuristic_level
		config.maxBlackBoxes = max_black
		if black_mode == 'DIAG':
			config.blackMode = 3
		elif black_mode == 'ANY':
			config.blackMode = 0
		elif black_mode == 'TWO':
			config.blackMode = 2
		elif black_mode == 'SINGLE':
			config.blackMode = 1

		(api, proto) = self._api ["SOLVER_Start"]
		instance = ctypes.c_ulonglong (self._instance)
		api (instance, ctypes.byref (config))
		

	# ============================================================================
	def solver_step (self, max_time_ms=-1, max_steps=-1):
		"""Move a few steps in the grid generation process"""
	# ============================================================================

		status = Wizium.Status ()

		(api, proto) = self._api ["SOLVER_Step"]
		instance = ctypes.c_ulonglong (self._instance)
		api (instance, max_time_ms, max_steps, ctypes.byref (status))

		return status


	# ============================================================================
	def solver_stop (self):
		"""Stop the grid generation process"""
	# ============================================================================

		(api, proto) = self._api ["SOLVER_Stop"]
		instance = ctypes.c_ulonglong (self._instance)
		api (instance)


	# ############################################################################	
	#
	# P R I V A T E
	#
	# ############################################################################	
	
	# ============================================================================
	def _wiz_init (self):
	# ============================================================================

		version = Wizium.Version ()
		(api, proto) = self._api ["WIZ_Init"]
		api (ctypes.byref (version))

		return version


	# ============================================================================
	def _wiz_create_instance (self, alphabet_size=0, max_word_length=20):
	# ============================================================================

		config = Wizium.Config ()
		config.alphabetSize = alphabet_size
		config.maxWordLength = max_word_length

		self._max_word_length = max_word_length
		self._alphabet_size = alphabet_size

		(api, proto) = self._api ["WIZ_CreateInstance"]
		self._instance = api (ctypes.byref (config))
		self._width = self._height = 0


	# ============================================================================
	def _wiz_destroy_instance (self):
	# ============================================================================

		instance = ctypes.c_ulonglong (self._instance)
		(api, proto) = self._api ["WIZ_DestroyInstance"]
		api (instance)