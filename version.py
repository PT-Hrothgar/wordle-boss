from pathlib import Path
import re

cwd = Path(__file__).resolve().parent


def get_version():
    with open(str(cwd / "src" / "wordle_boss" / "__init__.py")) as f:
        for line in f.readlines():
            if m := re.search(
                "(?<=__version__ = ['\"])[0-9.]+(?=['\"])",
                line
            ):
                return m.group()


if __name__ == "__main__":
    print(get_version())
