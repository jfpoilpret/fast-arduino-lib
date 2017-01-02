#ifndef EMPTYSTREAMS_HH
#define	EMPTYSTREAMS_HH

#include <stddef.h>
#include "utilities.hh"

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
