"""Create lists of past and future Wordle answers."""

try:
    from bs4 import BeautifulSoup
    import html5lib
    import requests
except ImportError:
    past_answers_installed = False
else:
    past_answers_installed = True

from pathlib import Path
import warnings

from wordle_boss.solvers import N

_cwd = Path(__file__).resolve().parent

SHORT_LIST = str(_cwd / "short_list.txt")
LONG_LIST = str(_cwd / "long_list.txt")

# Webpages with a list of past Wordle answers.
# Each page is represented by a 2-tuple containing the page's URL
# and a CSS selector that can be used to get all the DOM elements
# containing the words themselves.
past_answer_list_options = [
    (
        "http://rockpapershotgun.com/wordle-past-answers",
        ".article_body_content ul.inline li"
    ),
    (
        "http://wordfinder.yourdictionary.com/wordle/answers",
        "table.w-full.border-collapse.text-left.text-black.text-base"
        " tbody tr:not(:has(button)) td.rounded-r-2.px-4.py-3.font-bold"
    ),
    (
        "http://wordraiders.com/solvers/past-wordle-answers",
        ".wordle-list li.wordle-item"
    ),
    (
        "http://wordlehints.co.uk/wordle-past-answers",
        "#wordlehintsTable tr td:nth-child(3)"
    )
]


def get_past_answers() -> list[str]:
    """
    Get an alphabetized list of past Wordle answers from a website.

    This function uses a predefined list of webpages, retrieving the HTML
    of each page and attempting to parse it until one page works.

    :raises ValueError: on failure to retrieve a word list from any of the
                        online options.
    :raises RuntimeError: if :ref:`past-answers-dependency` is not
                          installed.
    :returns: A list of :py:const:`~wordle_boss.names.N`-letter strings
              representing all the past Wordle answers, retrieved from a
              webpage and sorted alphabetically.
    :rtype: list[str]
    """
    if not past_answers_installed:
        raise RuntimeError("past-answers dependency is not installed.")

    # Loop through all online options until one works
    for url, selector in past_answer_list_options:
        # Attempt to retrieve webpage's HTML
        try:
            response = requests.get(url)
        except Exception as e:
            continue

        if response.status_code != 200:
            continue

        # Parse the HTML
        soup = BeautifulSoup(response.text, "html5lib")
        words = set()
        success = True

        # Query by the CSS selector
        for elem in soup.select(selector):
            # Get only the last N non-whitespace characters of each element
            try:
                word = elem.string.strip()[-N:]
            except Exception as e:
                success = False
                break

            # Ensure the word looks the way we expect
            if len(word) != N or not word.isalpha():
                success = False
                break

            words.add(word.upper())

        if not success:
            continue

        words = sorted(words)

        if len(words) <= 1:
            continue

        return words

    raise ValueError(
        "Failed to get a past answers list from any online option"
    )


def get_word_list(
    filename: str,
    exclude_past_answers: bool = False,
    errors: int = 2
) -> list[str]:
    """
    Get a list of words from the given file.

    Leading and trailing whitespace in the file is ignored. Each line of
    the file must contain a word consisting of
    :py:const:`~wordle_boss.names.N` uppercase letters.

    :param str filename: Name/path of the file to use. Often set to
                        :py:const:`~wordle_boss.names.SHORT_LIST` or
                        :py:const:`~wordle_boss.names.LONG_LIST`.
    :param bool exclude_past_answers: If True, get a list of all past
                                      answers using
                                      :py:func:`get_past_answers` and remove
                                      them from the returned list. This
                                      requires the
                                      :ref:`past-answers-dependency` to be
                                      installed.

    .. warning::
        The usefulness of the ``exclude_past_answers`` feature is
        questionable since Wordle began reusing past answers on February 2,
        2026.

    :param int errors: How to handle any exception raised by
                       :py:func:`get_past_answers`. If set to 2 (the
                       default), allow the exception to propagate; if set to
                       1, issue a warning; if set to 0, do nothing. In the
                       last two cases, the returned value will simply
                       contain all the words from the file if
                       :py:func:`get_past_answers` raises an exception.

    :returns: A list containing all the words from the given file, excluding
              all words returned by :py:func:`get_past_answers` if
              ``exclude_past_answers`` is True.
    :rtype: list[int]
    """
    if errors not in range(3):
        raise ValueError("errors must be an integer from 0 to 2")

    with open(filename) as file:
        words = file.read().strip().split("\n")

    if exclude_past_answers:
        try:
            past_answers = get_past_answers()
        except Exception as e:
            past_answers = []

            if errors == 2:
                raise
            elif errors == 1:
                warnings.warn(str(e), stacklevel=2)

        words = sorted(set(words) - set(past_answers))

    return words


if __name__ == "__main__":
    print("These are the past Wordle answers:")
    print(*get_past_answers(), sep="\n")
    print("These are the future Wordle answers:")
    print(*get_word_list(SHORT_LIST, True), sep="\n")
