import unittest
import os

from wordle_boss import get_word_list

words = """\
CIGAR
REBUT
SISSY"""


class WordListTest(unittest.TestCase):
    @classmethod
    def tearDownClass(cls):
        if os.path.exists("list.txt"):
            os.remove("list.txt")

    def test_file_reading(self):
        with open("list.txt", "w") as file:
            file.write(words)

        self.assertEqual(
            get_word_list("list.txt"),
            ["CIGAR", "REBUT", "SISSY"]
        )

    def test_whitespace_handling(self):
        with open("list.txt", "w") as file:
            file.write(f"\n {words}\t\n")

        self.assertEqual(
            get_word_list("list.txt"),
            ["CIGAR", "REBUT", "SISSY"]
        )


if __name__ == "__main__":
    unittest.main()
