import unittest

from wordle_boss.names import *
from wordle_boss import Solver, get_word_list

MINI_LIST = ["CIGAR", "SISSY", "REBUT"]


class SolverTest(unittest.TestCase):
    words = ["CIGAR", "SISSY", "REBUT"]

    def test_solver_init(self):
        for arg_list in (
            (),
            (1, 2),
            ("cat",),
            (tuple(MINI_LIST),),
            ([1, 2, 3],),
            (["CIGAR", "CAT"],),
            ([],),
            (["Cigar", "SISSY"],)
        ):
            self.assertRaises((TypeError, ValueError), Solver, *arg_list)

        Solver(["CIGAR"])
        Solver(MINI_LIST)

    def test_add_guess(self):
        s = Solver(MINI_LIST)

        for arg_list in (
            (),
            (1, 2),
            ("BOOK", [0, 0, 0, 0]),
            ("CIGAR",),
            ("CIGAR", []),
            ("CIGAR", "00000"),
            ("CIGAR", [0, 0, 0, 0, 3])
        ):
            self.assertRaises((TypeError, ValueError), s.add_guess, *arg_list)

        s.add_guess("CIGAR", [0, 0, 0, 0, 0])
        s.add_guess("SISSY", [2, 0, 1, 0, 1])
        s.add_guess("ABCDE", [0, 0, 0, 0, 1])
        s.add_guess("FGHIJ", (0, 0, 0, 0, 0))

        self.assertRaises(RuntimeError, s.add_guess, "CIGAR", [0, 0, 0, 0, 0])

    def test_get_guesses(self):
        s = Solver(MINI_LIST)
        self.assertEqual(s.get_guesses(), [])

        s.add_guess("CIGAR", [0, 0, 0, 0, 0])
        s.add_guess("SISSY", [2, 0, 1, 0, 1])

        self.assertEqual(
            s.get_guesses(),
            [
                ("CIGAR", [0, 0, 0, 0, 0]),
                ("SISSY", [2, 0, 1, 0, 1])
            ]
        )

    def test_remove_guess(self):
        s = Solver(MINI_LIST)
        s.remove_guess()
        self.assertEqual(s.get_guesses(), [])

        s.add_guess("CIGAR", [0, 0, 0, 0, 0])
        self.assertRaises(ValueError, s.remove_guess, "SISSY")

        s.add_guess("SISSY", [2, 0, 1, 0, 1])
        s.remove_guess("SISSY")

        self.assertEqual(
            s.get_guesses(),
            [
                ("CIGAR", [0, 0, 0, 0, 0])
            ]
        )

        s.add_guess("SISSY", [2, 0, 1, 0, 1])
        s.add_guess("STYES", [2, 2, 2, 2, 2])
        s.remove_guess()
        self.assertEqual(s.get_guesses(), [])

    def test_is_word_possible(self):
        s = Solver(MINI_LIST)

        self.assertTrue(s.is_word_possible("CIGAR"))
        self.assertTrue(s.is_word_possible("REBUT"))
        self.assertTrue(s.is_word_possible("ABCDE"))
        self.assertRaises(ValueError, s.is_word_possible, "BOOK")

        s.add_guess("CIGAR", [0, 0, 0, 0, 0])
        s.add_guess("SISSY", [2, 0, 1, 0, 1])

        self.assertTrue(s.is_word_possible("STYES"))
        self.assertTrue(s.is_word_possible("SQYQS"))
        self.assertFalse(s.is_word_possible("CIGAR"))
        self.assertFalse(s.is_word_possible("ABCDE"))
        self.assertFalse(s.is_word_possible("SALSA"))
        self.assertFalse(s.is_word_possible("ABYSS"))

    def test_get_possibilities(self):
        s = Solver(MINI_LIST)
        self.assertEqual(set(s.get_possibilities()), set(MINI_LIST))

        s.add_guess("MOMMY", [0, 0, 0, 0, 0])
        self.assertEqual(
            set(s.get_possibilities()),
            set(MINI_LIST).difference({"SISSY"})
        )

        words = get_word_list(SHORT_LIST)

        s = Solver(words)
        s.add_guess("STARE", [0, 0, 0, 1, 1])
        result = s.get_possibilities()

        for word in ("INNER", "LOWER", "REPLY", "PERCH"):
            self.assertTrue(word in result)

        for word in ("ALIEN", "XXXER", "GENRE", "ALERT"):
            self.assertFalse(word in result)

        for word in result:
            self.assertTrue(word in words)

        s.add_guess("DOING", [0, 0, 0, 0, 0])
        s.add_guess("LUMPY", [0, 0, 0, 0, 1])
        self.assertEqual(s.get_possibilities(), ["CYBER"])

        s.add_guess("PLEAT", [0, 0, 2, 0, 0])
        self.assertEqual(s.get_possibilities(), [])

    def test_get_min_feedback(self):
        words = [
            "AAAAA",
            "BBBBB",
            "CCCCC",
            "DDDDD",
            "FFFFE",
            "GGGGE",
            "EEEEE"
        ]
        s = Solver(words)
        self.assertEqual(
            s.get_min_feedback("EEEEE"),
            ([0, 0, 0, 0, 0], 4)
        )

        s.add_guess("HHHHE", [0, 0, 0, 0, 2])
        self.assertEqual(
            s.get_min_feedback("EEEEE"),
            ([0, 0, 0, 0, 2], 2)
        )

        s = Solver(get_word_list(SHORT_LIST))
        self.assertEqual(s.get_min_feedback("FUZZY")[0], [0, 0, 0, 0, 0])

    def test_min_feedback_errors(self):
        s = Solver(MINI_LIST)
        s.add_guess("CIGAR", [0, 0, 0, 0, 0])
        s.add_guess("REBUT", [1, 0, 0, 0, 0])

        self.assertRaises(RuntimeError, s.get_min_feedback, "CIGAR")
        self.assertRaises(RuntimeError, s.get_min_feedback, "AAAAA")

    def test_min_feedback_ties(self):
        words = [
            "AAAAA",
            "BBBBA",
            "CCCCA",
            "ADDDD",
            "AEEEE"
        ]
        s = Solver(words)
        self.assertEqual(
            s.get_min_feedback("AAAAA"),
            ([0, 0, 0, 0, 2], 2)
        )

        words = [
            "AAAAB",
            "AAABB",
            "XXXAB",
            "XXXBA",
            "ABCDE"
        ]
        s = Solver(words)
        self.assertEqual(
            s.get_min_feedback("ABCDE"),
            ([1, 1, 0, 0, 0], 2)
        )
        s.add_guess("XXXXX", [0, 0, 0, 0, 0])
        self.assertEqual(
            s.get_min_feedback("ABCDE"),
            ([2, 1, 0, 0, 0], 2)
        )

        words = [
            "AAAAB",
            "CCCCB",
            "BDEFG",
            "BFDEG",
            "DEFHB",
            "EDFHB"
        ]
        s = Solver(words)
        self.assertEqual(
            s.get_min_feedback("DEFHB"),
            ([1, 1, 1, 0, 1], 2)
        )


