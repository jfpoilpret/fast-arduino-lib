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

COPYRIGHT_HEADER = """//   Copyright 2016-2022 Jean-Francois Poilpret
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
"""

HEADER_TEMPLATE = """{header}

/// @cond api

#ifndef {font_namespace}
#define {font_namespace}

#include {include_fastarduino_font_header}

namespace {namespace}
{{
	class {font_name} : public Font<{vertical}>
	{{
	public:
		{font_name}() : Font{{0x{first_char:02x}, 0x{last_char:02x}, {font_width}, {font_height}, FONT}} {{}}

	private:
		static const uint8_t FONT[] PROGMEM;
	}};
}}
#endif /* {font_namespace} */

/// @endcond
"""

SOURCE_TEMPLATE = """{header}

#include "{font_header}"

const uint8_t {namespace}::{font_name}::FONT[] PROGMEM =
{{
{font_glyphs}}};
"""

GLYPH_TEMPLATE = """	{glyph_row}	// 0x{glyph_code:02x} {glyph_char}
"""

def generate_header(filename: str, font_name: str, namespace: str, 
	width: int, height: int, first_char: int, last_char: int, 
	vertical: bool, fastarduino: bool) -> str:
	# generate header as string
	return HEADER_TEMPLATE.format(
		header = HEADER_TEMPLATE if fastarduino else '',
		font_namespace = filename.upper() + '_HH',
		include_fastarduino_font_header = 'font.h' if fastarduino else '<fastarduino/devices/font.h>',
		namespace = namespace,
		font_name = font_name,
		vertical = 'true' if vertical else 'false',
		first_char = first_char,
		last_char = last_char,
		font_width = width,
		font_height =  height)

def generate_glyph_rows(c: int, width: int, height: int, vertical: bool, glyph):
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

def generate_source(filename: str, font_name: str, namespace: str, 
	width: int, height: int, first_char: int, last_char: int, vertical: bool, 
	glyphs, fastarduino: bool) -> str:
	# First generate all rows for glyphs definition
	all_glyphs = ''
	for c in range(first_char, last_char + 1):
		glyph = glyphs[chr(c)]
		all_glyphs += generate_glyph_rows(c, width, height, vertical, glyph)
	return SOURCE_TEMPLATE.format(
		header = HEADER_TEMPLATE if fastarduino else '',
		font_header = filename + '.h',
		namespace = namespace,
		font_name = font_name,
		font_glyphs = all_glyphs)
