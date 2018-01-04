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
#ifndef STREAMS_HH
#define STREAMS_HH

#include <ctype.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "flash.h"
#include "utilities.h"

//TODO better alignment with C++ iostreams? method names, behavior, missing methods...
// but keep cautious about code/data size and performance (avoid virtual)
//TODO error handling for extraction operators?
/**
 * Defines C++-like streams API, based on circular buffers for input or output.
 * Typical usage of an output "stream":
 * @code
 * using streams::ostreambuf;
 * using streams::ostream;
 * using streams::dec;
 * using streams::hex;
 * using streams::endl;
 * using streams::flush;
 * 
 * const uint8_t BUFFER_SIZE;
 * char buffer[BUFFER_SIZE];
 * ostreambuf raw_out{buffer};
 * ostream out{raw_out};
 * out << "Hello, World!\n" << flush;
 * out << hex << 123 << dec << 123 << endl;
 * @endcode
 * Note that these streams are generally created for you by higher level API,
 * such as `serial::hard::UART` and similar classes.
 * 
 * @sa serial::hard
 * @sa serial::soft
 */
namespace streams
{
	/**
	 * Output API based on a ring buffer.
	 * Provides general methods to push characters or strings to the buffer;
	 * the buffer is supposed to be consumed by another class (e.g. `serial::hard::UATX`).
	 * The API provides protected "hooks" that get notified every time new content
	 * is successfully pushed to the buffer, or when the buffer is full while new
	 * content addition is attempted.
	 * 
	 * @param buffer the original ring buffer containing all pushed content; once
	 * passed to the constructor, it should never be used directly as it will be
	 * consumed by a `containers::Queue`.
	 */
	class ostreambuf : private containers::Queue<char, char>
	{
	private:
		using QUEUE = Queue<char, char>;

	public:
		template<uint8_t SIZE> ostreambuf(char (&buffer)[SIZE]) : QUEUE{buffer}, overflow_{false}
		{
		}

		/**
		 * Wait until all buffer content has been pulled by a consumer.
		 * This method clear the count of overflows that have occurred until now.
		 */
		void flush()
		{
			overflow_ = false;
			while (items()) time::yield();
		}

		/**
		 * Append a character to the buffer.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * @param c the character to append
		 * @param call_on_put `true` if `on_put()` should be called after @p c has
		 * been appended, `false` otherwise; when directly calling this method,
		 * you should keep the default value.
		 * @sa on_put()
		 * @sa overflow()
		 */
		void put(char c, bool call_on_put = true)
		{
			if (!push(c)) overflow_ = true;
			if (call_on_put) on_put();
		}

		/**
		 * Append several characters to the buffer.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * Once all characters have been appended, `on_put()` will be called,
		 * even if an overflow has occurred.
		 * 
		 * @param content the array of characters to be appended
		 * @param size the number of characters in @p content to append
		 * @sa on_put()
		 * @sa overflow()
		 */
		void put(const char* content, size_t size)
		{
			while (size--) put(*content++, false);
			on_put();
		}

		/**
		 * Append a string to the buffer.
		 * The `'\0'` end character of @p str is not transmitted.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * Once all string content has been appended, `on_put()` will be called,
		 * even if an overflow has occurred.
		 * 
		 * @param str the '\0' ended string (standard C-string) to append
		 * @sa on_put()
		 * @sa overflow()
		 */
		void puts(const char* str)
		{
			while (*str) put(*str++, false);
			on_put();
		}

		/**
		 * Append a string, stored on flash memory, to the buffer.
		 * The `'\0'` end character of @p str is not transmitted.
		 * If the buffer is full, then `overflow()` flag will be set.
		 * Once all string content has been appended, `on_put()` will be called,
		 * even if an overflow has occurred.
		 * Example:
		 * @code
		 * output.puts(F("Hello, World!\n"));
		 * @endcode
		 * 
		 * @param str the '\0' ended string (standard C-string), stored on flash,
		 * to append
		 * @sa F()
		 * @sa on_put()
		 * @sa overflow()
		 */
		void puts(const flash::FlashStorage* str)
		{
			uint16_t address = (uint16_t) str;
			while (char value = pgm_read_byte(address++)) put(value, false);
			on_put();
		}

