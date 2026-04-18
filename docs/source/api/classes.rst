.. py:currentmodule:: wordle_boss

API: Classes
============

.. py:class:: Solver(word_list: list[str], guess_list: Optional[list[str]] = None)

    Python object type providing much of Wordle Boss's functionality.

    See :doc:`../usage` for example of usage.

    :param word_list: List containing all the words that the :py:class:`Solver`
                      instance will consider to be possible target words.
    :type word_list: list[str]
    :param guess_list: (Optional) List containing all the words that the
                       :py:class:`Solver` instance will consider to be legal
                       guesses. If it is ``None`` or not provided,
                       ``word_list`` will be used for the list of guesses.
    :type guess_list: list[str] or None

    Neither of these two word lists can be changed after the object is created.

    :raises ValueError: if either of the provided lists contains an element
                        other than a :ref:`valid word <valid-words>`, or is
                        provided but empty.

    .. py:method:: add_guess(guess: str, feedback: Sequence[int], /) -> None

        Add a guess to ``self``.

        This guess, and the provided feedback for it, will be taken into
        account in all future calls to any of ``self``'s methods unless it is
        removed using :py:meth:`~wordle_boss.Solver.remove_guess`.

        :param str guess: String to add as guess.
        :param feedback: Sequence of :py:const:`N`
                         :ref:`feedback values <feedback-values>`.
        :type feedback: Sequence[int]
        :raises ValueError: if ``guess`` is not a
                            :ref:`valid word <valid-words>`, or if ``feedback``
                            is not of the correct type.
        :raises RuntimeError: if ``guess`` has already been added to ``self``.

    .. py:method:: best_guess(hard_mode: bool = False, absurdle_target: Optional[str] = None) -> str

        Get the best possible next guess according to the :ref:`minimax-alg`.

        :param bool hard_mode: If ``True``, the returned guess will be
                               consistent with all the guesses and feedback
                               added so far to ``self``. Default ``False``. See
                               :ref:`wordle-hard-mode`.
        :param absurdle_target: (Optional) Desired target word in
                                :ref:`challenge-mode`. Must be a possible
                                target word. If not ``None``, the returned
                                guess will be "safe"; it will not eliminate
                                ``absurdle_target`` if guessed.
        :type absurdle_target: str or None
        :returns: Best guess according to the :ref:`minimax-alg` that follows
                  the restrictions of the two parameters. Exception: if there
                  is only one target word remaining according to
                  :py:meth:`~wordle_boss.Solver.get_possibilities`, that word
                  will be returned regardless of the parameter values.
                  Otherwise, the returned word is always an element of the
                  guess list used when creating ``self``.
        :rtype: str
        :raises ValueError: if ``absurdle_target`` is provided but is not a
                            :ref:`valid word <valid-words>`, or is not one of
                            the words returned by
                            :py:meth:`~wordle_boss.Solver.get_possibilities`.
        :raises RuntimeError: if there are no possible target words remaining.
        :raises RuntimeError: if ``hard_mode`` is ``True`` but all the elements
                              of ``self``'s guess list are eliminated by
                              guesses.
        :raises RuntimeError: if ``absurdle_target`` is provided but all
                              possible guesses would eliminate it.

    .. py:method:: get_guesses() -> list[tuple[str, list[int]]]

        Return all the guesses added so far to ``self``.

        Each element of the returned list is a 2-tuple containing a guess that
        has been added to ``self`` and the feedback for that guess.

        Guesses are returned in the order in which they were added.

        Example:

        >>> s.add_guess("STARE", [0, 0, 0, 1, 1])
        >>> s.add_guess("DOING", [0, 0, 0, 0, 0])
        >>> s.get_guesses()
        [('STARE', [0, 0, 0, 1, 1]), ('DOING', [0, 0, 0, 0, 0])]

        :rtype: list[tuple[str, list[int]]]

    .. py:method:: get_min_feedback(guess: str, /) -> tuple[list[int], int]

        Get the most "disappointing" possible feedback for the given guess.

        That is, find the feedback for the guess that would eliminate the
        fewest possible words from the list of possibilities (the list that
        would be returned by :py:meth:`~wordle_boss.Solver.get_possibilities`.)
        See :ref:`absurdle-game`. Return this and the number of possible target
        words that the feedback would leave.

        :param str guess: Word to analyze.
        :returns: Tuple containing a list of integers
                  (:ref:`feedback values <feedback-values>`) representing the
                  most disappointing possible feedback for ``guess``, and the
                  number of possibilities that the feedback would leave.
        :rtype: tuple[list[int], int]
        :raises ValueError: if ``guess`` is not a
                            :ref:`valid word <valid-words>`.
        :raises RuntimeError: if there are no possible target words remaining.

        .. note::

            This method uses a special algorithm to break ties between two or
            more responses that would leave the same number of target words. To
            be as useful as possible for playing Absurdle, it attempts to break
            ties exactly the way Absurdle does; however, it is not clear
            exactly what method Absurdle uses. This method generally
            approximates it fairly well, but may fail sometimes.

    .. py:method:: get_possibilities() -> list[str]

        Get all the remaining possible target words.

        :returns: List of all the elements of the word list ``self`` was
                  created with that are not eliminated by the guesses added to
                  ``self``.
        :rtype: list[str]

    .. py:method:: is_word_possible(word: str, /) -> bool

        Determine if a word could be the target word for ``self``.

        :param str word: Word to test against ``self``'s guesses.
        :returns: ``True`` if ``word`` is consistent with the guesses and
                  feedback added to ``self``, otherwise ``False``.
        :rtype: bool
        :raises ValueError: if ``word`` is not a
                            :ref:`valid word <valid-words>`.

    .. py:method:: remove_guess(guess: Optional[str] = None, /) -> None

        Remove a guess, or all guesses, previously added to ``self``.

        Removed guesses are no longer taken into account in any calls to
        ``self``'s methods.

        :param guess: (Optional) Guess to remove from ``self``. If it is
                      ``None`` or not provided, all guesses will be removed.
        :type guess: str or None
        :raises ValueError: if ``guess`` is provided but is not a
                            :ref:`valid word <valid-words>`, or has not been
                            added to ``self``.
