//   Copyright 2016-2022 Jean-Francois Poilpret
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

/**
 * @file
 */
#ifndef FONT_HH
#define FONT_HH

#include <avr/pgmspace.h>

// - Generic font support: different widths, different characters sets (range)
namespace devices::display
{
	//TODO also allow reading fonts from files/stream?
	template<bool VERTICAL_> class Font
	{
	public:
		static constexpr bool VERTICAL = VERTICAL_;

		Font(char first_char, char last_char, uint8_t width, uint8_t height, const uint8_t* glyphs)
			:	first_char_{uint8_t(first_char)}, last_char_{uint8_t(last_char)},
				width_{width}, height_{height}, 
				glyphs_{glyphs} {}

		uint8_t width() const
		{
			return width_;
		}

		uint8_t height() const
		{
			return height_;
		}

		//TODO need to support width and height of more than 8 bits!
		uint8_t glyph_size() const
		{
			if (VERTICAL)
				return width_;
			else
				return height_;
		}

		// Return pointer to glyph's 1st cahracter or nullptr if required character does not exist in font
		uint16_t get_char_glyph_ref(char value) const
		{
			const uint8_t val = uint8_t(value);
			if ((val < first_char_) || (val > last_char_))
				return 0;
			// Find first byte of character in glyphs_
			//TODO the following LOC will draw multiply maths functions!
			uint16_t index  = (val - first_char_) * glyph_size();
			return uint16_t(&glyphs_[index]);
		}

		uint8_t get_char_glyph_byte(uint16_t glyph_ref, uint8_t index) const
		{
			if (index >= glyph_size()) return 0;
			return pgm_read_byte(glyph_ref + index);
		}

	private:
		// Supported character codes
		const uint8_t first_char_;
		const uint8_t last_char_;

		// Font size
		const uint8_t width_;
		const uint8_t height_;

		// Font used in characters display
		// const uint8_t glyph_size_;
		const uint8_t* glyphs_ = nullptr;
	};
}

#endif /* FONT_HH */
/// @endcond
