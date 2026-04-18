# Configuration file for the Sphinx documentation builder.
from pathlib import Path
import sys


# -- Project information
project = "wordle-boss"
copyright = "2026, Philip Turner"
author = "PT-Hrothgar"

sys.path.insert(1, str(Path(__file__).resolve().parent.parent.parent))

release = __import__("version").get_version()
version = ".".join(release.split(".")[:2])

# Restore sys.path to original version
sys.path.pop(1)

# -- General configuration

extensions = [
    "sphinx.ext.duration",
    "sphinx.ext.doctest",
    "sphinx.ext.autodoc",
    "sphinx.ext.autosummary",
    "sphinx.ext.intersphinx",
]

intersphinx_mapping = {
    "python": ("https://docs.python.org/3/", None),
    "sphinx": ("https://www.sphinx-doc.org/en/master/", None),
    "requests": ("https://requests.readthedocs.io/en/latest/", None),
    "html5lib": ("https://html5lib.readthedocs.io/en/latest/", None)
}
intersphinx_disabled_domains = ["std"]

templates_path = ["_templates"]

# -- Options for HTML output

html_theme = "sphinx_rtd_theme"

# -- Options for EPUB output
epub_show_urls = "footnote"
