//   Copyright 2016-2017 Jean-Francois Poilpret
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
 * C++-like std::iostream facilities.
 */
#ifndef IOS_H
#define IOS_H

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "streambuf.h"
#include "flash.h"

namespace streams
{
	/**
	 * Base class for formatted streams.
	 * Allows defining base, width and precision for numbers display.
	 */
	class ios_base
	{
	public:
		/**
		 * Bitmask type to represent stream state flags.
		 * This type is used as parameter or return value by methods `setstate`, 
		 * `rdstate` and `clear`.
		 * 
		 * The values passed and retrieved by these methods can be any valid combination
		 * of the predefined constants:
		 *     - goodbit (no error)
		 *     - badbit
		 *     - failbit
		 *     - eofbit
		 * 
		 * @sa setstate(iostate)
		 * @sa rdstate()
		 * @sa clear(iostate)
		 */
		using iostate = uint8_t;

		/** 
		 * This bit is set if the stream has unexpected reached its end during
		 * an extraction.
		 * Note that this bit is currently never set by istreambuf, as istreambuf
		 * is blocked until characters are input.
		 */
		static constexpr iostate eofbit = 0x01;
		/** 
		 * This bit is set when an input or operation failed due to a formatting 
		 * error during extraction. 
		 */
		static constexpr iostate failbit = 0x02;
		/**
		 * This bit is set when an irrecoverable stream error has occurred, e.g.
		 * if an overflow occurs when writing to ostreambuf.
		 */
		static constexpr iostate badbit = 0x04;
		/** No error; always `0`. */
		static constexpr iostate goodbit = 0;

		/**
		 * Return the current stream error state.
		 */
		inline iostate rdstate() const
		{
			return state_;
		}

		/**
		 * Set the stream error flags state in addition to currently set flags. 
		 * Essentially calls `clear(rdstate() | state)`.
		 */
		inline void setstate(iostate state)
		{
			clear(rdstate() | state);
		}

		/**
		 * Set the stream error state flags by assigning them the value of @p state. 
		 * By default, assigns ios::goodbit which has the effect of clearing all
		 * error state flags.
		 */
		inline void clear(iostate state = goodbit)
		{
			state_ = state;
		}

		/**
		 * @retval `true` if the most recent I/O operation on the stream completed 
		 * successfully. 
		 * @retval `false` if any I/O operation has failed since last call to
		 * `ios::clear()`.
		 * @sa rdstate()
		 * @sa goodbit
		 * @sa clear()
		 */
		inline bool good() const
		{
			return rdstate() == goodbit;
		}

		/**
		 * Return `true` if the associated stream has reached end-of-file. 
		 * Specifically, returns `true` if `ios::eofbit` is set in `rdstate()`.
		 * @sa rdstate()
		 * @sa eofbit
		 */
		inline bool eof() const
		{
			return rdstate() & eofbit;
		}

		/**
		 * Return `true` if an error has occurred on the associated stream,
		 * since last time state was reset (`clear()` was called). 
		 * Specifically, returns `true` if `ios::badbit` or `ios::failbit` is set
		 * in `rdstate()`.
		 * @sa rdstate()
		 * @sa failbit
		 * @sa badbit
		 * @sa clear()
		 */
		inline bool fail() const
		{
			return rdstate() & (failbit | badbit);
		}

		/**
		 * Return `true` if a non-recoverable error has occurred on the associated
		 * stream. 
		 * Specifically, returns `true` if `ios::badbit` is set in `rdstate()`.
		 * @sa rdstate()
		 * @sa badbit
		 */
		inline bool bad() const
		{
			return rdstate() & badbit;
		}

		/**
		 * Return `true` if an error has occurred on the associated stream,
		 * since last time state was reset (`clear()` was called). 
		 * Actually, this is equivalent to calling `fail()`.
		 * @sa fail()
		 */
		inline bool operator!() const
		{
			return fail();
		}

		/**
		 * Check that the current stream has no errors.
		 * Returns `true` if the stream has no errors and is ready for I/O operations.
		 * @sa fail()
		 */
		inline explicit operator bool() const
		{
			return !fail();
		}