		/**
		 * Indicate if a buffer overflow has occurred since last time `flush()` or
		 * `reset_overflow()` was called. 
		 * @sa flush()
		 * @sa reset_overflow()
		 */
		inline bool overflow() const
		{
			return overflow_;
		}

		/**
		 * Reset the overflow flag.
		 * @sa overflow()
		 */
		inline void reset_overflow()
		{
			overflow_ = false;
		}

		/**
		 * Return the underlying queue.
		 * Normally you will not need this method.
		 */
		QUEUE& queue()
		{
			return *this;
		}
		
	protected:
		/**
		 * Callback method called when new content has been added to the buffer.
		 * This can be overridden by a subclass to trigger interrupt-driven
		 * transmission of buffer data.
		 * Default implementation does nothing.
		 */
		virtual void on_put()
		{
		}

	private:
		bool overflow_;

		friend class ios_base;
	};

	/**
	 * Input API based on a ring buffer.
	 * Provides general methods to pull characters or strings from the buffer;
	 * the buffer content is supposed to be produced by another class (e.g. 
	 * `serial::hard::UARX`).
	 * 
	 * @param buffer the original ring buffer containing all pushed content; once
	 * passed to the constructor, it should never be used directly as it will be
	 * consumed by a `containers::Queue`.
	 */
	class istreambuf : private containers::Queue<char, char>
	{
	private:
		using QUEUE = Queue<char, char>;

	public:
		/**
		 * Special value returned by `get()` when buffer is empty.
		 */
		static const int EOF = -1;

		template<uint8_t SIZE> istreambuf(char (&buffer)[SIZE]) : QUEUE{buffer}
		{
		}

		/**
		 * @return number of available characters in buffer
		 */
		int available() const
		{
			return items();
		}

		/**
		 * @return next character to be read from buffer, or `EOF` if buffer is
		 * empty
		 */
		int get()
		{
			char value;
			if (pull(value)) return value;
			return EOF;
		}

		/**
		 * Read one word from buffer, blocking until a space is read or @p max
		 * characters have been read already.
		 * @param str the character array that will receive the next word
		 * @param max the maximum number of characters that can be stored in 
		 * @p str including the ending `'\0'`
		 * @return the next word read from buffer
		 */
		char* scan(char* str, size_t max);

		/**
		 * Return the underlying queue.
		 * Normally you will not need this method.
		 */
		QUEUE& queue()
		{
			return *this;
		}
	};

	/**
	 * Wait for @p in to have at least one character in buffer and get it.
	 * @param in the `istreambuf` to read one character from
	 * @return next character to be read from @p in
	 */
	char get(istreambuf& in);

	/**
	 * Wait for @p in to have at least @p size characters in buffer and get them.
	 * @param in the `istreambuf` to read characters from
	 * @param content the character array that will receive all characters read 
	 * from @p in
	 * @param size the number of characters to read from @p in
	 * @return @p content
	 */
	char* get(istreambuf& in, char* content, size_t size);

	/**
	 * Wait for @p in to have either at least @p size characters in buffer,
	 * or to reach character value @p end, then copy read string into @p str.
	 * @param in the `istreambuf` to read characters from
	 * @param str the character array that will receive all characters read 
	 * from @p in
	 * @param max the maximum number to read from @p in
	 * @param end the character marking the end of string to read
	 * @return the number of characters read from @p in and copied into @p str
	 */
	int gets(istreambuf& in, char* str, size_t max, char end = 0);

