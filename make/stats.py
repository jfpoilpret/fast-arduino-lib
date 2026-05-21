#!/usr/bin/python3
# encoding: utf-8

# In order to use this script from shell:
# > make CONF=UNO clean-examples
# > make CONF=UNO examples >tempsizes
# > cat tempsizes | ./stats.py >sizes
# > rm tempsizes
# Then sizes file can be opened in LibreOffice Calc

from __future__ import with_statement
import argparse, re, sys

def filter(args):
    bytes_extractor = re.compile(r" ([0-9]+) ")
    with args.output:
        with args.input:
            text = data = bss = 0
            for line in args.input:
                if line.find("avr-size") >= 0:
                    # Find example name (everything after last /)
                    example = line[line.rfind("/") + 1:-1]
                elif line.startswith(".text"):
                    # Find number of bytes of flash
                    matcher = bytes_extractor.search(line)
                    text = int(matcher.group(1))
                elif line.startswith(".data"):
                    # Find number of bytes of SRAM
                    matcher = bytes_extractor.search(line)
                    data = int(matcher.group(1))
                elif line.startswith(".bss"):
                    # Find number of bytes of SRAM
                    matcher = bytes_extractor.search(line)
                    bss = int(matcher.group(1))
                elif line.startswith("Total"):
                    # Write new line to output
                    args.output.write("%s\t%d\t%d\n" % (example, text + bss, data + bss))

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'XXXXXXXX')
    parser.add_argument('input', nargs='?', type=argparse.FileType('r'), default=sys.stdin)
    parser.add_argument('output', nargs='?', type=argparse.FileType('w'), default=sys.stdout)
    args = parser.parse_args()
    filter(args)