		/**
		 * Bitmask type to represent stream format flags.
		 * This type is used as parameter or return value by methods `flags`, `setf`
		 * and `unsetf`.
		 * The values passed and retrieved by these methods can be any valid combination
		 * of the predefined constants:
		 * - basefield flags
		 *     - dec (default)
		 *     - bin
		 *     - oct
		 *     - hex
		 * - floatfield flags
		 *     - fixed (default)
		 *     - scientific
		 * - adjustfield flags
		 *     - left
		 *     - right (default)
		 * - independent flags
		 *     - boolalpha
		 *     - showbase
		 *     - showpos
		 *     - skipws (default)
		 *     - unitbuf
		 *     - uppercase
		 * 
		 * @sa flags()
		 * @sa flags(fmtflags)
		 * @sa setf(fmtflags)
		 * @sa setf(fmtflags, fmtflags)
		 * @sa unsetf(fmtflags)
		 */
		using fmtflags = uint16_t;

		/** Read or write integral values using decimal (0..9) base format. */
		static constexpr fmtflags dec = 0x0001;
		/** Read or write integral values using binary (0,1) base format. */
		static constexpr fmtflags bin = 0x0002;
		/** Read or write integral values using octal (0..7) base format. */
		static constexpr fmtflags oct = 0x0004;
		/** Read or write integral values using hexadecimal (0..9,A..F) base format. */
		static constexpr fmtflags hex = 0x0008;
		/**
		 * Bitmask constant used with `setf(fmtflags, fmtflags)` when changing the
		 * output base format.
		 * @sa dec
		 * @sa bin
		 * @sa oct
		 * @sa hex
		 */
		static constexpr fmtflags basefield = dec | bin | oct | hex;

		/** 
		 * Pad all output to `width()` characters, with `fill()` character appended 
		 * at the end so that the output appears left-adjusted.
		 */
		static constexpr fmtflags left = 0x0010;
		/** 
		 * Pad all output to `width()` characters, with `fill()` character added 
		 * at the beginning so that the output appears right-adjusted.
		 */
		static constexpr fmtflags right = 0x0020;
		/**
		 * Bitmask constant used with `setf(fmtflags, fmtflags)` when changing the
		 * output adjustment.
		 * @sa left
		 * @sa right
		 */
		static constexpr fmtflags adjustfield = left | right;

		/** Write floating point values in fixed-point notation. */
		static constexpr fmtflags scientific = 0x0040;
		/** Write floating point values in scientific notation. */
		static constexpr fmtflags fixed = 0x0080;
		/**
		 * Bitmask constant used with `setf(fmtflags, fmtflags)` when changing the
		 * floating point output representation.
		 * @sa fixed
		 * @sa scientific
		 */
		static constexpr fmtflags floatfield = scientific | fixed;
		
		/** Read or write bool values as alphabetic string (`true` or `false`). */
		static constexpr fmtflags boolalpha = 0x0200;
		/** 
		 * Write integral values prefixed by their base:
		 * - decimal: no prefix
		 * - binary: `0b` prefix
		 * - octal: `0` prefix
		 * - hexadecimal: `0x` prefix
		 */
		static constexpr fmtflags showbase = 0x0400;
		/** Write non-negative numerical values preceded by `+`. */
		static constexpr fmtflags showpos = 0x1000;
		/** Skip leading spaces on certain extraction (read) operations. */
		static constexpr fmtflags skipws = 0x2000;
		/** Flush output after each insertion oepration. */
		static constexpr fmtflags unitbuf = 0x4000;
		/** 
		 * Write uppercase letters instead of lowercase in certain insertion operations. 
		 * This applies to hexadecimal letters when writing integral numbers and exponent
		 * letter when writing floating point numbers. Base prefixes (`0x` or `0b`) are
		 * not affected.
		 */
		static constexpr fmtflags uppercase = 0x8000;

		/**
		 * Set new format flags for this stream.
		 * @sa fmtflags
		 */
		inline void flags(fmtflags flags)
		{
			flags_ = flags;
		}

		/**
		 * Return the format flags currently selected in this stream.
		 * @sa fmtflags
		 */
		inline fmtflags flags() const
		{
			return flags_;
		}

