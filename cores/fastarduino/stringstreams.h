//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * C++-like std::stringstream facilities.
 */
#ifndef STRINGSTREAMS_H
#define STRINGSTREAMS_H

#include "streams.h"

/**
 * Defines C++-like stringstreams API; these are just wrappers around istream and ostream,
 * with extra methods to get and set string content.
 * TODO replace/remove
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
 * 
 * @sa stream::ostream
 * @sa stream::istream
 */
namespace streams
{
	#if 0
	//TODO abstract class then final class with instantiation of buffer?
	class ostringstream : public ostream
	{
	public:
		template<uint8_t SIZE>
		explicit ostringstream(char (&buffer)[SIZE]) : ostream{obuf_}, obuf_{buffer} {}

		/// @cond notdocumented
		ostringstream(const ostringstream&) = delete;
		ostringstream& operator=(const ostringstream&) = delete;
		/// @endcond

		const char* get_string(char* output, uint8_t size) const
		{
			//TODO
			// obuf_.xxx
		}

		//TODO clear()? reset()?

	private:
		streams::ostreambuf obuf_;
	};
	#endif

	#if 0
	class istringstream : public istream
	{
	public:
		// template<uint8_t SIZE>
		explicit istringstream(const char* input) : istream{ibuf_}, ibuf_{input} {}

		/// @cond notdocumented
		istringstream(const istringstream&) = delete;
		istringstream& operator=(const istringstream&) = delete;
		/// @endcond

	private:
		streams::istreambuf ibuf_;
	};
	#endif

		// ostreambuf& rdbuf() const
		// {
		// 	return streambuf_;
		// }

		// void after_insertion()
		// {
		// 	check_overflow();
		// 	if (flags() & unitbuf) streambuf_.pubsync();
		// 	width(0);
		// }
}

#endif /* STRINGSTREAMS_H */
/// @endcond
