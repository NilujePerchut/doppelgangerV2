#!/usr/bin/env python3
import re
import argparse

WRAP_LEN = len("__wrap_")

def wrap_extract(path):
    """Extracts the wrap ld flags from fiven file"""
    with open(path) as file:
        res = set(re.findall(r"__wrap_\w+", file.read()))
    for name in res:
        print(F"-Wl,--wrap={name[WRAP_LEN:]} ", end="")
    print("")

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("test_file", help="File to parse")
    args = parser.parse_args()
    wrap_extract(args.test_file)