	/**
	 * Base class for formatted streams.
	 * Allows defining base, width and precision for numbers display.
	 */
	class ios_base
	{
	public:
		/**
		 * Bitmask type to represent stream format flags.
		 * This type is used as parameter or return value by methods `flags`, `setf`
		 * and `unsetf`.
		 * The values passed an retrieved by these methods can be any valid combination
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
		 * The maximum allowed precision.
		 * @sa precision()
		 */
		static constexpr uint8_t MAX_PRECISION = 16;

	protected:
		/// @cond notdocumented
		ios_base() : flags_{skipws | dec}, width_{0}, precision_{6}, fill_{' '}
		{
		}

		static constexpr uint8_t DOUBLE_BUFFER_SIZE = MAX_PRECISION + 7 + 1;

		void init()
		{
			width_ = 0;
			precision_ = 6;
			flags_ = skipws | dec;
			fill_ = ' ';
		}
		// Conversions from string
		bool convert(const char* token, bool& b) const
		{
			if (flags() & boolalpha)
				b = (strcmp(token, "true") == 0);
			else
				b = (atol(token) != 0);
			return true;
		}
		bool convert(const char* token, double& v) const
		{
			char* endptr;
			double value = strtod(token, &endptr);
			if (endptr == token) return false;
			v = value;
			return true;
		}
		const char* binary_token(const char* token) const
		{
			if (base() == 2 && ((strncmp(token, "0b0", 3) == 0) || (strncmp(token, "0b1", 3) == 0)))
				return token + 2;
			return token;
		}
		bool convert(const char* token, long& v) const
		{
			char* endptr;
			long value = strtol(binary_token(token), &endptr, base());
			if (endptr == token) return false;
			v = value;
			return true;
		}
		bool convert(const char* token, unsigned long& v) const
		{
			char* endptr;
			unsigned long value = strtoul(binary_token(token), &endptr, base());
			if (endptr == token) return false;
			v = value;
			return true;
		}
		bool convert(const char* token, int& v) const
		{
			long value;
			if (!convert(token, value)) return false;
			v = (int) value;
			return true;
		}
		bool convert(const char* token, unsigned int& v) const
		{
			unsigned long value;
			if (!convert(token, value)) return false;
			v = (unsigned int) value;
			return true;
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
				justify(out, (b ? "true" : "false"), false, 0);
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
			if (add_sign) out.put('+', false);
			if (prefix && strlen(prefix)) out.puts(prefix);
			out.puts(input);
		}

		void output_filler(ostreambuf& out, char filler, uint8_t size) const
		{
			while (size--) out.put(filler, false);
		}

