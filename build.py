#!/usr/bin/env python3

import argparse
import pathlib
import subprocess
import sys
import tempfile
from datetime import datetime

parser = argparse.ArgumentParser(
    prog="build", description="Build an OMeta or C++ source file."
)

parser.add_argument("input", help="Input .ometa or .cpp file.")

parser.add_argument(
    "-o",
    "--output",
    help="Output executable. Defaults to the input filename without its extension.",
)

parser.add_argument(
    "--cpp", help="Path for the generated C++ file. Defaults to a temporary file."
)

parser.add_argument(
    "--transpiler", default="build/ometa-cpp", help="Transpiler executable (default: build/ometa-cpp)."
)

parser.add_argument(
    "--ometa-include", default="include", help="Standard OMeta include directory (default: include)."
)

parser.add_argument("--debug", action="store_true", help="Compile with debug flags.")

parser.add_argument(
    "--release", action="store_true", help="Compile with optimizations."
)

args = parser.parse_args()

input_path = pathlib.Path(args.input)

if input_path.suffix not in [".ometa", ".cpp"]:
    sys.exit("Input must have a .ometa or .cpp extension.")

output = pathlib.Path(args.output) if args.output else input_path.with_suffix("")
output.parent.mkdir(parents=True, exist_ok=True)

if input_path.suffix == ".ometa":
    if args.cpp:
        cpp = pathlib.Path(args.cpp)
    else:
        timestamp = datetime.now().strftime("%Y_%m_%d__%H_%M_%S")
        cpp = pathlib.Path(tempfile.gettempdir()) / f"{output.name}_{timestamp}.cpp"

    cpp.parent.mkdir(parents=True, exist_ok=True)

    subprocess.run(
        [
            args.transpiler,
            str(input_path),
            str(cpp),
        ],
        check=True,
    )
else:
    cpp = input_path

cmd = [
    "clang++",
    "-std=c++20",
    "-I" + args.ometa_include,
    "-ferror-limit=1",
    "-ftemplate-backtrace-limit=0",
]

if args.debug:
    cmd += ["-O0", "-DDEBUG_PRINTS"]
elif args.release:
    cmd += ["-O3"]
else:
    cmd += ["-O0"]

cmd += [
    str(cpp),
    "-o",
    str(output),
]

subprocess.run(cmd, check=True)
