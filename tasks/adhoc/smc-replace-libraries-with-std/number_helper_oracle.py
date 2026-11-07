#!/usr/bin/env python3
"""Oracle: prove the std::to_chars/from_chars number helpers match the old ones.

Slices the helper functions out of the OLD core/property_helper.cpp (from a git
ref, default HEAD) and the NEW one (working tree) into two namespaces of one
harness program, compiles it, and replays data/values.txt (discover.sh) plus
edge cases through both. Prints every mismatch; exit status 1 if any.

    python3 number_helper_oracle.py [old-ref]
"""

import pathlib
import subprocess
import sys
import tempfile

HERE: pathlib.Path = pathlib.Path(__file__).resolve().parent
ROOT: pathlib.Path = pathlib.Path(
    subprocess.run(["git", "rev-parse", "--show-toplevel"], cwd=HERE,
                   capture_output=True, text=True, check=True).stdout.strip())
REL: str = "smc/src/core/property_helper.cpp"
START: str = "std::string int_to_string(const int number) {"
END: str = "unsigned int string_to_version_number(std::string str) {"


def helpers(text: str) -> str:
    """Return the helper-function region of a property_helper.cpp text."""
    return text[text.index(START):text.index(END)]


def main() -> int:
    old_ref: str = sys.argv[1] if len(sys.argv) > 1 else "HEAD"
    old: str = subprocess.run(["git", "show", f"{old_ref}:{REL}"], cwd=ROOT,
                              capture_output=True, text=True, check=True).stdout
    new: str = (ROOT / REL).read_text()
    harness: str = (HERE / "harness_main.cpp.in").read_text()
    source: str = harness.replace("@OLD@", helpers(old)).replace("@NEW@", helpers(new))
    with tempfile.TemporaryDirectory() as tmp:
        cpp: pathlib.Path = pathlib.Path(tmp) / "oracle.cpp"
        exe: pathlib.Path = pathlib.Path(tmp) / "oracle"
        cpp.write_text(source)
        subprocess.run(["g++", "-std=c++23", "-O1", "-o", str(exe), str(cpp)], check=True)
        return subprocess.run([str(exe), str(HERE / "data" / "values.txt")]).returncode


if __name__ == "__main__":
    sys.exit(main())
