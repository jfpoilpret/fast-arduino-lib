#!/usr/bin/python
# encoding: utf-8

# This script generates a sheet (ready for LibreOffice import as tab-delimited)
# with all FastArduino examples code and data sizes for all MCU targets

from __future__ import with_statement
import argparse, re, sys

ALL_EXAMPLES = []
ALL_SIZES = {}

# Read template sheet
def read_template():
    global ALL_EXAMPLES
    global ALL_SIZES
    with open("make/examples-list.txt", "r") as handle:
        for line in handle:
            data = line.split('\t')
            ALL_EXAMPLES.append(data[0])
            ALL_SIZES[data[0]] = {
                'description': data[1].rstrip(),
                'sizes': {}
            }

def fill_target(target):
    global ALL_SIZES
    filename = "newsizes-%s" % target
    with open(filename, "r") as handle:
        for line in handle:
            data = line.split('\t')
            example = data[0]
            if example in ALL_EXAMPLES:
                sizes = ALL_SIZES[example]['sizes']
                sizes[target] = [data[1], data[2].rstrip()]

def create_sheet(args):
    global ALL_EXAMPLES
    global ALL_SIZES
    with args.output:
        # Write header #1
        args.output.write("Example\tDescription")
        for target in args.targets:
            args.output.write("\t%s\t" % target)
        args.output.write("\n")
        # Write header #2
        args.output.write("\t")
        for target in args.targets:
            args.output.write("\tcode\tdata")
        args.output.write("\n")
        # Write rows
        for example in ALL_EXAMPLES:
            data = ALL_SIZES[example]
            description = data['description']
            sizes = data['sizes']
            args.output.write("%s\t%s" % (example, description))
            for target in args.targets:
                if target in sizes:
                    size = sizes[target]
                    args.output.write("\t%s\t%s" % (size[0], size[1]))
                else:
                    args.output.write("\t\t")
            args.output.write("\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'XXXXXXXX')
    parser.add_argument('output', nargs='?', type=argparse.FileType('w'), default=sys.stdout)
    parser.add_argument('targets', nargs=argparse.REMAINDER, type=str)
    args = parser.parse_args()

    # First read list of all examples and prepare structure to hold all information
    read_template()
    # Then for each target, fill in the information
    for target in args.targets:
        fill_target(target)
    # Finally create sheet from all collected info
    create_sheet(args)
