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

# ############################################################################	

import os
import re
import platform
import random
from libWizium import Wizium

# ############################################################################	

# Update those paths if needed !
if platform.system()=='Linux':
    PATH = './../../Binaries/Linux/libWizium.so'
else:
    PATH = './../../Binaries/Windows/libWizium_x64.dll'

DICO_PATH = './../../Dictionaries/Fr_Simple.txt'


# ============================================================================
def draw (wiz):
    """Draw the grid content, with a very simple formating
        
    wiz     Wizium instance"""
# ============================================================================
    lines = wiz.grid_read ()
    for l in lines:
        print (''.join ([s + '   ' for s in l]))


# ============================================================================
def set_grid_1 (wiz):
    """Set the grid skeleton with a pattern of black boxes
        
    wiz     Wizium instance"""
# ============================================================================

    tx = [0, 2, 3]

    wiz.grid_set_size (11,11)
    wiz.grid_set_box (5, 5, 'BLACK')

    for i in range (3):
        wiz.grid_set_box (tx [i], 5-tx [i], 'BLACK')
        wiz.grid_set_box (5+tx [i], tx [i], 'BLACK')
        wiz.grid_set_box (10-tx [i], 5+tx [i], 'BLACK')
        wiz.grid_set_box (5-tx [i], 10-tx [i], 'BLACK')

    wiz.grid_set_box (5, 1, 'BLACK')
    wiz.grid_set_box (5, 9, 'BLACK')


# ============================================================================
def set_grid_2 (wiz):
    """Set the grid as a rectangular area with a hole at the center
    
    wiz     Wizium instance"""
# ============================================================================

    # Grid size
    wiz.grid_set_size (17,15)

    # Hole
    for i in range (5):
        for j in range (5):
            wiz.grid_set_box (6+i, 5+j, 'VOID')

    # Place some words on the grid
    wiz.grid_write (0,0, 'CONSTRAINT', 'H', add_block=True)
    wiz.grid_write (16,5, 'CONSTRAINT', 'V', add_block=True)
    wiz.grid_set_box (16, 4, 'BLACK')


# ============================================================================
def load_dictionary (wiz, dico_path):
    """Load the dictionary content from a file
        
    wiz         Wizium instance
    dico_path   Path to the dictionary to load
    """
# ============================================================================

    # Read file content
    with open (dico_path, 'r') as f:
        words = f.readlines ()

    # Remove what is not a letter, if any
    words = [re.sub('[^a-zA-Z]+', '', s) for s in words]
  
    # Load dictionary
    wiz.dic_clear ()
    n = wiz.dic_add_entries (words)

    print ("Number of words: ")
    print (" - in file: ", len (words))
    print (" - added: ", n)
    print (" - final: ", wiz.dic_gen_num_words ())


# ============================================================================
def solve (wiz, max_black=0, seed=0):
    """Solve the grid
        
    wiz         Wizium instance
    max_black   Max number of black cases to add (0 if not allowed)
    seed        Random Number Generator seed (0: take at random)
    """
# ============================================================================

    if not seed: seed = random.randint(1, 1000000)

    # Configure the solver
    wiz.solver_start (seed=seed, black_mode='DIAG', max_black=max_black, heuristic_level=2)
    
    # Solve with steps of 500ms max, in order to draw the grid content evolution
    while True:
        status = wiz.solver_step (max_time_ms=500) 

        draw (wiz)
        print (status)

        if status.fillRate == 100: 
            print ("SUCCESS !")
            break
        if status.fillRate == 0: 
            print ("FAILED !")
            break
    
    # Ensure to release grid content
    wiz.solver_stop ()


# ============================================================================
"""Main"""
# ============================================================================

# -->  C H O O S E  <--
EXAMPLE = 1

# Create a Wizium instance
wiz = Wizium (os.path.join (os.getcwd (), PATH))

# Load the dictionary
load_dictionary (wiz, DICO_PATH)


# Example with fixed pattern
if EXAMPLE == 1:
    set_grid_1 (wiz)
    solve (wiz, 0)

# Example with dynamic black cases placement
elif EXAMPLE == 2:
    set_grid_2 (wiz)
    solve (wiz, 30)

# Perfect 9x9 resolution example (french)
# Need 24e+9 tests in the worst case, which may take ~10hours
elif EXAMPLE == 3:

    # Add the missing words needed to be able to solve the grid
    words = ['REABRASES',
        'ENCRENENT',
        'OCTOCORDE',
        'CHICORIUM',
        'RAVAUDERA',
        'EPARTIRAS',
        'RENIERONS',
        'ALTERANTE',
        'SASSASSES',
        'REOCRERAS',
        'ENCHAPELA',
        'ACTIVANTS',
        'BROCARIES',
        'RECOUTERA',
        'ANORDIRAS',
        'SERIERONS',
        'ENDURANTE',
        'STEMASSES',]    
    wiz.dic_add_entries (words)
    
    wiz.grid_set_size (9,9)
    solve (wiz, 0)

