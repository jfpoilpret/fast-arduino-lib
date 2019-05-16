#!/usr/bin/python
# encoding: utf-8

# This script generates a sheet (ready for LibreOffice import as tab-delimited)
# with all FastArduino examples code and data sizes for all MCU targets

from __future__ import with_statement
import argparse, re, sys

def read_sizes(file, sizes):
    with file:
        for line in file:
            data = line.split('\t')
            sizes[data[0].strip()] = {
                'code': data[1].strip(),
                'data': data[2].strip()
            }

TAG = "//! [%s]\n"

TABLE = """<table class="markdownTable">%s%s%s
</table>
"""

TABLE_HEADER = """
	<tr class="markdownTableHead">
		<th class="markdownTableHeadNone"></th>
%s
	</tr>"""
TABLE_HEADER_COL = """		<th class="markdownTableHeadNone">%s</th>"""

TABLE_ROW_1 = """
	<tr class="markdownTableBody" class="markdownTableRowOdd">
		<td class="markdownTableBodyNone">code size</td>
%s
	</tr>"""
TABLE_ROW_2 = """
	<tr class="markdownTableBody" class="markdownTableRowEven">
		<td class="markdownTableBodyNone">data size</td>
%s
	</tr>"""
TABLE_ROW_COL = """		<td class="markdownTableBodyNone">%s</td>"""

def format_size(size):
    return size + (" byte" if size in ["0", "1"] else " bytes")

def create_tables(config, sizes, output):
    with output:
        with config:
            for line in config:
                data = line.split('\t')
                tag = TAG % data[0].strip()

                labels = [cell.strip() for i, cell in enumerate(data[1:]) if i % 2 == 0]
                programs = [cell.strip() for i, cell in enumerate(data[1:]) if i % 2 == 1]

                code_sizes = [sizes[p]['code'] for p in programs]
                data_sizes = [sizes[p]['data'] for p in programs]

                header_cols = [TABLE_HEADER_COL % label for label in labels]
                code_cols = [TABLE_ROW_COL % format_size(size) for size in code_sizes]
                data_cols = [TABLE_ROW_COL % format_size(size) for size in data_sizes]
                
                header = TABLE_HEADER % '\n'.join(header_cols)
                row1 = TABLE_ROW_1 % '\n'.join(code_cols)
                row2 = TABLE_ROW_2 % '\n'.join(data_cols)

                table = TABLE % (header, row1, row2)

                output.write(tag)
                output.write(table)
                output.write(tag)
                output.write("\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description = 'XXXXXXXX')
    parser.add_argument('platform1', nargs='?', type=argparse.FileType('r'))
    parser.add_argument('platform2', nargs='?', type=argparse.FileType('r'))
    parser.add_argument('config', nargs='?', type=argparse.FileType('r'))
    parser.add_argument('output', nargs='?', type=argparse.FileType('w'), default=sys.stdout)
    args = parser.parse_args()

    # Read sizes data for each platform
    sizes = {}
    read_sizes(args.platform1, sizes)
    read_sizes(args.platform2, sizes)
    # Finally create tables from all collected info
    create_tables(args.config, sizes, args.output)
