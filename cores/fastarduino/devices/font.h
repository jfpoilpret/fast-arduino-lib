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

//TODO explain how glyph bytes are must be ordered in glyph array! 
namespace devices::display
{
	/**
	 * Generic font support class.
	 * 
	 * Font glyphs are either stored horizontally (1 byte represents a row) or vertically
	 * (1 byte represents a columns).
	 * Direction selection is based on display devices internal raster organisation
	 * and is thus used for optimization purposes.
	 * 
	 * @tparam VERTICAL_ `true` if font is vertical, `false` if horizontal
	 */
	template<bool VERTICAL_> class Font
	{
	public:
		/** `true` if font is vertical, `false` if horizontal */
		static constexpr bool VERTICAL = VERTICAL_;

		/**
		 * Construct a new Font.
		 * 
		 * @param first_char code of first character mapped to a glyph
		 * @param last_char code of last character mapped to a glyph
		 * @param width width of a glyph in pixels 
		 * @param height height of a glyph in pixels
		 * @param glyphs pointer to an array of bytes containing all glyphs from 
		 * @p first_char and @p last_char; this array must be stored in MCU flash
		 * memory.
		 */
		constexpr Font(char first_char, char last_char, uint8_t width, uint8_t height, const uint8_t* glyphs)
			:	first_char_{uint8_t(first_char)}, last_char_{uint8_t(last_char)},
				width_{width}, height_{height}, 
				glyph_rows_{uint8_t(VERTICAL ? (height - 1) / 8 + 1 : height)},
				glyph_cols_{uint8_t(VERTICAL ? width : (width - 1) / 8 + 1)},
				glyph_size_{uint8_t(glyph_rows_ * glyph_cols_)},
				glyphs_{glyphs} {}

		/** Width of font glyphs in pixels. */
		uint8_t width() const
		{
			return width_;
		}

		/** Height of font glyphs in pixels. */
		uint8_t height() const
		{
			return height_;
		}

		/** Code of first char supported by this Font. */
		uint8_t first_char() const
		{
			return first_char_;
		}

		/** Code of last char supported by this Font. */
		uint8_t last_char() const
		{
			return last_char_;
		}

		/**
		 * Determine the number of rows this font uses for each of its glyphs.
		 * Result depends on:
		 * - vertical or horizontal font
		 * - font height
		 * 
		 * For a vertical font, one row is already composed of 8 pixels, hence the
		 * result will be 1 for fonts which height is 8 or less.
		 * For a horizontal font, the result will be exactly the font height.
		 * 
		 * @return uint8_t the number of rows used by glyph of this Font
		 */
		uint8_t glyph_rows() const
		{
			return glyph_rows_;
		}

		/**
		 * Determine the number of columns this font uses for each of its glyphs.
		 * Result depends on:
		 * - vertical or horizontal font
		 * - font width
		 * 
		 * For a vertical font, one column is one pixel, hence the result will be
		 * exactly the font width.
		 * For a horizontal font, one column is 8 pixels (one byte), hence the
		 * result will be 1 for fonts which width is 8 or less.
		 * 
		 * @return uint8_t the number of columns used by glyph of this Font
		 */
		uint8_t glyph_cols() const
		{
			return glyph_cols_;
		}

		/** Glyph size in bytes. */
		uint8_t glyph_size() const
		{
			return glyph_size_;
		}

		/**
		 * Get a glyph reference for the requested character @p value.
		 * the char glyph ref object
		 * 
		 * @param value character code to retrieve the glyph for
		 * @return uint16_t a unique glyph reference that can be used for actual 
		 * bytes reading with `get_char_glyph_byte()`; `0` if no glyph exists for 
		 * @p value.
		 * 
		 * @sa get_char_glyph_byte()
		 */
		uint16_t get_char_glyph_ref(char value) const
		{
			const uint8_t val = uint8_t(value);
			if ((val < first_char_) || (val > last_char_))
				return 0;
			// Find first byte of character in glyphs_
			uint16_t index  = (val - first_char_) * glyph_size();
			return uint16_t(&glyphs_[index]);
		}

		/**
		 * Get one byte of character glyph.
		 * 
		 * @param glyph_ref unique glyph referenced as returned by `get_char_glyph_ref()`
		 * @param index byte index to retrieve, from `0` to `glyph_size()`
		 * @return uint8_t the proper byte (pixels row or column, according to `VERTICAL`)
		 * for @p glyph_ref
		 * 
		 * @sa get_char_glyph_ref()
		 * @sa glyph_size()
		 * @sa VERTICAL
		 */
		//TODO rather have row and col as args no?
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
		// Glyph size in bytes: how many bytes for a row, how many bytes for a column
		const uint8_t glyph_rows_;
		const uint8_t glyph_cols_;
		const uint8_t glyph_size_;

		// Font used in characters display
		// const uint8_t glyph_size_;
		const uint8_t* glyphs_ = nullptr;
	};
}

#endif /* FONT_HH */
/// @endcond
