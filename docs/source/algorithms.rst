Algorithm Overview
==================

Explanations of the algorithms used by Wordle Boss and the games it attempts to
solve.


.. _wordle-game:

Wordle
------

Wordle, originally created in 2021 by Josh Wardle, is an online puzzle game in
which the player has six chances to guess a five-letter word (the "target
word"). The target word changes each day, and is the same for players all over
the world. Each guess must be a valid five-letter word, and every time the
player makes a guess, each letter of the guess changes color, giving
information ("feedback") about the target word.

* If a letter turns black or gray (⬜), that letter is not present in the target
  word.
* If a letter turns yellow (🟨), that letter is present in the target word, but
  in a different position.
* If a letter turns green (🟩), that letter is present in the target word and is
  in the correct position.

The algorithm is fairly simple; for example, if the target word is "SKATE" and
the guess is "CANOE", the feedback should be ⬜🟨⬜⬜🟩.

The only complication is with repeated letters, for which the following two
rules apply:

* If there is a choice between turning a letter of the guess yellow and turning
  it green, it should be turned **green**. So, if the target word is "SLEEP"
  and the guess is "DREAM", the feedback should be ⬜⬜🟩⬜⬜.
* Every letter in the guess that is marked as "present" (by turning yellow or
  green) must refer to a **unique** letter in the target word. So, if the
  target word is "CHUTE" and the guess is "SLEEP", the feedback should be
  ⬜⬜🟨⬜⬜. (The second E is not turned yellow, because there is only one E in the
  target word.)

Wordle is hosted by the New York Times at
https://nytimes.com/games/wordle/index.html. See the
`Wikipedia page <https://en.wikipedia.org/wiki/Wordle>`__ for more information.

The Wordle algorithm can be replicated by Wordle Boss using the function
:py:func:`wordle_boss.get_response`.

.. _wordle-hard-mode:

Wordle Hard Mode
~~~~~~~~~~~~~~~~

"Hard mode" is an optional setting in the Wordle game which, in the New York
Times's words, adds the rule that "Any revealed hints must be used in
subsequent guesses". This sounds like it means that all guesses must be
consistent with the feedback you have received so far: you may not guess a word
you have eliminated. (That is how :py:meth:`wordle_boss.Solver.best_guess`
interprets the hard mode rules).

However, in practice, hard mode is somewhat less strict. It is unclear exactly
what restrictions it places on your guess, as you are still allowed to guess
letters that you have eliminated, or letters that are in positions that you
have eliminated. For example, even if your first guess contains some yellow and
some gray letters, you will still be allowed to use the exact same word for
your second guess. In fact, the only obvious rule added by hard mode is that
any letter that you discover is in the word must be present, in some position,
in all subsequent guesses.


.. _absurdle-game:

Absurdle
--------

Absurdle (https://qntm.org/files/absurdle/absurdle.html) is an adversarial
version of Wordle. The game does not decide on a target word beforehand;
rather, for each guess it gives the feedback that eliminates the fewest words
from the list of possible target words. In essence, it gives the most
"disappointing" possible feedback for every guess.

This algorithm, finding the optimal feedback for a given guess, is also an
integral part of the :ref:`MINIMAX strategy <minimax-alg>` used by Wordle Boss.
It can be replicated using the method
:py:meth:`wordle_boss.Solver.get_min_feedback`.

A detailed explanation of the game is available at https://qntm.org/absurdle.

To win Absurdle (which always requires four or more guesses), it is necessary
to make a clever guess that is guaranteed to eliminate all but one word no
matter what feedback the game gives you, and then guess the one remaining word.

.. _challenge-mode:

Absurdle Challenge Mode
~~~~~~~~~~~~~~~~~~~~~~~

Because its algorithm is completely predictable, Absurdle provides a "challenge
mode" for variety. In this mode, the game behaves exactly the same way, but you
are given a random target word *beforehand* and you only win if you narrow the
list down to one word, *and* that word is the one you were given. Challenge
mode is very difficult without machine help (such as is provided by Wordle
Boss).

A detailed explanation of challenge mode is available at
https://qntm.org/challenge.


Wordle Boss
-----------

The package that solves it all.

.. _minimax-alg:

MINIMAX Algorithm
~~~~~~~~~~~~~~~~~

Wordle boss uses a
`MINIMAX algorithm <https://en.wikipedia.org/wiki/Minimax>`__ to find smart
Wordle and Absurdle guesses. This is not necessarily the optimal strategy, as
it only looks one guess into the future, but there is no very obvious better
one except exhaustively testing every possible guess sequence, which would be
extraordinarily slow.

The strategy is to determine the minimum feedback (see :ref:`absurdle-game`)
for every possible guess, and the number of words that it would leave, and pick
the guess that maximizes that minimum feedback (minimizes the number of words).
The algorithm will, for example, always find a guess that is guaranteed to
eliminate all words but one, if such a guess exists. For example, a player who
guesses "SIGHT" and receives feedback ⬜🟩🟩🟩🟩 is in a tricky situation, as there
are many words ending in "IGHT". This algorithm will find words like "ELFIN"
that try many possibilities for the first letter at once.

.. _valid-words:

Valid Words
~~~~~~~~~~~

The :py:class:`wordle_boss.Solver` class considers "valid words" to be Python
strings (:py:class:`str` instances) consisting of
:py:const:`~wordle_boss.names.N` uppercase ASCII letters. Lowercase letters are
**not allowed**!
