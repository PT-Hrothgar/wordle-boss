"""Stubs for wordle_boss.solvers extension module"""

from typing import Sequence, Optional


class Solver:
    def __init__(
        self,
        word_list: list[str],
        guess_list: Optional[list[str]] = None
    ) -> None:
        ...
    def add_guess(self, guess: str, feedback: Sequence[int], /) -> None: ...
    def get_guesses(self, /) -> list[tuple[str, list[int]]]: ...
    def remove_guess(self, guess: Optional[str] = None, /) -> None: ...
    def is_word_possible(self, guess: str, /) -> bool: ...
    def get_possibilities(self, /) -> list[str]: ...
    def get_min_feedback(self, guess: str, /) -> tuple[list[int], int]: ...
    def best_guess(
        self,
        hard_mode: bool = False,
        absurdle_target: Optional[str] = None
    ) -> str:
        ...


def get_response(target: str, guess: str, /) -> list[int]: ...


N: int
FB_ABSENT: int
FB_CORRECT: int
FB_PRESENT: int