		/**
		 * Set this stream's format flags whose bits are set in @p flags, leaving 
		 * unchanged the rest.
		 * This is equivalent to `flags(flags | flags());`.
		 * 
		 * This method is used to set independent flags.
		 * 
		 * @sa fmtflags
		 * @sa setf(fmtflags, fmtflags)
		 * @sa flags(fmtflags)
		 */
		inline void setf(fmtflags flags)
		{
			flags_ |= flags;
		}

		/**
		 * Set this stream's format flags whose bits are set in both @p flags and @p mask,
		 * and clears the format flags whose bits are set in @p mask but not in @p flags.
		 * This is equivalent to `flags((flags & mask) | (flags() & ~mask));`.
		 * 
		 * This method is used to set a value for one of a group of related flags,
		 * using one of the field bitmasks as the @p mask argument:
		 * 
		 * flags              | mask
		 * -------------------|--------------
		 * left, right        | adjustfield
		 * dec, bin, oct, hex | basefield
		 * scientific, fixed  | floatfield
		 * 
		 * @sa fmtflags
		 * @sa setf(fmtflags)
		 * @sa flags(fmtflags)
		 */
		inline void setf(fmtflags flags, fmtflags mask)
		{
			flags_ = (flags_ & ~mask) | (flags & mask);
		}

		/**
		 * Clear this stream's format flags whose bits are set in @p flags.
		 * @sa fmtflags
		 * @sa setf(fmtflags)
		 * @sa flags(fmtflags)
		 */
		inline void unsetf(fmtflags flags)
		{
			flags_ &= ~flags;
		}

		/**
		 * Return the *fill* character.
		 * The fill character is the character used by output insertion functions to 
		 * fill spaces when padding results to the field width.
		 * Default fill character is a space.
		 * @sa width()
		 */
		inline char fill() const
		{
			return fill_;
		}

		/**
		 * Set @p fill as new *fill* character for this stream.
		 * The fill character is the character used by output insertion functions to 
		 * fill spaces when padding results to the field width.
		 * @sa width()
		 */
		inline void fill(char fill)
		{
			fill_ = fill;
		}

		/**
		 * Set minimum width used for displaying values. If a value's 
		 * representation needs less than @p width characters for display, then
		 * additional characters will be added before or after the value.
		 * The filler character is determined by `fill()`, a space by default.
		 * Padding side is determined by `flags()`: `left` or `right`.
		 * Note that if the value representation needs more than @p width 
		 * characters, then @p width will have no effect on display.
		 * 
		 * @param width the new width to use for next formatted output
		 * 
		 * @sa fill(char)
		 * @sa flags(fmtflags)
		 * @sa left
		 * @sa right
		 */
		inline void width(uint8_t width)
		{
			width_ = width;
		}

		/**
		 * Get the current minimum width value (default = `0`) used for formatted 
		 * output.
		 * @return current minimum width 
		 */
		inline uint8_t width() const
		{
			return width_;
		}

		/**
		 * Set precision (number of digits after decimal point) used for 
		 * displaying floating values. 
		 * 
		 * @param precision the new precision for next formatted output
		 */
		inline void precision(uint8_t precision)
		{
			precision_ = (precision < MAX_PRECISION ? precision : MAX_PRECISION);
		}

		/**
		 * Get the current precision (default = `6`) used for formatted 
		 * floating values output.
		 * @return current precision
		 */
		inline uint8_t precision() const
		{
			return precision_;
		}

		/**
		 * Copy formatting information from @p rhs to `this` stream.
		 * Formatting information is:
		 * - flags()
		 * - width()
		 * - precision()
		 * - fill()
		 */
		ios_base& copyfmt(const ios_base& rhs)
		{
			flags_ = rhs.flags_;
			width_ = rhs.width_;
			precision_ = rhs.precision_;
			fill_ = rhs.fill_;
			return *this;
		}

		/**
		 * The maximum allowed precision.
		 * @sa precision()
		 */
		static constexpr uint8_t MAX_PRECISION = 16;

	protected:
		/// @cond notdocumented
		ios_base() : state_{0}, flags_{skipws | dec}, width_{0}, precision_{6}, fill_{' '}
		{
		}

		//TODO This make compilation failures...
		// ios_base(const ios_base&) = delete;
		ios_base& operator=(const ios_base&) = delete;

		static constexpr uint8_t DOUBLE_BUFFER_SIZE = MAX_PRECISION + 7 + 1;

