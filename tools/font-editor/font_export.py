#!/usr/bin/python
# encoding: utf-8

#
#   Copyright 2016-2022 Jean-Francois Poilpret
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
 
# This python module is used by the font creation mini-application; it performs 
# actual C++ header and source files generation

FASTARDUINO_HEADER_TEMPLATE = """//   Copyright 2016-2022 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/// @cond api

#ifndef {font_header_define}
#define {font_header_define}

#include "../font.h"

namespace devices::display
{{
	class {font_name} : public Font<{vertical}>
	{{
	public:
		{font_name}() : Font{{0x{first_char:02x}, 0x{last_char:02x}, {font_width}, {font_height}, FONT}} {{}}

	private:
		static const uint8_t FONT[] PROGMEM;
	}};
}}
#endif /* {font_header_define} */

/// @endcond
"""

FASTARDUINO_SOURCE_TEMPLATE = """//   Copyright 2016-2022 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#include "{font_header}"

const uint8_t devices::display::{font_name}::FONT[] PROGMEM =
{{
{font_glyphs}}};
"""

REGULAR_CODE_TEMPLATE = """
#include <fastarduino/devices/font.h>

class {font_name} : public devices::display::Font<{vertical}>
{{
public:
	{font_name}() : Font{{0x{first_char:02x}, 0x{last_char:02x}, {font_width}, {font_height}, FONT}} {{}}

private:
	static const uint8_t FONT[] PROGMEM;
}};

const uint8_t {font_name}::FONT[] PROGMEM =
{{
{font_glyphs}}};
"""

#FIXME if glyph_car is \ then it shall be escaped!
GLYPH_TEMPLATE = """	{glyph_row}	// 0x{glyph_code:02x} {glyph_char}
"""

def generate_fastarduino_header(filename: str, font_name: str, width: int, height: int, 
	first_char: int, last_char: int, vertical: bool) -> str:
	# generate header as string
	return FASTARDUINO_HEADER_TEMPLATE.format(
		font_header_define = filename.upper() + '_HH',
		font_name = font_name,
		vertical = 'true' if vertical else 'false',
		first_char = first_char,
		last_char = last_char,
		font_width = width,
		font_height =  height)

def generate_glyph_rows(c: int, width: int, height: int, vertical: bool, glyph: list[list[bool]]):
	glyph_rows = ''
	if vertical:
		for row in range(int((height - 1) / 8 + 1)):
			glyph_row = ''
			for col in range(width):
				mask = 1
				byte = 0
				for i in range(8):
					if row * 8 + i == height:
						break
					# print(f"glyph[{row * 8 + i}][{col}]")
					if glyph[row * 8 + i][col]:
						byte |= mask
					mask *= 2
				glyph_row += f'0x{byte:02x}, '
			glyph_rows += GLYPH_TEMPLATE.format(glyph_row = glyph_row, glyph_code = c, glyph_char = chr(c))
	else:
		#TODO
		pass
	return glyph_rows

def generate_all_glyphs(width: int, height: int, first_char: int, last_char: int, 
	vertical: bool, glyphs: dict[int, list[list[bool]]]) -> str:
	# First generate all rows for glyphs definition
	all_glyphs = ''
	for c in range(first_char, last_char + 1):
		glyph = glyphs[c]
		all_glyphs += generate_glyph_rows(c, width, height, vertical, glyph)
	return all_glyphs

def generate_fastarduino_source(filename: str, font_name: str, width: int, height: int, 
	first_char: int, last_char: int, vertical: bool, glyphs: dict[int, list[list[bool]]]) -> str:
	# First generate all rows for glyphs definition
	all_glyphs = generate_all_glyphs(width, height, first_char, last_char, vertical, glyphs)
	return FASTARDUINO_SOURCE_TEMPLATE.format(
		font_header = filename + '.h',
		font_name = font_name,
		font_glyphs = all_glyphs)

def generate_regular_code(filename: str, font_name: str, width: int, height: int, 
	first_char: int, last_char: int, vertical: bool, glyphs: dict[int, list[list[bool]]]) -> str:
	# First generate all rows for glyphs definition
	all_glyphs = generate_all_glyphs(width, height, first_char, last_char, vertical, glyphs)
	return REGULAR_CODE_TEMPLATE.format(
		font_name = font_name,
		vertical = 'true' if vertical else 'false',
		first_char = first_char,
		last_char = last_char,
		font_width = width,
		font_height =  height,
		font_glyphs = all_glyphs)
