"""
Get the smartest Wordle guesses you'll find all day.

Documentation is at https://wordle-boss.readthedocs.io/en/stable
"""

__all__ = [
    "FB_ABSENT",
    "FB_CORRECT",
    "FB_PRESENT",
    "get_response",
    "get_past_answers",
    "get_word_list",
    "LONG_LIST",
    "N",
    "SHORT_LIST",
    "Solver"
]

__version__ = "1.0.3"

from wordle_boss.lists import get_past_answers, get_word_list
from wordle_boss.solvers import Solver, get_response
from wordle_boss.names import (
    FB_ABSENT,
    FB_CORRECT,
    FB_PRESENT,
    LONG_LIST,
    N,
    SHORT_LIST
)
