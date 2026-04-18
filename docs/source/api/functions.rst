.. py:module:: wordle_boss
.. py:currentmodule:: wordle_boss

API: Functions
===============

.. important::

    **General API notes**: Please do not make use of any undocumented
    submodules of :py:mod:`wordle_boss` (such as ``wordle_boss.lists``). Import
    constants from :py:mod:`wordle_boss.names` and everything else from
    :py:mod:`wordle_boss`.

    Additionally, there is no public C API. The C files in this package provide
    a Python API and nothing else intended for public use.

.. autofunction:: get_past_answers
.. autofunction:: get_word_list

.. py:function:: get_response(target: str, guess: str, /) -> list[int]

    Get the Wordle algorithm's response to the given guess.

    :param str target: Target word being guessed.
    :param str guess: Attempt to guess ``target``.
    :returns: List of :ref:`feedback values <feedback-values>` representing the
              Wordle algorithm's response to the given target and guess.
    :rtype: list[int]

    Example:

    >>> wb.get_response("SLEEP", "SCREE")
    [2, 0, 0, 2, 1]
