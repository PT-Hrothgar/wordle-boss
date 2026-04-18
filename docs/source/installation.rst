Installation
============

This package requires CPython 3.9 or later.

Installation from PyPI (recommended)
------------------------------------

No matter what platform you are on, start with this command for installation::

    python -m pip install wordle-boss

Alternatively, install the :ref:`past-answers-dependency`::

    python -m pip install wordle-boss[past-answers]

Binary wheels are provided for 64-bit Linux, macOS, and Windows Intel platforms,
so installation on them should be very straightforward.

If you are on a different platform (such as Windows ARM64), installation will
involve building a wheel, which can be a complicated process. You'll be on your
own for that.


.. _source-install:

Installation from source
------------------------

An appropriate C compiler must be installed on your OS.

#. Clone and enter the repository::

       git clone https://github.com/PT-Hrothgar/wordle-boss
       cd wordle-boss

#. Install with ``pip``::

       python -m pip install .

   ``pip`` should follow the build instructions in the file ``pyproject.toml``
   to build a binary wheel, and then install the package from it.

#. Run tests (optional)::

    python -m unittest discover -s tests


.. _past-answers-dependency:

``past-answers`` dependency
---------------------------

This optional project dependency includes the modules :py:mod:`requests`,
`Beautiful Soup 4`_, and :py:mod:`html5lib` for HTML retrieving and parsing.

To install it, use the following command::

    python -m pip install wordle-boss[past-answers]

This will allow the collection of past Wordle answers from online sources using
the function :py:func:`~wordle_boss.get_past_answers`.

.. _Beautiful Soup 4: https://www.crummy.com/software/BeautifulSoup/bs4/doc/
