#ifndef EMPTYSTREAMS_HH
#define	EMPTYSTREAMS_HH

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
	
	EmptyOutput& operator << (char c)
	{
		return *this;
	}
	EmptyOutput& operator << (const char* s)
	{
		return *this;
	}
	EmptyOutput& operator << (int v)
	{
		return *this;
	}
	EmptyOutput& operator << (unsigned int v)
	{
		return *this;
	}
	EmptyOutput& operator << (long v)
	{
		return *this;
	}
	EmptyOutput& operator << (unsigned long v)
	{
		return *this;
	}
	EmptyOutput& operator << (double v)
	{
		return *this;
	}
	
	typedef void (*Manipulator)(EmptyOutput&);
	EmptyOutput& operator << (Manipulator f)
	{
		return *this;
	}
};

inline void bin(EmptyOutput& stream) {}

inline void oct(EmptyOutput& stream) {}

inline void dec(EmptyOutput& stream) {}

inline void hex(EmptyOutput& stream) {}

inline void flush(EmptyOutput& stream) {}

inline void endl(EmptyOutput& stream) {}

#endif	/* EMPTYSTREAMS_HH */
