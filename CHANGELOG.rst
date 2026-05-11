Changelog
=========

v1.0.2 (latest)
---------------

Prefer guesses that are possible target words, all other things being equal.

Because the package does not distinguish between eliminating all words but one
and actually guessing the word and eliminating all other possibilities that
way, it formerly exhibited behavior such as the following that was not ideal:

>>> s = wb.Solver(wb.get_word_list(SHORT_LIST))
>>> s.add_guess(
...     "STARE",
...     [FB_ABSENT, FB_ABSENT, FB_CORRECT, FB_PRESENT, FB_ABSENT]
... )
>>> s.add_guess(
...     "DRAIN",
...     [FB_ABSENT, FB_CORRECT, FB_CORRECT, FB_CORRECT, FB_CORRECT]
... )
>>> s.get_possibilities()
['BRAIN', 'GRAIN']
>>> s.best_guess()
'ABACK'

It is true in this case that guessing "ABACK" is guaranteed to eliminate all
words but one, and the same is true of the two words "BRAIN" and "GRAIN"
themselves. But it is evidently better in this case to guess one of those two,
thus preserving the possibility of getting the word on this guess.

In this release, if there is no preference between two possible guesses based on
the number of words they are guaranteed to eliminate, but one is a possible
target word itself and the other is not, the package will prefer the possible
target word.

v1.0.1
---------------

Initial release.
