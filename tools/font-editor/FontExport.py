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

#ifndef {font_namespace}_HH
#define {font_namespace}_HH

#include {include_fastarduino_font_header}

namespace {namespace}
{{
	class {font_name} : public Font<{vertical}>
	{
	public:
		font_name() : Font{{{first_char}, {last_char}, {font_width}, {font_height}, FONT}} {{}}

	private:
		static const uint8_t FONT[] PROGMEM;
	};
}}
#endif /* {font_namespace}_HH */

/// @endcond
"""

SOURCE_TEMPLATE = """{header}

#include "{font_header}"

const uint8_t {namespace}::{font_name}::FONT[] PROGMEM =
{
	{font_glyphs}
	0x78, 0x46, 0x41, 0x46, 0x78  // 0x7f DEL
};
"""

GLYPH_TEMPLATE = """
	{glyph_row},	// {glyph_code} {glyph_char}
"""