class BestGuessTest(unittest.TestCase):
    def test_one_word_remaining(self):
        s = Solver(MINI_LIST)

        s.add_guess("STARE", [0, 0, 1, 1, 0])
        self.assertEqual(s.best_guess(), "CIGAR")
        self.assertEqual(s.best_guess(absurdle_target="CIGAR"), "CIGAR")

        s.remove_guess("STARE")
        s.add_guess("STARE", [2, 0, 0, 0, 0])
        self.assertEqual(s.best_guess(), "SISSY")

        s.remove_guess("STARE")
        s.add_guess("STARE", [0, 1, 0, 1, 1])
        self.assertEqual(s.best_guess(), "REBUT")

    def test_errors(self):
        s = Solver(MINI_LIST)
        s.add_guess("STARE", [2, 2, 1, 1, 1])
        self.assertRaises(RuntimeError, s.best_guess)

        # Absurdle target is not one of the possibilities
        s.remove_guess("STARE")
        self.assertRaises(ValueError, s.best_guess, absurdle_target="HUMPH")

        s2 = Solver([
            "ABCDE",
            "AAAAA",
            "BBBBB",
            "CCCCC"
        ])
        # All possible guesses would eliminate the Absurdle target word
        self.assertRaises(RuntimeError, s2.best_guess, absurdle_target="ABCDE")

    def test_absurdle_target(self):
        l1 = ["AAAAA", "BBBBB", "CCCCC", "DDDDD"]
        s = Solver(l1)

        for word in l1:
            guess = s.best_guess(absurdle_target=word)
            self.assertTrue(guess in l1)
            self.assertNotEqual(guess, word)

        l2 = ["AAAAA", "BBBBB"]
        s2 = Solver(l2)

        for i in range(2):
            self.assertEqual(s2.best_guess(absurdle_target=l2[i]), l2[-i - 1])

    def test_simple_cases(self):
        s = Solver([
            "AAAAE",
            "BBBBE",
            "CECCC",
            "DEDDD",
            "FEFFF",
            "EGGGG",
            "EHHHH",
            "EEEEE"
        ])
        self.assertEqual(s.best_guess(), "EEEEE")

        l = [
            "AAAAA",
            "BBBBB",
            "CCCCC",
            "DDDDE",
            "FFFFE",
            "IIIFE",
            "GGGCE",
            "HHHCE"
        ]
        s2 = Solver(l)
        w = s2.best_guess()
        self.assertTrue(w in l[-2:])
        s2.add_guess(w, [0, 0, 0, 0, 2])
        self.assertTrue(s2.best_guess() in l[4:6])

    def test_real_case(self):
        s = Solver(["STARE", "ELFIN"] + [f"{c}IGHT" for c in "ELFNS"])
        s.add_guess("TIGHT", [0, 2, 2, 2, 2])
        self.assertEqual(s.best_guess(), "ELFIN")

    def test_ties(self):
        l = [
            "AAABB",
            "CCCCC",
            "DDDDD",
            "EEEEE",
            "AFFFF",
            "GAGGG",
            "HHAHH"
        ]
        s = Solver(l)
        self.assertEqual(s.best_guess(), "AAABB")
        self.assertTrue(s.best_guess(absurdle_target="AFFFF") in l[1:4])

        s2 = Solver([
            "ABCDE",
            "FFFFE",
            "GGGGE",
            "HHHHE",
            "IIIIE",
            "EEEAB",
            "JJJAB",
            "KKBAK",
            "LLLLL"
        ])
        self.assertEqual(s2.best_guess(), "EEEAB")
        self.assertEqual(s2.best_guess(absurdle_target="EEEAB"), "LLLLL")


if __name__ == "__main__":
    unittest.main()
