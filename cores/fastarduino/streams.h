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
	//TODO do we really need on_overflow() why not just use a bool flag?
	//TODO make queue private but delegate pull methods?
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
	class OutputBuffer : public containers::Queue<char, char>
	{
	public:
		template<uint8_t SIZE> OutputBuffer(char (&buffer)[SIZE]) : Queue<char, char>{buffer}
		{
			static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
		}

		/**
		 * Wait until all buffer content has been pulled by a consumer.
		 */
		void flush()
		{
			while (items()) time::yield();
		}

		/**
		 * Append a character to the buffer.
		 * If the buffer is full, then `on_overflow()` will be called.
		 * @param c the character to append
		 * @param call_on_put `true` if `on_put()` should be called after @p c has
		 * been appended, `false` otherwise; when directly calling this method,
		 * you should keep the default value.
		 * @sa on_put()
		 * @sa on_overflow()
		 */
		void put(char c, bool call_on_put = true)
		{
			if (!push(c)) on_overflow(c);
			if (call_on_put) on_put();
		}

		/**
		 * Append several characters to the buffer.
		 * If the buffer is full, then `on_overflow()` will be called.
		 * Once all characters have been appended, `on_put()` will be called,
		 * even if an overflow has occurred.
		 * 
		 * @param content the array of characters to be appended
		 * @param size the number of characters in @p content to append
		 * @sa on_put()
		 * @sa on_overflow()
		 */
		void put(const char* content, size_t size)
		{
			while (size--) put(*content++, false);
			on_put();
		}

		/**
		 * Append a string to the buffer.
		 * The `'\0'` end character of @p str is not transmitted.
		 * If the buffer is full, then `on_overflow()` will be called.
		 * Once all string content has been appended, `on_put()` will be called,
		 * even if an overflow has occurred.
		 * 
		 * @param str the '\0' ended string (standard C-string) to append
		 * @sa on_put()
		 * @sa on_overflow()
		 */
		void puts(const char* str)
		{
			while (*str) put(*str++, false);
			on_put();
		}

		/**
		 * Append a string, stored on flash memory, to the buffer.
		 * The `'\0'` end character of @p str is not transmitted.
		 * If the buffer is full, then `on_overflow()` will be called.
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
		 * @sa on_overflow()
		 */
		void puts(const flash::FlashStorage* str)
		{
			uint16_t address = (uint16_t) str;
			while (char value = pgm_read_byte(address++)) put(value, false);
			on_put();
		}

	protected:
		/**
		 * Callback method called when an attempt to add new content has failed
		 * because the current buffer is full.
		 * @param c the character that could not be appended due to buffer
		 * overflow
		 */
		virtual void on_overflow(UNUSED char c)
		{
		}

		/**
		 * Callback method called when new content has been added to the buffer.
		 * This can be overridden by a subclass to trigger interrupt-driven
		 * transmission of buffer data.
		 * Default implementation does nothing.
		 */
		virtual void on_put()
		{
		}
	};

	//TODO make queue private but provide delegate push()
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
	class InputBuffer : public containers::Queue<char, char>
	{
	public:
		/**
		 * Special value returned by `get()` when buffer is empty.
		 */
		static const int EOF = -1;

		template<uint8_t SIZE> InputBuffer(char (&buffer)[SIZE]) : Queue<char, char>{buffer}
		{
			static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
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

	protected:
		//FIXME these never get called???
		// Listeners of events on the buffer
		//		virtual void on_empty() {}
		//		virtual void on_get(UNUSED char c) {}
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

	/**
	 * Base class for formatted streams.
	 * Allows defining base, width and precision for numbers display.
	 */
	class FormatBase
	{
	public:
		/**
		 * Base for numbers representation.
		 */
		enum class Base : uint8_t
		{
			//		bcd = 0,
			/** Binary */
			bin = 2,
			/** Octal */
			oct = 8,
			/** Decimal */
			dec = 10,
			/** Hexadecimal */
			hex = 16
		};

		FormatBase() : _width{6}, _precision{4}, _base{Base::dec}
		{
		}

		/**
		 * Set minimum width used for displaying values. If a value's 
		 * representation needs less than @p width characters for display, then
		 * additional characters will be added before the value.
		 * For numbers, the base defines the filler character, either `0` for
		 * binary, octal and hexadecimal, or ` ` for decimal.
		 * For strings, a space is added.
		 * If @p width is `>0` then text is right aligned, if @p width is `<0`,
		 * then text is left aligned.
		 * Note that if the value representation needs more than @p width 
		 * characters, then @p width will have no effect on display.
		 * 
		 * @param width the new width to use for next formatted output
		 */
		inline void width(int8_t width)
		{
			_width = width;
		}

		/**
		 * Get the current minimum width value (default = `6`) used for formatted 
		 * output.
		 * @return current minimum width 
		 */
		inline int8_t width()
		{
			return _width;
		}

		/**
		 * Set precision (number of digits after decimal point) used for 
		 * displaying floating values. 
		 * 
		 * @param precision the new precision for next formatted output
		 */
		inline void precision(uint8_t precision)
		{
			_precision = precision;
		}

		/**
		 * Get the current precision (default = `4`) used for formatted 
		 * floating values output.
		 * @return current precision
		 */
		inline uint8_t precision()
		{
			return _precision;
		}

		/**
		 * Set new base to use for next outputs of numbers.
		 * 
		 * @param base new base used for numbers display
		 */
		inline void base(Base base)
		{
			_base = base;
		}

		/**
		 * Get the current base used for numbers representation (default = `Base::dec`).
		 * @return current base
		 */
		inline Base base()
		{
			return _base;
		}

	protected:
		/// @cond notdocumented
		void reset()
		{
			_width = 6;
			_precision = 4;
			_base = Base::dec;
		}
		// conversions from string to numeric value
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
			long value = strtol(token, &endptr, (uint8_t) _base);
			if (endptr == token) return false;
			v = value;
			return true;
		}
		bool convert(const char* token, unsigned long& v)
		{
			char* endptr;
			unsigned long value = strtoul(token, &endptr, (uint8_t) _base);
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
			return justify(itoa(v, conversion_buffer, (uint8_t) _base), filler());
		}
		const char* convert(unsigned int v)
		{
			return justify(utoa(v, conversion_buffer, (uint8_t) _base), filler());
		}
		const char* convert(long v)
		{
			return justify(ltoa(v, conversion_buffer, (uint8_t) _base), filler());
		}
		const char* convert(unsigned long v)
		{
			return justify(ultoa(v, conversion_buffer, (uint8_t) _base), filler());
		}
		const char* convert(double v)
		{
			return dtostrf(v, _width, _precision, conversion_buffer);
		}
		const char* justify(char* input, char filler = ' ')
		{
			uint8_t width = (_width >= 0 ? _width : -_width);
			if (strlen(input) < width)
			{
				uint8_t add = width - strlen(input);
				memmove(input + add, input, strlen(input) + 1);
				memset(input, filler, add);
			}
			return input;
		}
		char filler()
		{
			switch (_base)
			{
			case Base::bin:
			case Base::hex:
			case Base::oct:
				return '0';
			case Base::dec:
				return ' ';
			}
		}

		static const uint8_t MAX_BUF_LEN = 64;
		/// @endcond

	private:
		int8_t _width;
		uint8_t _precision;
		Base _base;
		char conversion_buffer[MAX_BUF_LEN];
	};

	//TODO Add reset of latest format used
	/**
	 * Output stream wrapper to provide formatted output API, a la C++.
	 * @tparam STREAM the output stream to wrap, typically OutputBuffer.
	 */
	template<typename STREAM> class FormattedOutput : public FormatBase
	{
	public:
		/**
		 * Construct a formatted output wrapper of @p stream
		 * @param stream the output stream to be wrapped
		 */
		FormattedOutput(STREAM& stream) : _stream{stream}
		{
		}

		/**
		 * @copydoc OutputBuffer::flush()
		 */
		void flush()
		{
			_stream.flush();
		}

		/**
		 * @copydoc OutputBuffer::put(char, bool)
		 */
		void put(char c, bool call_on_put = true)
		{
			_stream.put(c, call_on_put);
		}

		/**
		 * @copydoc OutputBuffer::put(const char*, size_t)
		 */
		void put(const char* content, size_t size)
		{
			_stream.put(content, size);
		}

		/**
		 * @copydoc OutputBuffer::puts(const char*)
		 */
		void puts(const char* str)
		{
			_stream.puts(str);
		}

		/**
		 * @copydoc OutputBuffer::puts(const flash::FlashStorage*)
		 */
		void puts(const flash::FlashStorage* str)
		{
			_stream.puts(str);
		}

		//TODO add support for void* (address)

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
			_stream.put(c);
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
			//TODO Add justify with width if <0
			_stream.puts(s);
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
			//TODO Add justify with width if <0
			_stream.puts(s);
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
			_stream.puts(convert(v));
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
			_stream.puts(convert(v));
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
			_stream.puts(convert(v));
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
			_stream.puts(convert(v));
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
			_stream.puts(convert(v));
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
		STREAM& _stream;

		/// @cond notdocumented
		template<typename FSTREAM> friend void bin(FSTREAM&);
		template<typename FSTREAM> friend void oct(FSTREAM&);
		template<typename FSTREAM> friend void dec(FSTREAM&);
		template<typename FSTREAM> friend void hex(FSTREAM&);

		template<typename FSTREAM> friend void flush(FSTREAM&);
		template<typename FSTREAM> friend void endl(FSTREAM&);
		/// @endcond
	};

	/**
	 * Input stream wrapper to provide formatted input API, a la C++.
	 * @tparam STREAM the input stream to wrap, typically InputBuffer.
	 */
	template<typename STREAM> class FormattedInput : public FormatBase
	{
	public:
		/**
		 * Construct a formatted input wrapper of @p stream
		 * @param stream the input stream to be wrapped
		 */
		FormattedInput(STREAM& stream) : _stream{stream}, _skipws{true}
		{
		}

		/**
		 * @copydoc InputBuffer::available()
		 */
		int available() const
		{
			return _stream.available();
		}

		/**
		 * @copydoc InputBuffer::get()
		 */
		int get()
		{
			return _stream.get();
		}

		/**
		 * Wait for this buffer to have at least @p size characters and get them.
		 * @param content the character array that will receive all characters read
		 * @param size the number of characters to read
		 * @return @p content
		 */
		char* get(char* content, size_t size)
		{
			return get(_stream, content, size);
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
			return gets(_stream, str, max);
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
			skip_whitespaces(_skipws);
			char c = containers::pull(_stream);
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
			skip_whitespaces(_skipws);
			v = containers::pull(_stream);
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
			skip_whitespaces(_skipws);
			char buffer[sizeof(int) * 8 + 1];
			convert(_stream.scan(buffer, sizeof buffer), v);
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
			skip_whitespaces(_skipws);
			char buffer[sizeof(int) * 8 + 1];
			convert(_stream.scan(buffer, sizeof buffer), v);
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
			skip_whitespaces(_skipws);
			char buffer[sizeof(long) * 8 + 1];
			convert(_stream.scan(buffer, sizeof buffer), v);
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
			skip_whitespaces(_skipws);
			char buffer[sizeof(long) * 8 + 1];
			convert(_stream.scan(buffer, sizeof buffer), v);
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
			skip_whitespaces(_skipws);
			char buffer[MAX_BUF_LEN];
			convert(_stream.scan(buffer, sizeof buffer), v);
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
		void skip_whitespaces(bool skip = true)
		{
			if (skip)
				while (isspace(containers::peek(_stream))) containers::pull(_stream);
		}

		STREAM& _stream;
		bool _skipws;

		/// @cond notdocumented
		template<typename FSTREAM> friend void bin(FSTREAM&);
		template<typename FSTREAM> friend void oct(FSTREAM&);
		template<typename FSTREAM> friend void dec(FSTREAM&);
		template<typename FSTREAM> friend void hex(FSTREAM&);
		template<typename FSTREAM> friend void ws(FSTREAM&);
		template<typename FSTREAM> friend void skipws(FSTREAM&);
		template<typename FSTREAM> friend void noskipws(FSTREAM&);
		/// @endcond
	};

	//TODO define argumented manipulators for width, precision, filler

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
		stream.skip_whitespaces();
	}

	/**
	 * Manipulator for an input stream, which will activate whitespace discarding
	 * before formatted input operations on that stream .
	 */
	template<typename FSTREAM> inline void skipws(FSTREAM& stream)
	{
		stream._skipws = true;
	}

	/**
	 * Manipulator for an input stream, which will deactivate whitespace discarding
	 * before formatted input operations on that stream .
	 */
	template<typename FSTREAM> inline void noskipws(FSTREAM& stream)
	{
		stream._skipws = false;
	}

	/**
	 * Manipulator for an output or input stream, which will set the base, used to
	 * represent (output) or interpret (input) integral	numerical values, to binary.
	 */
	template<typename FSTREAM> inline void bin(FSTREAM& stream)
	{
		stream.base(FormatBase::Base::bin);
	}

	/**
	 * Manipulator for an output or input stream, which will set the base, used to
	 * represent (output) or interpret (input) integral	numerical values, to octal.
	 */
	template<typename FSTREAM> inline void oct(FSTREAM& stream)
	{
		stream.base(FormatBase::Base::oct);
	}

	/**
	 * Manipulator for an output or input stream, which will set the base, used to
	 * represent (output) or interpret (input) integral	numerical values, to decimal.
	 */
	template<typename FSTREAM> inline void dec(FSTREAM& stream)
	{
		stream.base(FormatBase::Base::dec);
	}

	/**
	 * Manipulator for an output or input stream, which will set the base, used to
	 * represent (output) or interpret (input) integral	numerical values, to hexadecimal.
	 */
	template<typename FSTREAM> inline void hex(FSTREAM& stream)
	{
		stream.base(FormatBase::Base::hex);
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
}

#endif /* STREAMS_HH */
/// @endcond
