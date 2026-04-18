.. py:currentmodule:: wordle_boss

Basic Usage
===========

Much of ``wordle_boss``'s functionality is contained within the
:py:class:`Solver` class, which can be used like this:

>>> import wordle_boss as wb
>>> from wordle_boss.names import *
>>> # Create a Solver with the list of most common 5-letter words:
>>> s = wb.Solver(wb.get_word_list(SHORT_LIST))
>>> # No guesses have been added so far, so this will get the best first guess:
>>> s.best_guess()
'RAISE'
>>> # Add a guess with the first three letters gray and the last two yellow
>>> s.add_guess(
...     "STARE",
...     [
...         FB_ABSENT,
...         FB_ABSENT,
...         FB_ABSENT,
...         FB_PRESENT,
...         FB_PRESENT
...     ]
... )
>>> # Best second guess in this situation:
>>> s.best_guess()
'PRIOR'
>>> # Best second guess in "hard mode", where each guess must incorporate all
>>> # the information from previous guesses:
>>> s.best_guess(hard_mode=True)
'DECOR'

In general, to use this package:

    #. Get a word list to use (or two, one of guesses and one of possible
       target words) using :py:func:`get_word_list`, normally passing it either
       :py:const:`~wordle_boss.names.SHORT_LIST` or
       :py:const:`~wordle_boss.names.LONG_LIST`.

    #. Create an instance of the :py:class:`Solver` class, passing it the word
       list(s).

    #. Add guesses using :py:meth:`Solver.add_guess`.

    #. Get all possible remaining target words using
       :py:meth:`Solver.get_possibilities`.

    #. Get the best next guess according to the MINIMAX algorithm using
       :py:meth:`Solver.best_guess`.
