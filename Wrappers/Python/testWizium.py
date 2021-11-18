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
import time
import functools
import operator
from libWizium import Wizium

# ############################################################################

# Update those paths if needed !
if platform.system()=='Linux':
    PATH = './../../Binaries/Linux/libWizium.so'
elif platform.system()=='Darwin':
    PATH = './../../Binaries/Darwin/liblibWizium.dylib'
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
def solve (wiz, max_black=0, heuristic_level=0, seed=0, black_mode='DIAG'):
    """Solve the grid

    wiz             Wizium instance
    max_black       Max number of black cases to add (0 if not allowed)
    heuristic_level Heuristic level (0 if deactivated)
    seed            Random Number Generator seed (0: take at random)
    """
# ============================================================================

    if not seed: seed = random.randint(1, 1000000)

    # Configure the solver
    wiz.solver_start (seed=seed, black_mode=black_mode, max_black=max_black, heuristic_level=heuristic_level)
    tstart = time.time ()

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

    tend = time.time ()
    print ("Compute time: {:.01f}s".format (tend-tstart))

def example_1():
    # Create a Wizium instance
    wiz = Wizium (os.path.join (os.getcwd (), PATH))
    # Load the dictionary
    load_dictionary (wiz, DICO_PATH)

    set_grid_1 (wiz)
    solve (wiz, max_black=0, heuristic_level=2)

def example_2():
    # Create a Wizium instance
    wiz = Wizium (os.path.join (os.getcwd (), PATH))
    # Load the dictionary
    load_dictionary (wiz, DICO_PATH)

    set_grid_2 (wiz)
    solve (wiz, max_black=30, heuristic_level=2)

def example_3():
    # Create a Wizium instance
    wiz = Wizium (os.path.join (os.getcwd (), PATH))
    # Load the dictionary
    load_dictionary (wiz, DICO_PATH)

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
    solve (wiz, max_black=0, heuristic_level=0)

def example_4():
    dictionary = sorted(
        """
        ali lepa pač pol oči češ koli oče plač pokal ekipa lep kola šola kopač pik
        išče loka klop ako češki čelo lek peči špela peš koš čop šok pil
        plošča lik peč kip pika pel kapo plašč očka čok alpe kol šale čela
        kap špik lok pak pekla lipa čile koča kopališče pola kali pike lepša
        pila pleča peka čip opel kal loški ček laški šopek poka lipe opla šop
        eko pok šal paki koč očali apel klepač polič čopa plošček kač oli
        pošle kolišče oklep šepa ila pičlo kopel oča piščal kopal ilo kleč
        polka kape pikl piš kič kop čik kolač peki api pek kopa palček kapič
        čeka pičel peška opeka laik šipek poček čil špika pišek kepa peklo
        kopič polk oček pilo pelo kliše poli kalič leči lak alo šilo pleška
        lečo lašč liko čipka pleč opa polke peča klep očak šik aki lopa pečal
        leča šipa klešč poč paš lop pleša pekač loč ščap ščip laki pečka
        leški lišp klopa šček kep pišče pači kila čep peški šopa šipka olika
        epika šap lepak kališče piška lakič opal keš oka čap piške paški čak
        pako kiša čao plašček špila lička košič špil kli poliček ilka eia
        poleči čopka šlek lošč kleča čoka alk leš ipak čelika pečak
        lišpa opekač klo keč ščipa loček količ okel čepa klope klip opeči
        ščep klap šlk pilka kečap kalo epik akel eki kopišče klošč
        """.split())

    alphabet = "".join(sorted(functools.reduce(operator.or_, map(set, dictionary))))

    print(f"Dictionary with {len(dictionary)} words uses alphabet '{alphabet}'")

    # Create a Wizium instance
    wiz = Wizium (os.path.join (os.getcwd (), PATH),
                  alphabet=alphabet)

    # Load dictionary
    wiz.dic_clear()
    n = wiz.dic_add_entries(dictionary)

    wiz.grid_set_size(10, 10)
    solve(wiz, max_black=25, heuristic_level=2, black_mode='DIAG')

# ============================================================================
"""Main"""
# ============================================================================

# -->  C H O O S E  <--
EXAMPLE = 4


# Example with fixed pattern
if EXAMPLE == 1:
    example_1()

# Example with dynamic black cases placement
elif EXAMPLE == 2:
    example_2()
# Perfect 9x9 resolution example (french)
# Need 24e+9 tests in the worst case, which may take ~10hours
elif EXAMPLE == 3:
    example_3()
# Use foreign alphabet
elif EXAMPLE == 4:
    example_4()

exit ()
