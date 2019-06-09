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

"""Compute expected transition times depending on the paving algorithm"""
# ############################################################################	

import math
import numpy as np

# ############################################################################	

# Number of words in the dictionary, depending on their length
N_tab = [0, 26, 98, 525, 2069, 6160, 13385, 22596, 31337, 36325]

# Probability of coincidence
pco = 0.078

# Average number of words in the dictionary, given the prefix length
M0 =  []
M1 = [N_tab [1]]
M2 = [N_tab [2], 4.1]
M3 = [N_tab [3], 21.0, 3.1]
M4 = [N_tab [4], 82.8, 9.6, 2.0]
M5 = [N_tab [5], 236.9, 25.6, 3.9, 1.7]
M6 = [N_tab [6], 514.8, 53.8, 7.4, 2.6, 1.6]
M7 = [N_tab [7], 869.1, 94.2, 12.5, 4.1, 2.1, 1.5]
M8 = [N_tab [8], 1205.3, 130, 17.7, 5.8, 3.1, 1.9, 1.4]
M9 = [N_tab [9], 1397.1, 153.9, 21.8, 7.2, 4.0, 2.6, 1.7, 1.3]

M_tab = [M0, M1, M2, M3, M4, M5, M6, M7, M8, M9]

# =============================================================================
def scaff_pmatch (S):
    """Compute a vector of size S, giving the probabilities to be successful
    when chosing a word in the (Stack) backtracking algorithm

    S:      Grid size
    return: Vector of probabilities"""
# =============================================================================
    
    N = N_tab [S]
    pmatch = [0] * S
    for i in range (S):
        
        # Number of remaining vertical words
        M = M_tab [S] [i]

        # Probability that 1 horizontal word fails for all M words in a column
        pfc = (1-pco)**M
        pmc = 1-pfc

        # Probability to succeed for all columns
        pmr = pmc**S
        pfr = 1-pmr

        # Probability every N words fail
        pf = pfr**N
        pm = 1-pf

        pmatch [i] = pm
    return pmatch


# =============================================================================
def weave_pmatch (S):
    """Compute a vector of size S, giving the probabilities to be successful
    when chosing a word in the (Weave) backtracking algorithm

    S:      Grid size
    return: Vector of probabilities"""
# =============================================================================
    
    N = N_tab [S]
    v = [1 - (1-pco**i)**N for i in range (S+1)]
    v2 =  [v [(i+1)//2] for i in range (2*S)]
    return v2


# =============================================================================
def make_Q (vector):
    """Compute the transient matrix Q of a Markov chain whose transition prob.
    is given by a vector"""
# =============================================================================

    n = len (vector) 
    Q = np.zeros ((n, n))

    for i in range (n -1):
        Q [i+1, i] = vector [i]
        Q [i, i+1] = 1.0 - vector [i+1]
    return Q

# =============================================================================
def make_N (Q):
    """Compute the fundamental matrix N from the transient matrix Q"""
# =============================================================================
    I = np.eye (Q.shape [0])
    N = np.linalg.inv (I-Q)

    return N

# =============================================================================
def simulate (vector):
    """Simulate the number of steps required to reach the absorbption state
    of a Markov chain, given the transition probabilities. 

    Useful to ensure the theorical results are correct when the number of steps is small

    vector:         Transition probabilities"""
# =============================================================================
    state = 0
    counter = 0

    while state < len (vector):
        counter += 1
        p = vector [state]
        c = np.random.choice ([-1, 1], p=[1-p, p])
        state += c
    
    return counter


S = 9
pscaff = scaff_pmatch (S)        
pweave = weave_pmatch (S)

Qscaff = make_Q (pscaff)
Qweave = make_Q (pweave)
Nscaff = make_N (Qscaff)
Nweave = make_N (Qweave)

steps_scaff = np.sum (Nscaff, axis=0) [0]
steps_weave = np.sum (Nweave, axis=0) [0]

print ("GRID SIZE: {}".format (S))
print ("\nTransition probabilities for the SCAFFOLDING method")
for idx, p in enumerate (pscaff):
    print (" - P. success of word {}: {:.03E}".format (idx, p))

print ("--> Average number of steps: {:.03E}".format (steps_scaff))

print ("\nTransition probabilities for the WEAVING method")
for idx, p in enumerate (pweave):
    layout = 'HORZ' if idx%2 == 0 else 'VERT'
    print (" - P. success of {} word {}: {:.03E}".format (layout, idx//2, p))

print ("--> Average number of steps: {:.03E}".format (steps_weave))


