import unittest

from wordle_boss.names import *
from wordle_boss import Solver, get_word_list

MINI_LIST = ["CIGAR", "SISSY", "REBUT"]


class SolverTest(unittest.TestCase):
    def test_solver_init(self):
        self.assertRaises((ValueError, TypeError), Solver, (["CIGAR"], []))
        self.assertRaises((ValueError, TypeError), Solver, ([], ["CIGAR"]))

        Solver(*((["CIGAR"],) * 2))
        Solver(["CIGAR"], ["SISSY"])
        Solver(MINI_LIST, list(reversed(MINI_LIST)))

    def test_real_case(self):
        s = Solver(
            [f"{c}IGHT" for c in "ELFIN"],
            MINI_LIST + ["ELFIN"]
        )
        self.assertEqual(s.best_guess(), "ELFIN")

    def test_best_guess(self):
        s = Solver(
            [
                "KKKKA",
                "LLLLA",
                "MMMMA",
                "BBCFF",
                "GBCFF",
                "HHHHF",
                "IIIIF"
            ],
            ["ABCDE", "BACDE", "KLMNO"]
        )
        self.assertEqual(s.best_guess(), "BACDE")

    def test_hard_mode(self):
        s = Solver(
            [
                "ABWYZ",
                "ABXYZ",
                "CBXYZ",
                "CDXYZ",
                "EFXYZ",
                "EFWYZ"
            ],
            [
                "CBWWW",
                "EFOYZ",
                "ABWWW",
                "CBOYZ"
            ]
        )
        s.add_guess("MMMYZ", [0, 0, 0, 2, 2])
        self.assertEqual(s.best_guess(), "CBWWW")
        self.assertEqual(s.best_guess(hard_mode=True), "CBOYZ")

    def test_absurdle_target(self):
        s = Solver(
            [
                "ABCCC",
                "DECCC",
                "FGCCC",
                "ABHHH",
                "IJHHH",
                "KLHHH",
                "MNHHH"
            ],
            ["ABCCC", "ACIKM", "ACDFI"]
        )
        self.assertEqual(s.best_guess(absurdle_target="DECCC"), "ACIKM")

        s2 = Solver(
            [f"AAAA{c}" for c in "BCDEFGH"],
            ["DEFGH", "FGHIJ", "GHIJK", "AFGHI", "ABCDE"]
        )
        s2.add_guess("AXXXX", [2, 0, 0, 0, 0])
        self.assertEqual(s2.best_guess(), "DEFGH")
        self.assertEqual(s2.best_guess(hard_mode=True), "ABCDE")
        self.assertEqual(
            s2.best_guess(hard_mode=True, absurdle_target="AAAAB"),
            "AFGHI"
        )
