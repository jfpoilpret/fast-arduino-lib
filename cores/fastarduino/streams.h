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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "queue.h"
#include "flash.h"
#include "utilities.h"

//TODO better alignment with C++ iostreams? class names, method names, behvior, missing methods...
//TODO improve data size by removing conversion_buffer from ios_base fields.
/**
 * Defines C++-like streams API, based on circular buffers for input or output.
 * Typical usage of an output "stream":
 * @code
 * using streams::OutputBuffer;
 * using streams::FormattedOutput;
 * using streams::dec;
 * using streams::hex;
 * using streams::endl;
 * using streams::flush;
 * 
 * const uint8_t BUFFER_SIZE;
 * char buffer[BUFFER_SIZE];
 * OutputBuffer raw_out{buffer};
 * FormattedOutput<OutputBuffer> out{raw_out};
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
	class OutputBuffer : private containers::Queue<char, char>
	{
	private:
		using QUEUE = Queue<char, char>;

	public:
		template<uint8_t SIZE> OutputBuffer(char (&buffer)[SIZE]) : QUEUE{buffer}, overflow_{false}
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
	class InputBuffer : private containers::Queue<char, char>
	{
	private:
		using QUEUE = Queue<char, char>;

	public:
		/**
		 * Special value returned by `get()` when buffer is empty.
		 */
		static const int EOF = -1;

		template<uint8_t SIZE> InputBuffer(char (&buffer)[SIZE]) : QUEUE{buffer}
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
	 * @param in the `InputBuffer` to read one character from
	 * @return next character to be read from @p in
	 */
	char get(InputBuffer& in);

	/**
	 * Wait for @p in to have at least @p size characters in buffer and get them.
	 * @param in the `InputBuffer` to read characters from
	 * @param content the character array that will receive all characters read 
	 * from @p in
	 * @param size the number of characters to read from @p in
	 * @return @p content
	 */
	char* get(InputBuffer& in, char* content, size_t size);

	/**
	 * Wait for @p in to have either at least @p size characters in buffer,
	 * or to reach character value @p end, then copy read string into @p str.
	 * @param in the `InputBuffer` to read characters from
	 * @param str the character array that will receive all characters read 
	 * from @p in
	 * @param max the maximum number to read from @p in
	 * @param end the character marking the end of string to read
	 * @return the number of characters read from @p in and copied into @p str
	 */
	int gets(InputBuffer& in, char* str, size_t max, char end = 0);

	//TODO infer on a way to define callbacks to stream put/flush/other for output
	//TODO same for input LATER
	/**
	 * Base class for formatted streams.
	 * Allows defining base, width and precision for numbers display.
	 */
	class ios_base
	{
	public:
		//TODO DOC refer to C++11 std::fmtflags (note some flags are not supported)
		using fmtflags = uint16_t;

		//TODO handle for input also!
		static constexpr fmtflags dec = 0x0001;
		static constexpr fmtflags bin = 0x0002;
		static constexpr fmtflags oct = 0x0004;
		static constexpr fmtflags hex = 0x0008;
		static constexpr fmtflags basefield = dec | bin | oct | hex;

		static constexpr fmtflags left = 0x0010;
		static constexpr fmtflags right = 0x0020;
		static constexpr fmtflags adjustfield = left | right;

		static constexpr fmtflags scientific = 0x0040;
		static constexpr fmtflags fixed = 0x0080;
		static constexpr fmtflags floatfield = scientific | fixed;
		
		static constexpr fmtflags boolalpha = 0x0200;
		static constexpr fmtflags showbase = 0x0400;
		static constexpr fmtflags showpos = 0x1000;
		static constexpr fmtflags skipws = 0x2000;
		static constexpr fmtflags unitbuf = 0x4000;
		static constexpr fmtflags uppercase = 0x8000;

		ios_base() : flags_{skipws | dec}, width_{0}, precision_{6}, fill_{' '}
		{
		}

		//TODO DOCs
		inline void flags(fmtflags flags)
		{
			flags_ = flags;
		}

		inline fmtflags flags() const
		{
			return flags_;
		}

		inline void setf(fmtflags flags)
		{
			flags_ |= flags;
		}

		inline void setf(fmtflags flags, fmtflags mask)
		{
			flags_ = (flags_ & ~mask) | (flags & mask);
		}

		inline void unsetf(fmtflags flags)
		{
			flags_ &= ~flags;
		}

		inline char fill() const
		{
			return fill_;
		}

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
			precision_ = precision;
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

	protected:
		/// @cond notdocumented
		void init()
		{
			width_ = 0;
			precision_ = 6;
			flags_ = skipws | dec;
			fill_ = ' ';
		}
		// conversions from string to numeric value
		bool convert(const char* token, bool& b)
		{
			if (flags() & boolalpha)
				b = (strcmp(token, "true") != 0);
			else
				b = (atol(token) != 0);
			return true;
		}
		bool convert(const char* token, double& v)
		{
			char* endptr;
			double value = strtod(token, &endptr);
			if (endptr == token) return false;
			v = value;
			return true;
		}
		bool convert(const char* token, long& v)
		{
			char* endptr;
			long value = strtol(token, &endptr, base());
			if (endptr == token) return false;
			v = value;
			return true;
		}
		bool convert(const char* token, unsigned long& v)
		{
			char* endptr;
			unsigned long value = strtoul(token, &endptr, base());
			if (endptr == token) return false;
			v = value;
			return true;
		}
		bool convert(const char* token, int& v)
		{
			long value;
			if (!convert(token, value)) return false;
			v = (int) value;
			return true;
		}
		bool convert(const char* token, unsigned int& v)
		{
			unsigned long value;
			if (!convert(token, value)) return false;
			v = (unsigned int) value;
			return true;
		}

		// conversions from numeric value to string
		const char* convert(int v)
		{
			return format_number(itoa(v, conversion_buffer, base()));
		}
		const char* convert(unsigned int v)
		{
			return format_number(utoa(v, conversion_buffer, base()));
		}
		const char* convert(long v)
		{
			return format_number(ltoa(v, conversion_buffer, base()));
		}
		const char* convert(unsigned long v)
		{
			return format_number(ultoa(v, conversion_buffer, base()));
		}
		const char* convert(double v)
		{
			char* buffer;
			if (flags() & fixed)
				buffer = dtostrf(v, 0, precision(), conversion_buffer);
			else if (flags() & scientific)
				buffer = dtostre(v, conversion_buffer, precision(), 0);
			else
				buffer = dtostrf(v, 0, 0, conversion_buffer);
			return justify(add_sign(buffer));
		}
		const char* convert(bool b)
		{
			if (flags() & boolalpha)
				return justify(strcpy(conversion_buffer, b ? "true" : "false"));
			else
				return convert(b ? 1 : 0);
		}

		//TODO avoid memmove as much as possible.
		char* upper(char* input) const
		{
			if ((flags() & uppercase) && (flags() & hex))
				strupr(input);
			return input;
		}
		char* add_base(char* input) const
		{
			if ((flags() & showbase) && (flags() & (bin | oct | hex)))
			{
				const char* prefix = (flags() & bin) ? "0b" : (flags() & oct) ? "0" : "0x";
				memmove(input + strlen(prefix), input, strlen(input) + 1);
				strncpy(input, prefix, strlen(prefix));
			}
			return input;
		}
		char* add_sign(char* input) const
		{
			if ((flags() & dec) && (flags() & showpos) && (input[0] != '+') && (input[0] != '-'))
			{
				// Add + sign if not exist
				memmove(input + 1, input, strlen(input) + 1);
				input[0] = '+';
			}
			return input;
		}

		char* format_number(char* input) const
		{
			return justify(add_sign(add_base(upper(input))));
		}

		//TODO this method would be better if it could directly write to the stream
		char* justify(char* input) const
		{
			if (strlen(input) < width())
			{
				uint8_t add = width() - strlen(input);
				if (flags() & left)
				{
					memmove(input + add, input, strlen(input) + 1);
					memset(input, fill(), add);
				}
				else if (flags() & right)
				{
					memset(input + strlen(input), fill(), add);
					input[width() + 1] = 0;
				}
			}
			return input;
		}

		int base() const
		{
			if (flags() & bin) return 2;
			if (flags() & oct) return 8;
			if (flags() & hex) return 16;
			return 10;
		}

		static const uint8_t MAX_BUF_LEN = 64;
		/// @endcond

	private:
		fmtflags flags_;
		uint8_t width_;
		uint8_t precision_;
		char fill_;
		char conversion_buffer[MAX_BUF_LEN];

		// template<typename FSTREAM> friend void bin(FSTREAM&);
		// template<typename FSTREAM> friend void oct(FSTREAM&);
		// template<typename FSTREAM> friend void dec(FSTREAM&);
		// template<typename FSTREAM> friend void hex(FSTREAM&);
		// template<typename FSTREAM> friend void skipws(FSTREAM&);
		// template<typename FSTREAM> friend void noskipws(FSTREAM&);
		// template<typename FSTREAM> friend void boolalpha(FSTREAM&);
		// template<typename FSTREAM> friend void noboolalpha(FSTREAM&);
		// template<typename FSTREAM> friend void showbase(FSTREAM&);
		// template<typename FSTREAM> friend void noshowbase(FSTREAM&);
		// template<typename FSTREAM> friend void showpos(FSTREAM&);
		// template<typename FSTREAM> friend void noshowpos(FSTREAM&);
		// template<typename FSTREAM> friend void uppercase(FSTREAM&);
		// template<typename FSTREAM> friend void nouppercase(FSTREAM&);
		// template<typename FSTREAM> friend void unitbuf(FSTREAM&);
		// template<typename FSTREAM> friend void nounitbuf(FSTREAM&);
		// template<typename FSTREAM> friend void left(FSTREAM&);
		// template<typename FSTREAM> friend void right(FSTREAM&);
		// template<typename FSTREAM> friend void fixed(FSTREAM&);
		// template<typename FSTREAM> friend void scientific(FSTREAM&);
		// template<typename FSTREAM> friend void defaultfloat(FSTREAM&);
	};

	/**
	 * Output stream wrapper to provide formatted output API, a la C++.
	 * @tparam STREAM_ the output stream to wrap, typically OutputBuffer.
	 */
	template<typename STREAM_> class FormattedOutput : public ios_base
	{
	public:
		/** The output stream wrapped by this FormattedOutput. */
		using STREAM = STREAM_;

		/**
		 * Construct a formatted output wrapper of @p stream
		 * @param stream the output stream to be wrapped
		 */
		FormattedOutput(STREAM& stream) : stream_{stream}
		{
		}

		/**
		 * @copydoc OutputBuffer::flush()
		 */
		void flush()
		{
			stream_.flush();
		}

		/**
		 * @copydoc OutputBuffer::put(char, bool)
		 */
		void put(char c, bool call_on_put = true)
		{
			stream_.put(c, call_on_put);
		}

		/**
		 * @copydoc OutputBuffer::put(const char*, size_t)
		 */
		void put(const char* content, size_t size)
		{
			stream_.put(content, size);
		}

		/**
		 * @copydoc OutputBuffer::puts(const char*)
		 */
		void puts(const char* str)
		{
			stream_.puts(str);
		}

		/**
		 * @copydoc OutputBuffer::puts(const flash::FlashStorage*)
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
		FormattedOutput<STREAM>& operator<<(const void* p)
		{
			stream_.puts(convert((uint16_t) p));
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
		FormattedOutput<STREAM>& operator<<(char c)
		{
			stream_.put(c);
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
		FormattedOutput<STREAM>& operator<<(const char* s)
		{
			//FIXME need to implement padding here (justify())
			// stream_.puts(justify(s));
			stream_.puts(s);
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
		FormattedOutput<STREAM>& operator<<(const flash::FlashStorage* s)
		{
			// Specific case: perform justification manually here
			size_t len = strlen_P((const char*) s);
			if (len < width())
			{
				uint8_t add = width() - len;
				if (flags() & left)
				{
					while (add--)
						stream_.put(fill(), false);
					stream_.puts(s);
				}
				else if (flags() & right)
				{
					stream_.puts(s);
					while (add--)
						stream_.put(fill(), false);
				}
			}
			else
				stream_.puts(s);
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
		FormattedOutput<STREAM>& operator<<(int v)
		{
			stream_.puts(convert(v));
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
		FormattedOutput<STREAM>& operator<<(unsigned int v)
		{
			stream_.puts(convert(v));
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
		FormattedOutput<STREAM>& operator<<(long v)
		{
			stream_.puts(convert(v));
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
		FormattedOutput<STREAM>& operator<<(unsigned long v)
		{
			stream_.puts(convert(v));
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
		FormattedOutput<STREAM>& operator<<(double v)
		{
			stream_.puts(convert(v));
			after_insertion();
			return *this;
		}

		/**
		 * General type of a manipulator function applicable to this output stream.
		 */
		using Manipulator = void (*)(FormattedOutput<STREAM>&);

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
		FormattedOutput<STREAM>& operator<<(Manipulator f)
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

		STREAM& stream_;

		// template<typename FSTREAM> friend void flush(FSTREAM&);
		// template<typename FSTREAM> friend void endl(FSTREAM&);
	};

	/**
	 * Input stream wrapper to provide formatted input API, a la C++.
	 * @tparam STREAM_ the input stream to wrap, typically InputBuffer.
	 */
	template<typename STREAM_> class FormattedInput : public ios_base
	{
	public:
		/** The input stream wrapped by FormattedInput. */
		using STREAM = STREAM_;

		/**
		 * Construct a formatted input wrapper of @p stream
		 * @param stream the input stream to be wrapped
		 */
		FormattedInput(STREAM& stream) : stream_{stream}
		{
		}

		/**
		 * @copydoc InputBuffer::available()
		 */
		int available() const
		{
			return stream_.available();
		}

		/**
		 * @copydoc InputBuffer::get()
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
			return get(stream_, content, size);
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
			return gets(stream_, str, max);
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
		FormattedInput<STREAM>& operator>>(bool& v)
		{
			skipws_if_needed();
			char c = containers::pull(stream_.queue());
			v = (c != '0');
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
		FormattedInput<STREAM>& operator>>(char& v)
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
		FormattedInput<STREAM>& operator>>(int& v)
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
		FormattedInput<STREAM>& operator>>(unsigned int& v)
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
		FormattedInput<STREAM>& operator>>(long& v)
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
		FormattedInput<STREAM>& operator>>(unsigned long& v)
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
		FormattedInput<STREAM>& operator>>(double& v)
		{
			skipws_if_needed();
			char buffer[MAX_BUF_LEN];
			convert(stream_.scan(buffer, sizeof buffer), v);
			return *this;
		}

		/**
		 * General type of a manipulator function applicable to this input stream.
		 */
		using Manipulator = void (*)(FormattedInput<STREAM>&);

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
		FormattedInput<STREAM>& operator>>(Manipulator f)
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

		STREAM& stream_;

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
	/// @endcond

	//TODO DOCS
	constexpr const setw_ setw(uint8_t width)
	{
		return setw_{width};
	}
	constexpr const setprecision_ setprecision(uint8_t precision)
	{
		return setprecision_{precision};
	}
	constexpr const setbase_ setbase(int base)
	{
		return setbase_{base};
	}
	constexpr const setfill_ setfill(char fill)
	{
		return setfill_{fill};
	}
	constexpr const setiosflags_ setiosflags(ios::fmtflags mask)
	{
		return setiosflags_{mask};
	}
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

	template<typename FSTREAM> inline void boolalpha(FSTREAM& stream)
	{
		stream.setf(ios::boolalpha);
	}

	template<typename FSTREAM> inline void noboolalpha(FSTREAM& stream)
	{
		stream.unsetf(ios::boolalpha);
	}
	
	template<typename FSTREAM> inline void showbase(FSTREAM& stream)
	{
		stream.setf(ios::showbase);
	}
	
	template<typename FSTREAM> inline void noshowbase(FSTREAM& stream)
	{
		stream.unsetf(ios::showbase);
	}
	
	template<typename FSTREAM> inline void showpos(FSTREAM& stream)
	{
		stream.setf(ios::showpos);
	}
	
	template<typename FSTREAM> inline void noshowpos(FSTREAM& stream)
	{
		stream.unsetf(ios::showpos);
	}
	
	template<typename FSTREAM> inline void uppercase(FSTREAM& stream)
	{
		stream.setf(ios::uppercase);
	}
	
	template<typename FSTREAM> inline void nouppercase(FSTREAM& stream)
	{
		stream.unsetf(ios::uppercase);
	}
	
	template<typename FSTREAM> inline void unitbuf(FSTREAM& stream)
	{
		stream.setf(ios::unitbuf);
	}
	
	template<typename FSTREAM> inline void nounitbuf(FSTREAM& stream)
	{
		stream.unsetf(ios::unitbuf);
	}
	
	template<typename FSTREAM> inline void left(FSTREAM& stream)
	{
		stream.setf(ios::left, ios::adjustfield);
	}
	
	template<typename FSTREAM> inline void right(FSTREAM& stream)
	{
		stream.setf(ios::right, ios::adjustfield);
	}
	
	template<typename FSTREAM> inline void fixed(FSTREAM& stream)
	{
		stream.setf(ios::fixed, ios::floatfield);
	}
	
	template<typename FSTREAM> inline void scientific(FSTREAM& stream)
	{
		stream.setf(ios::scientific, ios::floatfield);
	}

	template<typename FSTREAM> inline void defaultfloat(FSTREAM& stream)
	{
		stream.unsetf(ios::floatfield);
	}
}

#endif /* STREAMS_HH */
/// @endcond
