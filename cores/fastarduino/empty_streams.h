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

#ifndef EMPTYSTREAMS_HH
#define	EMPTYSTREAMS_HH

#include <stddef.h>
#include "utilities.h"

class EmptyOutput
{
public:
	EmptyOutput() {}
	
	void flush() {}
	void put(char c UNUSED, bool call_on_put UNUSED = true) {}
	void put(const char* content UNUSED, size_t size UNUSED) {}
	void puts(const char* str UNUSED) {}
//	void puts_P();
	
	EmptyOutput& operator << (char c UNUSED)
	{
		return *this;
	}
	EmptyOutput& operator << (const char* s UNUSED)
	{
		return *this;
	}
	EmptyOutput& operator << (int v UNUSED)
	{
		return *this;
	}
	EmptyOutput& operator << (unsigned int v UNUSED)
	{
		return *this;
	}
	EmptyOutput& operator << (long v UNUSED)
	{
		return *this;
	}
	EmptyOutput& operator << (unsigned long v UNUSED)
	{
		return *this;
	}
	EmptyOutput& operator << (double v UNUSED)
	{
		return *this;
	}
	
	typedef void (*Manipulator)(EmptyOutput&);
	EmptyOutput& operator << (Manipulator f UNUSED)
	{
		return *this;
	}
};

inline void bin(EmptyOutput& stream UNUSED) {}

inline void oct(EmptyOutput& stream UNUSED) {}

inline void dec(EmptyOutput& stream UNUSED) {}

inline void hex(EmptyOutput& stream UNUSED) {}

inline void flush(EmptyOutput& stream UNUSED) {}

inline void endl(EmptyOutput& stream UNUSED) {}

#endif	/* EMPTYSTREAMS_HH */