		void init()
		{
			width_ = 0;
			precision_ = 6;
			flags_ = skipws | dec;
			fill_ = ' ';
		}
		// Conversions from string
		void convert(const char* token, bool& b)
		{
			if (flags() & boolalpha)
				b = (strcmp(token, "true") == 0);
			else
				b = (atol(token) != 0);
		}
		void convert(const char* token, double& v)
		{
			char* endptr;
			double value = strtod(token, &endptr);
			if (endptr != token)
				v = value;
			else
				setstate(failbit);
		}
		const char* binary_token(const char* token) const
		{
			if (base() == 2 && ((strncmp(token, "0b0", 3) == 0) || (strncmp(token, "0b1", 3) == 0)))
				return token + 2;
			return token;
		}
		bool convert(const char* token, long& v)
		{
			char* endptr;
			long value = strtol(binary_token(token), &endptr, base());
			if (endptr != token)
				v = value;
			else
				setstate(failbit);
			return (endptr != token);
		}
		bool convert(const char* token, unsigned long& v)
		{
			char* endptr;
			unsigned long value = strtoul(binary_token(token), &endptr, base());
			if (endptr != token)
				v = value;
			else
				setstate(failbit);
			return (endptr != token);
		}
		void convert(const char* token, int& v)
		{
			long value;
			if (convert(token, value))
				v = (int) value;
		}
		void convert(const char* token, unsigned int& v)
		{
			unsigned long value;
			if (convert(token, value))
				v = (unsigned int) value;
		}

		// Conversions to string
		void convert(ostreambuf& out, int v) const
		{
			// Allocate sufficient size for bin representation
			char buffer[sizeof(int) * 8 + 1];
			format_number(out, itoa(v, buffer, base()));
		}
		void convert(ostreambuf& out, unsigned int v) const
		{
			// Allocate sufficient size for bin representation
			char buffer[sizeof(unsigned int) * 8 + 1];
			format_number(out, utoa(v, buffer, base()));
		}
		void convert(ostreambuf& out, long v) const
		{
			// Allocate sufficient size for bin representation
			char buffer[sizeof(long) * 8 + 1];
			format_number(out, ltoa(v, buffer, base()));
		}
		void convert(ostreambuf& out, unsigned long v) const
		{
			// Allocate sufficient size for bin representation
			char buffer[sizeof(unsigned long) * 8 + 1];
			format_number(out, ultoa(v, buffer, base()));
		}
		static size_t double_digits(double v)
		{
			int digits = int(log10(fabs(v)));
			if (digits < 0)
				return 1;
			else
				return size_t(digits + 1);
		}
		bool is_too_large(double v) const
		{
			// Number of chars = sign + digits before DP + DP + precision + \0
			return (1 + double_digits(v) + 1 + precision() + 1 > DOUBLE_BUFFER_SIZE);
		}
		void convert(ostreambuf& out, double v) const
		{
			// Allocate sufficient size for fixed/scientific representation with precision max = 16
			// Need 1 more for sign, 1 for DP, 1 for first digit, 4 for e+00
			char buffer[DOUBLE_BUFFER_SIZE];
			// If v is too large, force scientific anyway
			if ((flags() & scientific) || is_too_large(v))
			{
				//FIXME if precision() > 7, then it is limited to 7 by dtostre(), add 0 manually then
				dtostre(v, buffer, precision(), 0);
			}
			else if (flags() & fixed)
				dtostrf(v, 0, precision(), buffer);
			else
				// default is fixed currently (no satisfying conversion function exists)
				dtostrf(v, 0, precision(), buffer);
			justify(out, buffer, add_sign(buffer), 0);
		}
		void convert(ostreambuf& out, char c) const
		{
			char buffer[1 + 1];
			buffer[0] = c;
			buffer[1] = 0;
			justify(out, buffer, false, 0);
		}
		void convert(ostreambuf& out, bool b) const
		{
			if (flags() & boolalpha)
				justify(out, (b ? F("true") : F("false")));
				// justify(out, (b ? "true" : "false"), false, 0);
			else
				convert(out, (b ? 1 : 0));
		}

		void upper(char* input) const
		{
			if ((flags() & uppercase) && (flags() & hex))
				strupr(input);
		}

