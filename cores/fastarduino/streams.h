//   Copyright 2016-2018 Jean-Francois Poilpret
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
#include "queue.h"
#include "flash.h"
#include "ios.h"
#include "streambuf.h"
#include "time.h"

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

		ostream& operator=(const ostream&) = delete;

		/**
		 * Return the stream buffer associated with this stream.
		 */
		ostreambuf& rdbuf() const
		{
			return stream_;
		}

		/**
		 * Flush this ostream and blocks until all its buffer has been written to
		 * the underlying device.
		 * A manipulator exists with the same name and behavior.
		 * @sa ostreambuf::pubsync()
		 * @sa streams::flush()
		 */
		void flush()
		{
			stream_.pubsync();
		}

		/**
		 * Insert character @p c into this stream. The character is buffered by 
		 * the underlying ostreambuf and transmitted to the connected device when 
		 * possible. 
		 * If the underlying ostreambuf overflows, then `badbit` is set for this stream.
		 * @sa ostreambuf::sputc(char)
		 */
		void put(char c)
		{
			stream_.sputc(c);
			check_overflow();
		}

		/**
		 * Write a block of data to this stream. @p content gets buffered by the
		 * underlying ostreambuf and transmitted to the connected device when 
		 * possible.
		 * If the underlying ostreambuf overflows, then `badbit` is set for this stream.
		 * @param content data to be transmitted
		 * @param size the number of bytes from @p content to be transmitted
		 * @sa ostreambuf::sputn(const char*, size_t)
		 */
		void write(const char* content, size_t size)
		{
			stream_.sputn(content, size);
			check_overflow();
		}

		/**
		 * Write a string (null-terminated) to this stream. @p str gets buffered
		 * by the underlying ostreambuf and transmitted to the connected device when 
		 * possible. The terminating `\0` is not transmitted.
		 * If the underlying ostreambuf overflows, then `badbit` is set for this stream.
		 * @param str the string to be transmitted
		 * @sa ostreambuf::sputn(const char*)
		 */
		void write(const char* str)
		{
			stream_.sputn(str);
			check_overflow();
		}

		/**
		 * Write a flash-stored string (null-terminated) to this stream. @p str 
		 * gets buffered by the underlying ostreambuf and transmitted to the 
		 * connected device when possible. The terminating `\0` is not transmitted.
		 * If the underlying ostreambuf overflows, then `badbit` is set for this stream.
		 * @param str the string to be transmitted
		 * @sa ostreambuf::sputn(const flash::FlashStorage*)
		 */
		void write(const flash::FlashStorage* str)
		{
			stream_.sputn(str);
			check_overflow();
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
			check_overflow();
			if (flags() & unitbuf)
				stream_.pubsync();
			width(0);
		}

		void check_overflow()
		{
			if (stream_.overflow()) setstate(badbit);
		}

		ostreambuf& stream_;
	};

	// NOTE: istream API is blocking, while istreambuf is not
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

		istream& operator=(const istream&) = delete;

		/**
		 * Return the stream buffer associated with this stream.
		 */
		istreambuf& rdbuf() const
		{
			return stream_;
		}


		/**
		 * Return the next character in this input stream, without extracting it.
		 * The method blocks until one character is available in the underlying 
		 * istreambuf.
		 * @sa istreambuf::sgetc()
		 */
		int peek()
		{
			int value;
			while ((value = stream_.sgetc()) == istreambuf::EOF) time::yield();
			return value;
		}

		/**
		 * Extract a single character from this input stream.
		 * The method blocks until one character is available in the underlying 
		 * istreambuf.
		 * @sa istreambuf::sbumpc()
		 */
		int get()
		{
			int value;
			while ((value = stream_.sbumpc()) == istreambuf::EOF) time::yield();
			return value;
		}

		/**
		 * Extract a single character from this input stream.
		 * The method blocks until one character is available in the underlying 
		 * istreambuf.
		 * @sa get()
		 * @sa istreambuf::sbumpc()
		 */
		istream& get(char& c)
		{
			c = get();
			return *this;
		}

		/**
		 * Extract characters from this input stream and stores them as a C-string,
		 * until either `(n - 1)` characters have been extracted or the @p delim
		 * character is encountered. The delimiting character is not extracted 
		 * from the stream and also not added to @p s.
		 * A null character `\0` is automatically appended to @p s.
		 * @sa getline()
		 */
		istream& get(char* s, size_t n, char delim = '\n')
		{
			while (--n)
			{
				int c = peek();
				if (c == delim) break;
				*s++ = get();
			}
			*s = 0;
			return *this;
		}

		/**
		 * Extract characters from this input stream and stores them as a C-string,
		 * until either `(n - 1)` characters have been extracted or the @p delim
		 * character is encountered. The delimiting character is extracted 
		 * from the stream but is not added to @p s.
		 * A null character `\0` is automatically appended to @p s.
		 * @sa get(char*, size_t, char)
		 */
		istream& getline(char* s, size_t n, char delim = '\n')
		{
			while (--n)
			{
				int c = get();
				if (c == delim) break;
				*s++ = c;
			}
			*s = 0;
			return *this;
		}

		/**
		 * Extract characters from this input stream and discards them, until
		 * either `n` characters have been extracted, or the @p delim character
		 * is encountered. The delimiting character, if found, is also discarded.
		 * If @p n is `0`, then **all** characters are discarded (no number limit)
		 * until @p delim is encountered.
		 */
		istream& ignore(size_t n = 1, int delim = istreambuf::EOF)
		{
			bool forever = !n;
			while (forever || n--)
				if (get() == delim) break;
			return *this;
		}

		/**
		 * Read a block of data from this input stream.
		 * Extracts exactly @p n characters and copies them to @p s.
		 * The method blocks until @p n characters have been read.
		 */
		istream& read(char* s, size_t n)
		{
			while (n--)
				*s++ = get();
			return *this;
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
				scan(buf, width());
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
			convert(scan(buffer, sizeof buffer), v);
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
			convert(scan(buffer, sizeof buffer), v);
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
			convert(scan(buffer, sizeof buffer), v);
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
			convert(scan(buffer, sizeof buffer), v);
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
			convert(scan(buffer, sizeof buffer), v);
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
			convert(scan(buffer, sizeof buffer), v);
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

		char* scan(char* str, size_t max);

		istreambuf& stream_;

		template<typename FSTREAM> friend void ws(FSTREAM&);
	};

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
