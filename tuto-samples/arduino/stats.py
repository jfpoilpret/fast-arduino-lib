#!/usr/bin/python
# encoding: utf-8

# In order to use this script from shell:
# > ./build-samples >tempsizes
# > cat tempsizes | ./stats.py >sizes
# > rm tempsizes
# Then sizes file can be opened in LibreOffice Calc

from __future__ import with_statement
import argparse, re, sys

#TODO from example name, extract tutorial tag, index, and postfix
def filter(args):
    bytes_extractor = re.compile(r"([0-9]+) bytes")
    with args.output:
        with args.input:
            for line in args.input:
                if line.find("Building... ") >= 0:
                    # Find example name (everything after last /)
                    example = line[line.find(" ") + 1:-1]
                elif line.startswith("Sketch uses"):
                    # Find number of bytes of flash
                    matcher = bytes_extractor.search(line)
                    program = matcher.group(1)
                elif line.startswith("Global variables use"):
                    # Find number of bytes of SRAM
                    matcher = bytes_extractor.search(line)
                    data = matcher.group(1)
                    # Write new line to output
                    args.output.write("%s\t%s\t%s\n" % (example, program, data))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'XXXXXXXX')
    parser.add_argument('input', nargs='?', type=argparse.FileType('r'), default=sys.stdin)
    parser.add_argument('output', nargs='?', type=argparse.FileType('w'), default=sys.stdout)
    args = parser.parse_args()
    filter(args)