		const char* prefix_base() const
		{
			if ((flags() & showbase) && (flags() & (bin | oct | hex)))
				return (flags() & bin) ? "0b" : (flags() & oct) ? "0" : "0x";
			return 0;
		}

		bool add_sign(const char* input, bool is_float = false) const
		{
			return ((flags() & dec) || is_float) && (flags() & showpos) && (input[0] != '+') && (input[0] != '-');
		}

		void format_number(ostreambuf& out, char* input) const
		{
			upper(input);
			justify(out, input, add_sign(input), prefix_base());
		}

		void output_number(ostreambuf& out, const char* input, bool add_sign, const char* prefix) const
		{
			if (add_sign) out.put_('+', false);
			if (prefix) out.sputn(prefix);
			out.sputn(input);
		}

		void output_filler(ostreambuf& out, char filler, uint8_t size) const
		{
			while (size--) out.put_(filler, false);
		}

		void justify(ostreambuf& out, const char* input, bool add_sign, const char* prefix) const
		{
			// Handle case where padding must be added
			// Speed optimization: handle padding situation ONLY if width is != 0
			if (width())
			{
				size_t len = strlen(input) + (prefix ? strlen(prefix) : 0) + (add_sign ? 1 : 0);
				if (len < width())
				{
					uint8_t add = width() - len;
					if (flags() & left)
					{
						output_number(out, input, add_sign, prefix);
						output_filler(out, fill(), add);
						out.on_put();
					}
					else
					{
						output_filler(out, fill(), add);
						output_number(out, input, add_sign, prefix);
					}
				}
				return;
			}
			// Handle case where no padding is needed
			output_number(out, input, add_sign, prefix);
		}

		void justify(ostreambuf& out, const flash::FlashStorage* input) const
		{
			size_t len = strlen_P((const char*) input);
			if (len < width())
			{
				uint8_t add = width() - len;
				if (flags() & left)
				{
					out.sputn(input);
					output_filler(out, fill(), add);
					out.on_put();
				}
				else
				{
					output_filler(out, fill(), add);
					out.sputn(input);
				}
			}
			else
				out.sputn(input);
		}

		int base() const
		{
			if (flags() & bin) return 2;
			if (flags() & oct) return 8;
			if (flags() & hex) return 16;
			return 10;
		}
		/// @endcond

	private:
		iostate state_;
		fmtflags flags_;
		uint8_t width_;
		uint8_t precision_;
		char fill_;
	};

	using ios = ios_base;

	/**
	 * Manipulator for an input stream, which will activate whitespace discarding
	 * before formatted input operations on that stream .
	 */
	template<typename FSTREAM> inline void skipws(FSTREAM& stream)
	{
		stream.setf(ios::skipws);
	}

	/**
	 * Manipulator for an input stream, which will deactivate whitespace discarding
	 * before formatted input operations on that stream .
	 */
	template<typename FSTREAM> inline void noskipws(FSTREAM& stream)
	{
		stream.unsetf(ios::skipws);
	}

	/**
	 * Manipulator for an output or input stream, which will set the base, used to
	 * represent (output) or interpret (input) integral	numerical values, to binary.
	 */
	template<typename FSTREAM> inline void bin(FSTREAM& stream)
	{
		stream.setf(ios::bin, ios::basefield);
	}

	/**
	 * Manipulator for an output or input stream, which will set the base, used to
	 * represent (output) or interpret (input) integral	numerical values, to octal.
	 */
	template<typename FSTREAM> inline void oct(FSTREAM& stream)
	{
		stream.setf(ios::oct, ios::basefield);
	}

	/**
	 * Manipulator for an output or input stream, which will set the base, used to
	 * represent (output) or interpret (input) integral	numerical values, to decimal.
	 */
	template<typename FSTREAM> inline void dec(FSTREAM& stream)
	{
		stream.setf(ios::dec, ios::basefield);
	}

	/**
	 * Manipulator for an output or input stream, which will set the base, used to
	 * represent (output) or interpret (input) integral	numerical values, to hexadecimal.
	 */
	template<typename FSTREAM> inline void hex(FSTREAM& stream)
	{
		stream.setf(ios::hex, ios::basefield);
	}

