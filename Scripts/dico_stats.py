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

"""Compute statistics on a given dictionary"""
# ############################################################################	
 
import math
import re

# ############################################################################	

# =============================================================================
def get_words (path):
    """Open a list of words"""
# =============================================================================
    
    with open (path, 'r') as f:
        words = f.readlines ()
    
    words = [re.sub('[^a-zA-Z]+', '', s) for s in words]
    return words

# =============================================================================
def make_empty_substats (word_len):
    """Build empty dictionary to compute stats on all words of a given length"""
# =============================================================================

    stats = {'frequencies' : [0] * 26,
            'count': 0,
            'pco': 0.0,
            'prefix_count' : [{} for w in range (0, word_len)]}

    return stats

# =============================================================================
def make_empty_stats (max_length):
    """Buld empty dictionary to compute stats on all words"""
# =============================================================================
    stats = {'frequencies' : None,
            'bylength' : [make_empty_substats (i) for i in range (0, max_length+1)]}

    return stats

# =============================================================================
def process_sub_stats (substats, word):
    """Process a word to update statistics"""
# =============================================================================
    substats ['count'] += 1

    for l in range (1, len (word)):
        prefix = word [0:l]
        
        dic = substats ['prefix_count'][l]
        if prefix in dic: dic [prefix] += 1
        else: dic [prefix] = 1

    for letter in word:
        substats ['frequencies'] [ord (letter.upper ()) - ord ('A')] += 1


# =============================================================================
    """ Compute statistics on a dictionary"""
# =============================================================================

words = get_words ('./../Dictionaries/Fr_Simple.txt')
max_length = 15
stats = make_empty_stats (max_length)
stats ['count'] = len (words)

# Process words
for idx, w in enumerate (words):
    l = len (w)
    if l > max_length: continue
    substats = stats ['bylength'] [l]
    process_sub_stats (substats, w)
    # if idx > 1000: break
  
# Finalize stats
for substats in stats ['bylength']:

    # compute final frequencies
    v = substats ['frequencies']
    s = sum (v)
    if s == 0: continue
    v = [value / s for value in v]
    substats ['frequencies'] = v

    # Prob. of coincindence
    substats ['pco'] = sum ([x*x for x in substats ['frequencies']])

    # compute final number of words, given a prefix length (average)
    for idx, dic in enumerate (substats ['prefix_count']):
        v = dic.values ()        
        avg = sum (v) / len (v) if len (v) > 0 else None
        substats ['prefix_count'] [idx] = avg

# Compute letter frequency whatever the length
freq = [0] * 26
for substats in stats ['bylength']:
    v = [substats ['count'] * x for x in substats ['frequencies']]
    freq = [f + x for (f, x) in zip (freq, v)]

freq = [f / stats ['count'] for f in freq]
stats ['frequencies'] = freq
stats ['pco'] = sum ([x*x for x in stats ['frequencies']])

# Print results
print ("Total number of words: {}".format (stats ['count']))
print ("Total by length: ")
for l in range (1, max_length+1):
    print (" - [{}]: {}".format (l, stats ['bylength'][l]['count']))
print ("Probability of coincidence: {:.03f}".format (stats ['pco']))
print ("Probability by length: ")
for l in range (1, max_length+1):
    print (" - [{}]: {:.03f}".format (l, stats ['bylength'][l]['pco']))

for l in range (1, max_length+1):
    print ("Average number of words of length {} for a given prefix length".format (l))

    for pl in range (1, l):
        print (" - [{}]: {:.01f}".format (pl, stats ['bylength'][l]['prefix_count'][pl]))