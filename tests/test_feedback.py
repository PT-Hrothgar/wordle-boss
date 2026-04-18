import unittest

from wordle_boss import get_response, N


class FeedbackTest(unittest.TestCase):
    def test_simple_cases(self):
        tests = (
            ("CIGAR", "CIGAR", [2] * N),
            ("CIGAR", "STARE", [0, 0, 1, 1, 0]),
            ("CIGAR", "SUGAR", [0, 0, 2, 2, 2]),
            ("CIGAR", "CHINA", [2, 0, 1, 0, 1])
        )

        for target, guess, feedback in tests:
            self.assertEqual(get_response(target, guess), feedback)

    def test_repeated_letters(self):
        tests = (
            ("SLEEP", "EERIE", [1, 1, 0, 0, 0]),
            ("SLEEP", "SCREE", [2, 0, 0, 2, 1]),
            ("EERIE", "TABLE", [0, 0, 0, 0, 2]),
            ("TABLE", "SLEEP", [0, 1, 1, 0, 0]),
            ("SISSY", "RINSE", [0, 2, 0, 2, 0])
        )

        for target, guess, feedback in tests:
            self.assertEqual(get_response(target, guess), feedback)


if __name__ == "__main__":
    unittest.main()