	/**
	 * Set the ios::boolalpha format flag for @p stream.
	 * When the boolalpha format flag is set, bool values are inserted/extracted
	 * by their textual representation: either `true` or `false`, instead of 
	 * integral values.
	 * 
	 * This flag can be unset with the `noboolalpha` manipulator.
	 * @sa noboolalpha
	 */
	template<typename FSTREAM> inline void boolalpha(FSTREAM& stream)
	{
		stream.setf(ios::boolalpha);
	}

	/**
	 * Clear the ios::boolalpha format flag for @p stream.
	 * When the boolalpha format flag is not set, bool values are inserted/extracted
	 * as integral values (`0` and `1`) instead of their textual representations:
	 * `true` or `false`.
	 * This flag can be set with the `boolalpha` manipulator.
	 * @sa boolalpha
	 */
	template<typename FSTREAM> inline void noboolalpha(FSTREAM& stream)
	{
		stream.unsetf(ios::boolalpha);
	}
	
	/**
	 * Set the ios::showbase format flag for @p stream.
	 * When the showbase format flag is set, numerical integer values inserted 
	 * into output streams are prefixed with the same prefixes used by C++ 
	 * literal constants: `0x` for hexadecimal values (see `hex`), `0` for octal 
	 * values (see `oct`), `0b` for binary values (see `bin`) and no prefix for
	 * decimal-base values (see `dec`).
	 * 
	 * This option can be unset with the `noshowbase` manipulator. 
	 * @sa noshowbase
	 * @sa dec
	 * @sa hex
	 * @sa oct
	 * @sa bin
	 */
	template<typename FSTREAM> inline void showbase(FSTREAM& stream)
	{
		stream.setf(ios::showbase);
	}
	
	/**
	 * Clear the ios::showbase format flag for @p stream.
	 * When the showbase format flag is not set, numerical integer values are inserted 
	 * into @p stream without prefixing them with any numerical base prefix (i.e.
	 * `0x` for hexadecimal values, `0` for octal values, `0b` for binary values 
	 * and no prefix for decimal-base values).
	 * 
	 * This option can be set with the `showbase` manipulator.
	 * @sa showbase
	 */
	template<typename FSTREAM> inline void noshowbase(FSTREAM& stream)
	{
		stream.unsetf(ios::showbase);
	}
	
	/**
	 * Set the ios::showpos format flag for @p stream.
	 * When the showpos format flag is set, a plus sign (+) precedes every 
	 * non-negative numerical value inserted into @p stream (including zeros).
	 * 
	 * This flag can be unset with the `noshowpos` manipulator.
	 * @sa noshowpos
	 */
	template<typename FSTREAM> inline void showpos(FSTREAM& stream)
	{
		stream.setf(ios::showpos);
	}
	
	/**
	 * Clear the ios::showpos format flag for @p stream.
	 * When the showpos format flag is not set, no plus signs precede positive
	 * values inserted into @p stream.
	 * 
	 * This flag can be set with the `noshowpos` manipulator.
	 * @sa showpos
	 */
	template<typename FSTREAM> inline void noshowpos(FSTREAM& stream)
	{
		stream.unsetf(ios::showpos);
	}
	
	/**
	 * Set the ios::uppercase format flag for @p stream.
	 * When the uppercase format flag is set, uppercase (capital) letters are used 
	 * instead of lowercase for representations on output operations involving 
	 * stream-generated letters, like hexadecimal representations.
	 * 
	 * This flag can be unset with the `nouppercase` manipulator, not forcing the
	 * use of uppercase for generated letters.
	 * @sa nouppercase
	 */
	template<typename FSTREAM> inline void uppercase(FSTREAM& stream)
	{
		stream.setf(ios::uppercase);
	}
	
	/**
	 * Clear the ios::uppercase format flag for @p stream.
	 * When the uppercase format flag is notset, the letters automatically generated
	 * by @p stream for certain representations (like hexadecimal representations)
	 * are displayed as lwoercase.
	 * 
	 * This flag can be set with the `uppercase` manipulator, forcing the
	 * use of uppercase for generated letters.
	 * @sa uppercase
	 */
	template<typename FSTREAM> inline void nouppercase(FSTREAM& stream)
	{
		stream.unsetf(ios::uppercase);
	}
	
