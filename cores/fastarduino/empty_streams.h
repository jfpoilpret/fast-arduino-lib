//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * Empty version of streams API, useful when you want to introduce traces that
 * you can easily disable without adding preprocessor conditional compilation
 * everywhere traces are output.
 * Note that you shall not include "empty_streams.h" and "streams.h" in the same
 * source file.
 */
#ifndef EMPTYSTREAMS_HH
#define EMPTYSTREAMS_HH

#include <stddef.h>
#include "ios.h"
#include "flash.h"
#include "utilities.h"

namespace streams
{
	/**
	 * Implements an empty formatted output that does nothing.
	 * It can be used everywhere a `streams::ostream` is expected but 
	 * produces no code at all.
	 * The following example shows how to add traces that can be disabled at 
	 * compile time by defining a specific macro:
	 * @code
	 * // Include the right header
	 * #ifdef NO_TRACE
	 * #include <fastarduino/empty_streams.h>
	 * #else
	 * #include <fastarduino/streams.h>
	 * #endif
	 * //...
	 * // Instantiate the right output
	 * #ifdef NO_TRACE
	 * streams::null_ostream trace;
	 * #else
	 * // We use UART for tracing
	 * static char output_buffer[64];
	 * serial::hard::UATX<board::USART::USART0> uatx{output_buffer};
	 * auto trace = uatx.out();
	 * #endif
	 * //...
	 * // Directly use trace anywhere, this won't generate any code if `NO_TRACE`
	 * // was defined at compile time
	 * void f(int value)
	 * {
	 *     //... do something
	 *     trace << F("DEBUG") << value << streams::endl;
	 *     //... do something else
	 * }
	 * @endcode
	 */
	class null_ostream
	{
	public:
		/// @cond notdocumented
		null_ostream() = default;

		void setf(ios::fmtflags flags UNUSED) {}

		void setf(ios::fmtflags flags UNUSED, ios::fmtflags mask UNUSED) {}

		void unsetf(ios::fmtflags flags UNUSED) {}

		void fill(char fill UNUSED) {}

		void width(uint8_t width UNUSED) {}

		void precision(uint8_t precision UNUSED) {}

		void flush() {}
		void put(char val UNUSED) {}
		void write(const char* content UNUSED, size_t size UNUSED) {}
		void write(const char* str UNUSED) {}
		void write(const flash::FlashStorage* str UNUSED) {}

		null_ostream& operator<<(const void* ptr UNUSED)
		{
			return *this;
		}
		null_ostream& operator<<(bool val UNUSED)
		{
			return *this;
		}
		null_ostream& operator<<(char val UNUSED)
		{
			return *this;
		}
		null_ostream& operator<<(const char* str UNUSED)
		{
			return *this;
		}
		null_ostream& operator<<(int val UNUSED)
		{
			return *this;
		}
		null_ostream& operator<<(unsigned int val UNUSED)
		{
			return *this;
		}
		null_ostream& operator<<(long val UNUSED)
		{
			return *this;
		}
		null_ostream& operator<<(unsigned long val UNUSED)
		{
			return *this;
		}
		null_ostream& operator<<(double val UNUSED)
		{
			return *this;
		}

		typedef void (*Manipulator)(null_ostream&);
		null_ostream& operator<<(Manipulator func UNUSED)
		{
			return *this;
		}
		/// @endcond
	};

	/// @cond notdocumented
	void bin(null_ostream& stream UNUSED) {}
	void oct(null_ostream& stream UNUSED) {}
	void dec(null_ostream& stream UNUSED) {}
	void hex(null_ostream& stream UNUSED) {}
	void flush(null_ostream& stream UNUSED) {}
	void endl(null_ostream& stream UNUSED) {}
	void skipws(null_ostream& stream UNUSED) {}
	void noskipws(null_ostream& stream UNUSED) {}
	void boolalpha(null_ostream& stream UNUSED) {}
	void noboolalpha(null_ostream& stream UNUSED) {}
	void showbase(null_ostream& stream UNUSED) {}
	void noshowbase(null_ostream& stream UNUSED) {}
	void showpos(null_ostream& stream UNUSED) {}
	void noshowpos(null_ostream& stream UNUSED) {}
	void uppercase(null_ostream& stream UNUSED) {}
	void nouppercase(null_ostream& stream UNUSED) {}
	void unitbuf(null_ostream& stream UNUSED) {}
	void nounitbuf(null_ostream& stream UNUSED) {}
	void left(null_ostream& stream UNUSED) {}
	void right(null_ostream& stream UNUSED) {}
	void fixed(null_ostream& stream UNUSED) {}
	void scientific(null_ostream& stream UNUSED) {}
	/// @endcond
}

#endif /* EMPTYSTREAMS_HH */
/// @endcond