		void justify(ostreambuf& out, const char* input, bool add_sign, const char* prefix) const
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
			else
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
					out.puts(input);
					output_filler(out, fill(), add);
					out.on_put();
				}
				else
				{
					output_filler(out, fill(), add);
					out.puts(input);
				}
			}
			else
				out.puts(input);
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
		fmtflags flags_;
		uint8_t width_;
		uint8_t precision_;
		char fill_;
	};

	/**
	 * Output stream wrapper to provide formatted output API, a la C++.
	 */
	class ostream : public ios_base
	{
	public:
		/**
		 * Construct a formatted output wrapper of @p stream
		 * @param stream the output stream to be wrapped
		 */
		ostream(ostreambuf& stream) : stream_{stream}
		{
		}

		/**
		 * @copydoc ostreambuf::flush()
		 */
		void flush()
		{
			stream_.flush();
		}

		/**
		 * @copydoc ostreambuf::put(char, bool)
		 */
		void put(char c, bool call_on_put = true)
		{
			stream_.put(c, call_on_put);
		}

		/**
		 * @copydoc ostreambuf::put(const char*, size_t)
		 */
		void put(const char* content, size_t size)
		{
			stream_.put(content, size);
		}

		/**
		 * @copydoc ostreambuf::puts(const char*)
		 */
		void puts(const char* str)
		{
			stream_.puts(str);
		}

		/**
		 * @copydoc ostreambuf::puts(const flash::FlashStorage*)
		 */
		void puts(const flash::FlashStorage* str)
		{
			stream_.puts(str);
		}

		/**
		 * Output the address of a pointer.
		 * @code
		 * int i = 0;
		 * int* p = &i;
		 * out << p;
		 * @endcode
		 * @param p the pointer which address to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(const void* p)
		{
			convert(stream_, uint16_t(p));
			after_insertion();
			return *this;
		}

		/**
		 * Output a boolean value.
		 * @code
		 * out << true;
		 * @endcode
		 * @param b the bool to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(bool b)
		{
			convert(stream_, b);
			after_insertion();
			return *this;
		}

		/**
		 * Output a single character.
		 * @code
		 * out << '\n';
		 * @endcode
		 * @param c the character to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(char c)
		{
			convert(stream_, c);
			after_insertion();
			return *this;
		}

		/**
		 * Output a C-string (`\0` terminated).
		 * @code
		 * out << "Hello, Worlds!\n";
		 * @endcode
		 * @param s the string to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(const char* s)
		{
			justify(stream_, s, false, 0);
			after_insertion();
			return *this;
		}

		/**
		 * Output a C-string (`\0` terminated) that is stored in flash memory.
		 * @code
		 * out << F("Hello, Worlds!\n");
		 * @endcode
		 * @param s the string to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(const flash::FlashStorage* s)
		{
			justify(stream_, s);
			after_insertion();
			return *this;
		}

		/**
		 * Output a signed integral number, represented within the current `base()`,
		 * using the current minimum `width()`.
		 * @code
		 * int x = -123;
		 * out << x;
		 * @endcode
		 * @param v the number to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(int v)
		{
			convert(stream_, v);
			after_insertion();
			return *this;
		}

		/**
		 * Output an unsigned integral number, represented within the current `base()`,
		 * using the current minimum `width()`.
		 * @code
		 * unsigned int x = 64000;
		 * out << x;
		 * @endcode
		 * @param v the number to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(unsigned int v)
		{
			convert(stream_, v);
			after_insertion();
			return *this;
		}

		/**
		 * Output a signed long integral number, represented within the current `base()`,
		 * using the current minimum `width()`.
		 * @code
		 * long x = -999999L;
		 * out << x;
		 * @endcode
		 * @param v the number to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(long v)
		{
			convert(stream_, v);
			after_insertion();
			return *this;
		}

		/**
		 * Output an unsigned long integral number, represented within the current `base()`,
		 * using the current minimum `width()`.
		 * @code
		 * unsigned long x = 999999UL;
		 * out << x;
		 * @endcode
		 * @param v the number to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(unsigned long v)
		{
			convert(stream_, v);
			after_insertion();
			return *this;
		}

		/**
		 * Output a floating point number, using the current minimum `width()` and
		 * `precision()`.
		 * @code
		 * double x = 123.456;
		 * out << x;
		 * @endcode
		 * @param v the number to output
		 * @return @p this formatted output
		 */
		ostream& operator<<(double v)
		{
			convert(stream_, v);
			after_insertion();
			return *this;
		}

		/**
		 * General type of a manipulator function applicable to this output stream.
		 */
		using Manipulator = void (*)(ostream&);

		/**
		 * Apply a `Manipulator` to this output stream.
		 * A manipulator may:
		 * - change formatting option (base, width, precision)
		 * - call some method of this output stream 
		 * 
		 * @code
		 * using streams::hex;
		 * using streams::endl;
		 * using streams::flush;
		 * 
		 * out << hex << 16384 << endl << flush;
		 * @endcode
		 * @param f the manipulator to apply to this output stream
		 * @return @p this formatted output
		 */
		ostream& operator<<(Manipulator f)
		{
			f(*this);
			return *this;
		}

	private:
		void after_insertion()
		{
			if (flags() & unitbuf) stream_.flush();
			width(0);
		}

		ostreambuf& stream_;
	};

	/**
	 * Input stream wrapper to provide formatted input API, a la C++.
	 */
	class istream : public ios_base
	{
	public:
		/**
		 * Construct a formatted input wrapper of @p stream
		 * @param stream the input stream to be wrapped
		 */
		istream(istreambuf& stream) : stream_{stream}
		{
		}

		/**
		 * @copydoc istreambuf::available()
		 */
		int available() const
		{
			return stream_.available();
		}

		/**
		 * @copydoc istreambuf::get()
		 */
		int get()
		{
			return stream_.get();
		}

		/**
		 * Wait for this buffer to have at least @p size characters and get them.
		 * @param content the character array that will receive all characters read
		 * @param size the number of characters to read
		 * @return @p content
		 */
		char* get(char* content, size_t size)
		{
			return streams::get(stream_, content, size);
		}

		/**
		 * Wait for this buffer to have either at least @p size characters,
		 * or to reach character `\0`, then copy read string into @p str.
		 * @param str the character array that will receive all characters read 
		 * @param max the maximum number to read
		 * @return the number of characters read and copied into @p str
		 */
		int gets(char* str, size_t max)
		{
			return streams::gets(stream_, str, max);
		}

		/**
		 * Read characters from buffer into @p buf until one of these conditions happen:
		 * - a space has been encountered (not read)
		 * - `width() - 1` characters have been read
		 * An `'\0'` character is added in last position of @p buf.
		 * If `skipws()` is in action, then any white spaces read from the input
		 * will be skipped and the first non white space character will be copied first
		 * to @p buf. 
		 * @param buf the char array to be filled from the input stream; it must have
		 * a minimum size of `width()`.
		 * @return @p this formatted input
		 */
		istream& operator>>(char* buf)
		{
			if (width() > 0)
			{
				skipws_if_needed();
				stream_.scan(buf, width());
				width(0);
			}
			return *this;
		}

		/**
		 * Input and interpret next character from buffer as a boolean value.
		 * If read character is '0' then, it will be interpreted as `false`,
		 * any other value will be interpreted as `true`.
		 * If `skipws()` is in action, then any white spaces read from the input
		 * will be skipped and the first non white space character will be used 
		 * to determine the value to set to @p v.
		 * @code
		 * bool b;
		 * in >> b;
		 * @endcode
		 * @param v the boolean value read from the input stream
		 * @return @p this formatted input
		 */
		istream& operator>>(bool& v)
		{
			skipws_if_needed();
			char buffer[10 + 1];
			convert(stream_.scan(buffer, sizeof buffer), v);
			return *this;
		}

		/**
		 * Input next character from buffer.
		 * If `skipws()` is in action, then any white spaces read from the input
		 * will be skipped and the first non white space character will be used 
		 * to determine the value to set to @p v.
		 * @code
		 * char c;
		 * in >> c;
		 * @endcode
		 * @param v the next character read from the input stream
		 * @return @p this formatted input
		 */
		istream& operator>>(char& v)
		{
			skipws_if_needed();
			v = containers::pull(stream_.queue());
			return *this;
		}

		/**
		 * Input and interpret next word from buffer as a signed integer value.
		 * If `skipws()` is in action, then any white spaces read from the input
		 * will be skipped and the first non white space character will be used 
		 * to determine the value to set to @p v.
		 * @code
		 * int i;
		 * in >> i;
		 * @endcode
		 * @param v the integer value read from the input stream
		 * @return @p this formatted input
		 */
		istream& operator>>(int& v)
		{
			skipws_if_needed();
			char buffer[sizeof(int) * 8 + 1];
			convert(stream_.scan(buffer, sizeof buffer), v);
			return *this;
		}

		/**
		 * Input and interpret next word from buffer as an unsigned integer value.
		 * If `skipws()` is in action, then any white spaces read from the input
		 * will be skipped and the first non white space character will be used 
		 * to determine the value to set to @p v.
		 * @code
		 * unsigned int i;
		 * in >> i;
		 * @endcode
		 * @param v the unsigned integer value read from the input stream
		 * @return @p this formatted input
		 */
		istream& operator>>(unsigned int& v)
		{
			skipws_if_needed();
			char buffer[sizeof(int) * 8 + 1];
			convert(stream_.scan(buffer, sizeof buffer), v);
			return *this;
		}

		/**
		 * Input and interpret next word from buffer as a signed long integer value.
		 * If `skipws()` is in action, then any white spaces read from the input
		 * will be skipped and the first non white space character will be used 
		 * to determine the value to set to @p v.
		 * @code
		 * long i;
		 * in >> i;
		 * @endcode
		 * @param v the long integer value read from the input stream
		 * @return @p this formatted input
		 */
		istream& operator>>(long& v)
		{
			skipws_if_needed();
			char buffer[sizeof(long) * 8 + 1];
			convert(stream_.scan(buffer, sizeof buffer), v);
			return *this;
		}

		/**
		 * Input and interpret next word from buffer as an unsigned long integer value.
		 * If `skipws()` is in action, then any white spaces read from the input
		 * will be skipped and the first non white space character will be used 
		 * to determine the value to set to @p v.
		 * @code
		 * unsigned long i;
		 * in >> i;
		 * @endcode
		 * @param v the unsigned long integer value read from the input stream
		 * @return @p this formatted input
		 */
		istream& operator>>(unsigned long& v)
		{
			skipws_if_needed();
			char buffer[sizeof(long) * 8 + 1];
			convert(stream_.scan(buffer, sizeof buffer), v);
			return *this;
		}

		/**
		 * Input and interpret next word from buffer as a floating point value.
		 * If `skipws()` is in action, then any white spaces read from the input
		 * will be skipped and the first non white space character will be used 
		 * to determine the value to set to @p v.
		 * @code
		 * double d;
		 * in >> d;
		 * @endcode
		 * @param v the floating point value read from the input stream
		 * @return @p this formatted input
		 */
		istream& operator>>(double& v)
		{
			skipws_if_needed();
			// Allocate sufficient size for fixed/scientific representation with precision max = 16
			// Need 1 more for sign, 1 for DP, 1 for first digit, 4 for e+00
			char buffer[DOUBLE_BUFFER_SIZE];
			convert(stream_.scan(buffer, sizeof buffer), v);
			return *this;
		}

		/**
		 * General type of a manipulator function applicable to this input stream.
		 */
		using Manipulator = void (*)(istream&);

		/**
		 * Apply a `Manipulator` to this input stream.
		 * A manipulator may:
		 * - change formatting option (base)
		 * - call some method of this input stream 
		 * 
		 * @code
		 * using streams::hex;
		 * 
		 * unsigned int value;
		 * // Read next integral value from `in`; this value is expected to be 
		 * represented in hexadecimal in the input, e.g. 0xABCD.
		 * in >> hex >> value;
		 * @endcode
		 * @param f the manipulator to apply to this input stream
		 * @return @p this formatted input
		 */
		istream& operator>>(Manipulator f)
		{
			f(*this);
			return *this;
		}

	private:
		void skipws_if_needed()
		{
			if (flags() & skipws) skip_whitespace();
		}

		void skip_whitespace()
		{
			while (isspace(containers::peek(stream_.queue()))) containers::pull(stream_.queue());
		}

		istreambuf& stream_;

		template<typename FSTREAM> friend void ws(FSTREAM&);
	};

	using ios = ios_base;

	/// @cond notdocumented
	class setw_
	{
	public:
		template<typename FSTREAM> void operator() (FSTREAM& stream) const
		{
			stream.width(width_);
		}
	private:
		constexpr setw_(uint8_t width):width_{width}
		{
		}
		const uint8_t width_;
		friend constexpr const setw_ setw(uint8_t width);
	};
	template<typename FSTREAM> FSTREAM& operator<< (FSTREAM& stream, const setw_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>> (FSTREAM& stream, const setw_ f)
	{
		f(stream);
		return stream;
	}

	class setprecision_
	{
	public:
		template<typename FSTREAM> void operator() (FSTREAM& stream) const
		{
			stream.precision(precision_);
		}
	private:
		constexpr setprecision_(uint8_t precision):precision_{precision}
		{
		}
		const uint8_t precision_;
		friend constexpr const setprecision_ setprecision(uint8_t precision);
	};
	template<typename FSTREAM> FSTREAM& operator<< (FSTREAM& stream, const setprecision_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>> (FSTREAM& stream, const setprecision_ f)
	{
		f(stream);
		return stream;
	}

	class setfill_
	{
	public:
		template<typename FSTREAM> void operator() (FSTREAM& stream) const
		{
			stream.fill(fill_);
		}
	private:
		constexpr setfill_(char fill):fill_{fill}
		{
		}
		const char fill_;
		friend constexpr const setfill_ setfill(char fill);
	};
	template<typename FSTREAM> FSTREAM& operator<< (FSTREAM& stream, const setfill_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>> (FSTREAM& stream, const setfill_ f)
	{
		f(stream);
		return stream;
	}

	class setbase_
	{
	public:
		template<typename FSTREAM> void operator() (FSTREAM& stream) const
		{
			stream.setf(base_, ios::basefield);
		}
	private:
		constexpr setbase_(int b):base_{b == 2 ? ios::bin : b == 8 ? ios::oct : b == 16 ? ios::hex : ios::dec}
		{
		}
		const ios::fmtflags base_;
		friend constexpr const setbase_ setbase(int base);
	};
	template<typename FSTREAM> FSTREAM& operator<< (FSTREAM& stream, const setbase_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>> (FSTREAM& stream, const setbase_ f)
	{
		f(stream);
		return stream;
	}

	class setiosflags_
	{
	public:
		template<typename FSTREAM> void operator() (FSTREAM& stream) const
		{
			stream.setf(mask_);
		}
	private:
		constexpr setiosflags_(ios::fmtflags mask):mask_{mask}
		{
		}
		const ios::fmtflags mask_;
		friend constexpr const setiosflags_ setiosflags(ios::fmtflags mask);
	};
	template<typename FSTREAM> FSTREAM& operator<< (FSTREAM& stream, const setiosflags_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>> (FSTREAM& stream, const setiosflags_ f)
	{
		f(stream);
		return stream;
	}

	class resetiosflags_
	{
	public:
		template<typename FSTREAM> void operator() (FSTREAM& stream) const
		{
			stream.unsetf(mask_);
		}
	private:
		constexpr resetiosflags_(ios::fmtflags mask):mask_{mask}
		{
		}
		const ios::fmtflags mask_;
		friend constexpr const resetiosflags_ resetiosflags(ios::fmtflags mask);
	};
	template<typename FSTREAM> FSTREAM& operator<< (FSTREAM& stream, const resetiosflags_ f)
	{
		f(stream);
		return stream;
	}
	template<typename FSTREAM> FSTREAM& operator>> (FSTREAM& stream, const resetiosflags_ f)
	{
		f(stream);
		return stream;
	}
	/// @endcond

	/**
	 * Set the field width to be used on output (and some input) operations.
	 * This method should only be used as a stream manipulator.
	 * 
	 * The folowing example displays `123` right-aligned on 10 positions, i.e.
	 * with 7 *fill* characters prepended:
	 * @code
	 * out << setw(10) << dec << right << 123 << endl;
	 * @endcode
	 * Note that `setw()` is effective only for one input or output operation,
	 * and thus must be called before each operation.
	 */
	constexpr const setw_ setw(uint8_t width)
	{
		return setw_{width};
	}

	/**
	 * Set the decimal precision to be used to format floating-point values on 
	 * output operations.
	 * This method should only be used as a stream manipulator.
	 * 
	 * The folowing example displays `123.456789` with various precision, i.e.
	 * various number of decimals afeter the decimal point:
	 * @code
	 * out << setprecision(2) << 123.456789 << endl;
	 * out << setprecision(4) << 123.456789 << endl;
	 * out << setprecision(6) << 123.456789 << endl;
	 * out << setprecision(8) << 123.456789 << endl;
	 * @endcode
	 */
	constexpr const setprecision_ setprecision(uint8_t precision)
	{
		return setprecision_{precision};
	}

	/**
	 * Set the ios::basefield to one of its possible values (ios::dec, ios::bin, 
	 * ios::oct or ios::hex) according to @p base, which must be one of 10, 2, 8 or 16.
	 * This method should only be used as a stream manipulator.
	 * 
	 * The folowing example displays `123` under all available bases:
	 * @code
	 * out << setbase(10) << 123 << endl;
	 * out << setbase(2) << 123 << endl;
	 * out << setbase(8) << 123 << endl;
	 * out << setbase(16) << 123 << endl;
	 * @endcode
	 * 
	 * Note that it is generally preferrable to use the other manipulators.
	 * @sa dec()
	 * @sa bin()
	 * @sa oct()
	 * @sa hex()
	 */
	constexpr const setbase_ setbase(int base)
	{
		return setbase_{base};
	}

	/**
	 * Set a new *fill* character.
	 * This method should only be used as a stream manipulator.
	 * 
	 * The following example displays `123` in hexadecimal form on 4 positions,
	 * padded with `0` if necessary:
	 * @code
	 * out << setfill(`0`) << setw(4) << hex << right << 123 << endl;
	 * @endcode
	 */
	constexpr const setfill_ setfill(char fill)
	{
		return setfill_{fill};
	}

	/**
	 * Set the format flags specified by @p mask.
	 * This method should only be used as a stream manipulator.
	 * 
	 * Behaves as if `ios::setf` was called with @p mask as argument, on the 
	 * stream on which it is inserted/extracted as a manipulator (it can be 
	 * inserted/extracted on input streams or output streams).
	 * See ios::fmtflags for more information on the particular flags that can
	 * be modified with this manipulator function.
	 * @sa ios::fmtflags
	 * @sa ios::setf()
	 */
	constexpr const setiosflags_ setiosflags(ios::fmtflags mask)
	{
		return setiosflags_{mask};
	}

	/**
	 * Unset the format flags specified by @p mask.
	 * This method should only be used as a stream manipulator.
	 * 
	 * Behaves as if `ios::unsetf` was called with @p mask as argument, on the 
	 * stream on which it is inserted/extracted as a manipulator (it can be 
	 * inserted/extracted on input streams or output streams).
	 * See ios::fmtflags for more information on the particular flags that can
	 * be modified with this manipulator function.
	 * @sa ios::fmtflags
	 * @sa ios::unsetf()
	 */
	constexpr const resetiosflags_ resetiosflags(ios::fmtflags mask)
	{
		return resetiosflags_{mask};
	}

	/**
	 * Manipulator for an input stream, which will swallow all white spaces from
	 * that stream.
	 * The following sample code puts the next non white space character of `in`
	 * into `c`:
	 * @code
	 * char c;
	 * in >> ws >> c;
	 * @endcode
	 */
	template<typename FSTREAM> inline void ws(FSTREAM& stream)
	{
		stream.skip_whitespace();
	}

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
	 * Manipulator for an output stream, which will flush the stream buffer.
	 */
	template<typename FSTREAM> inline void flush(FSTREAM& stream)
	{
		stream.flush();
	}

	/**
	 * Manipulator for an output stream, which will insert a new-line character
	 * and flush the stream buffer.
	 */
	template<typename FSTREAM> inline void endl(FSTREAM& stream)
	{
		stream.put('\n');
		stream.flush();
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

#endif /* STREAMS_HH */
/// @endcond