	/**
	 * Set the ios::unitbuf format flag for @p stream.
	 * When the unitbuf flag is set, the associated buffer is flushed after each
	 * insertion operation.
	 * 
	 * This flag can be unset with the `nounitbuf` manipulator, not forcing flushes
	 * after every insertion.
	 * @sa nounitbuf
	 */
	template<typename FSTREAM> inline void unitbuf(FSTREAM& stream)
	{
		stream.setf(ios::unitbuf);
	}
	
	/**
	 * Clear the ios::unitbuf format flag for @p stream.
	 * When the unitbuf flag is not set, the associated buffer is not forced to
	 * be flushed after every insertion operation.
	 * With this mode, you can force buffer flush any time by using `flush` manipulator,
	 * or using `endl` which will output a new line follwoed by a buffer flush.
	 * 
	 * This flag can be set with the `unitbuf` manipulator, forcing flushes
	 * after every insertion.
	 * @sa unitbuf
	 */
	template<typename FSTREAM> inline void nounitbuf(FSTREAM& stream)
	{
		stream.unsetf(ios::unitbuf);
	}
	
	/**
	 * Set the ios::adjustfield format flag for @p stream to ios::left, thus
	 * adjusting next output to the left.
	 * When adjustfield is set to left, the output is padded to the field width 
	 * (`ios::width()`) by inserting fill characters (`ios::fill()`) at the end, 
	 * effectively adjusting the field to the left.
	 * @sa right
	 */
	template<typename FSTREAM> inline void left(FSTREAM& stream)
	{
		stream.setf(ios::left, ios::adjustfield);
	}
	
	/**
	 * Set the ios::adjustfield format flag for @p stream to ios::right, thus
	 * adjusting next output to the right.
	 * When adjustfield is set to right, the output is padded to the field width 
	 * (`ios::width()`) by inserting fill characters (`ios::fill()`) at the beginning, 
	 * effectively adjusting the field to the right.
	 * @sa left
	 */
	template<typename FSTREAM> inline void right(FSTREAM& stream)
	{
		stream.setf(ios::right, ios::adjustfield);
	}
	
	/**
	 * Set the ios::floatfield format flag for @p stream to ios::defaultfloat.
	 * When floatfield is set to defaultfloat, floating-point values are written 
	 * using the default notation: the representation uses as many meaningful digits 
	 * as needed up to the stream's decimal precision (`ios::precision()`), 
	 * counting both the digits before and after the decimal point (if any).
	 * 
	 * NOTE: current implementation behaves the same as `fixed`.
	 * @sa fixed
	 * @sa scientific
	 * @sa ios::precision()
	 * @sa ios::precision(uint8_t)
	 */
	template<typename FSTREAM> inline void defaultfloat(FSTREAM& stream)
	{
		stream.unsetf(ios::floatfield);
	}
	
	/**
	 * Set the ios::floatfield format flag for @p stream to ios::fixed.
	 * When floatfield is set to fixed, floating-point values are written using 
	 * fixed-point notation: the value is represented with exactly as many digits 
	 * in the decimal part as specified by the precision field (`ios::precision()`)
	 * and with no exponent part.
	 * @sa defaultfloat
	 * @sa scientific
	 * @sa ios::precision()
	 * @sa ios::precision(uint8_t)
	 */
	template<typename FSTREAM> inline void fixed(FSTREAM& stream)
	{
		stream.setf(ios::fixed, ios::floatfield);
	}
	
	/**
	 * Set the ios::floatfield format flag for @p stream to ios::scientific.
	 * When floatfield is set to scientific, floating-point values are written 
	 * using scientific notation: the value is represented always with only one 
	 * digit before the decimal point, followed by the decimal point and as many 
	 * decimal digits as the precision field (`ios::precision()`). Finally, this 
	 * notation always includes an exponential part consisting on the letter `e` 
	 * followed by an optional sign and two exponential digits.
	 * @sa defaultfloat
	 * @sa fixed
	 * @sa ios::precision()
	 * @sa ios::precision(uint8_t)
	 */
	template<typename FSTREAM> inline void scientific(FSTREAM& stream)
	{
		stream.setf(ios::scientific, ios::floatfield);
	}
}

#endif /* IOS_H */
/// @endcond
