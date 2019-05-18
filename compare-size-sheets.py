#!/usr/bin/python
# encoding: utf-8

# This script compares 2 sheets with FastArduino examples code and data sizes 
# for all MCU targets. It generates a new sheet with absolute and relative 
# differences.

from __future__ import with_statement
import argparse, re, sys

ALL_TARGETS = []
ALL_EXAMPLES = []
ALL_SIZES = {}

# Read template sheet
def read_template():
    global ALL_EXAMPLES
    global ALL_SIZES
    with open("examples-list.txt", "r") as handle:
        for line in handle:
            data = line.split('\t')
            ALL_EXAMPLES.append(data[0])
            ALL_SIZES[data[0]] = {
                'sizes1': {},
                'sizes2': {}
            }

def fill_data(infile, index):
    global ALL_SIZES
    global ALL_TARGETS
    with infile:
        num_line = 1
        targets = []
        for line in infile:
            data = line.split('\t')
            if num_line == 1:
                # Read targets list from headers
                targets = [ t.rstrip() for t in data[2:] if t.rstrip() ]
            elif num_line != 2:
                # Store sizes for all targets of current example
                example = data[0]
                sizes = ALL_SIZES[example]['sizes%d' % index]
                info = [ d.rstrip() for d in data[2:] ]
                for i, target in enumerate(targets):
                    if info[2 * i]:
                        sizes[target] = [info[2 * i], info[2 * i + 1]]
            num_line += 1
    ALL_TARGETS = targets

def print_diff(output, size1, size2):
    diff = int(size2) - int(size1)
    diff_percent = 100.0 * diff / int(size1) if int(size1) > 0 else 0.0
    output.write("\t%d\t%d\t%.0f" % (int(size1), diff, diff_percent))

def create_diff_sheet(args):
    global ALL_EXAMPLES
    global ALL_SIZES
    with args.output:
        # Write header #1
        args.output.write("Example")
        for target in ALL_TARGETS:
            args.output.write("\t%s\t\t\t\t\t" % target)
        args.output.write("\n")
        # Write header #2
        for target in ALL_TARGETS:
            args.output.write("\tcode\tdiff\tdiff%\tdata\tdiff\tdiff%")
        args.output.write("\n")
        # Write rows
        for example in ALL_EXAMPLES:
            data = ALL_SIZES[example]
            sizes1 = data['sizes1']
            sizes2 = data['sizes2']
            args.output.write("%s" % example)
            for target in ALL_TARGETS:
                if target in sizes1 and target in sizes2:
                    size1 = sizes1[target]
                    size2 = sizes2[target]
                    print_diff(args.output, size1[0], size2[0])
                    print_diff(args.output, size1[1], size2[1])
                else:
                    args.output.write("\t\t\t\t\t\t")
            args.output.write("\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'XXXXXXXX')
    parser.add_argument('inputs', nargs=2, type=argparse.FileType('r'))
    parser.add_argument('output', nargs='?', type=argparse.FileType('w'), default=sys.stdout)
    args = parser.parse_args()

    # First read list of all examples and prepare structure to hold all information
    read_template()
    # Read each input file
    fill_data(args.inputs[0], 1)
    fill_data(args.inputs[1], 2)
    # Finally create sheet with differences
    create_diff_sheet(args)
