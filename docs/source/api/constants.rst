.. py:module:: wordle_boss.names
.. py:currentmodule:: wordle_boss.names

API: Constants
==============

.. important::

    Treat these values as constants: do not try to change them!

All of these values are exposed in the module :py:mod:`wordle_boss.names`, and
can thus be imported with this wildcard import:

>>> from wordle_boss.names import *
>>> N
5

They are also attributes of the main package, ``wordle_boss``:

>>> import wordle_boss as wb
>>> wb.N
5

.. py:data:: N
    :type: int
    :value: 5

    Number of letters in a :ref:`wordle-game` guess or target word. (Import
    this value to avoid hardcoding it into your code.)

.. attention::

    There is no publicly available list of all the possible Wordle target
    words. Any list, such as that contained in the file
    :py:const:`~wordle_boss.names.SHORT_LIST`, is only an approximation of the
    list actually used by the game.

.. py:data:: SHORT_LIST
    :type: str

    The full path to a file provided with the package containing a list of
    the most common :py:const:`~wordle_boss.names.N`-letter words.

    This file is commonly used as the list of possible target words. In fact,
    it is identical to the list used by :ref:`absurdle-game`. However, it is
    **shorter** than the list used by :ref:`wordle-game`, and so will provide
    somewhat inaccurate analyses of actual Wordle games played on
    https://nytimes.com.

.. py:data:: LONG_LIST
    :type: str

    The full path to a file provided with the package containing a list of
    over 10,000 common and uncommon :py:const:`~wordle_boss.names.N`-letter
    words, i.e. the legal guesses in :ref:`wordle-game`. This list is identical
    to the "expanded list" that can be enabled in :ref:`absurdle-game`.

.. _feedback-values:

Feedback Values
---------------

These three values all represent "feedback" given by the Wordle algorithm to a
guess, or the background colors of the letters of the guess. They should be
used in the ``feedback`` argument of :py:meth:`wordle_boss.Solver.add_guess`,
and are also used in the return values of :py:func:`wordle_boss.get_response`,
:py:meth:`wordle_boss.Solver.get_guesses`, and
:py:meth:`wordle_boss.Solver.get_min_feedback`.

.. py:data:: FB_ABSENT
    :type: int

    Represents a letter turned black or gray (⬜), meaning that it is not
    present in the target word.

.. py:data:: FB_PRESENT
    :type: int

    Represents a letter turned yellow (🟨), meaning that it is present in the
    target word but in a different position.

.. py:data:: FB_CORRECT
    :type: int

    Represents a letter turned green (🟩), meaning that it is present in the
    target word and in the correct position.
