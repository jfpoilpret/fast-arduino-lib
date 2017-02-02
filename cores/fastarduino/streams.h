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

#ifndef STREAMS_HH
#define	STREAMS_HH

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "Queue.hh"

//TODO Add operator out << in; to unstack all input and push it to output (optimization possible?)
//TODO Maybe add operator in >> out; question is when should the redirection be finished?
class OutputBuffer:public Queue<char, char>
{
public:
	template<uint8_t SIZE>
	OutputBuffer(char (&buffer)[SIZE]): Queue<char, char>(buffer)
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}
	
	void flush()
	{
		while (items())
			Time::yield();
	}
	void put(char c, bool call_on_put = true)
	{
		if (!push(c)) on_overflow(c);
		if (call_on_put) on_put();
	}
	void put(const char* content, size_t size)
	{
		while (size--) put(*content++, false);
		on_put();
	}
	void puts(const char* str)
	{
		while (*str) put(*str++, false);
		on_put();
	}
//	void puts_P();
	
protected:
	// Listeners of events on the buffer
	virtual void on_overflow(UNUSED char c) {}
	virtual void on_put() {}
};

//TODO Handle generic errors coming from UART RX (eg Parity...)
//TODO allow blocking input somehow
class InputBuffer: public Queue<char, char>
{
public:
	static const int EOF = -1;
	
	template<uint8_t SIZE>
	InputBuffer(char (&buffer)[SIZE]): Queue<char, char>(buffer)
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}

	int available() const
	{
		return items();
	}
	int get()
	{
		char value;
		if (pull(value)) return value;
		return EOF;
	}
	
protected:
	// Listeners of events on the buffer
	virtual void on_empty() {}
	virtual void on_get(UNUSED char c) {}
	
	void scan(char* str, size_t max);
};

// The following functions are blocking until input is satisfied
char get(InputBuffer& in);
char* get(InputBuffer& in, char* content, size_t size);
int gets(InputBuffer& in, char* str, size_t max, char end = 0);

class FormatBase
{
public:
	enum class Base: uint8_t
	{
//		bcd = 0,
		bin = 2,
		oct = 8,
		dec = 10,
		hex = 16
	};
	
	FormatBase(): _width(6), _precision(4), _base(Base::dec) {}
	
	inline void width(int8_t width)
	{
		_width = width;
	}
	inline int8_t width()
	{
		return _width;
	}
	inline void precision(int8_t precision)
	{
		_precision = precision;
	}
	inline int8_t precision()
	{
		return _precision;
	}
	inline void base(Base base)
	{
		_base = base;
	}
	inline Base base()
	{
		return _base;
	}
	
protected:
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
		if (endptr == token)
			return false;
		v = value;
		return true;
	}
	bool convert(const char* token, long& v)
	{
		char* endptr;
		long value = strtol(token, &endptr, (uint8_t) _base);
		if (endptr == token)
			return false;
		v = value;
		return true;
	}
	bool convert(const char* token, unsigned long& v)
	{
		char* endptr;
		unsigned long value = strtoul(token, &endptr, (uint8_t) _base);
		if (endptr == token)
			return false;
		v = value;
		return true;
	}
	bool convert(const char* token, int& v)
	{
		long value;
		if (!convert(token, value))
			return false;
		v = (int) value;
		return true;
	}
	bool convert(const char* token, unsigned int& v)
	{
		unsigned long value;
		if (!convert(token, value))
			return false;
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
	
private:
	int8_t _width;
	uint8_t _precision;
	Base _base;
	char conversion_buffer[MAX_BUF_LEN];
};

//TODO Add reset of latest format used
template<typename STREAM>
class FormattedOutput: public FormatBase
{
public:
	FormattedOutput(STREAM& stream): _stream(stream) {}

	void flush()
	{
		_stream.flush();
	}
	void put(char c, bool call_on_put = true)
	{
		_stream.put(c, call_on_put);
	}
	void put(const char* content, size_t size)
	{
		_stream.put(content, size);
	}
	void puts(const char* str)
	{
		_stream.puts(str);
	}
	//TODO Handle PROGMEM strings output
//	void puts_P();
	
	//TODO add support for char, void* (address), PSTR
	FormattedOutput<STREAM>& operator << (char c)
	{
		_stream.put(c);
		return *this;
	}
	FormattedOutput<STREAM>& operator << (const char* s)
	{
		//TODO Add justify with width if <0
		_stream.puts(s);
		return *this;
	}
	FormattedOutput<STREAM>& operator << (int v)
	{
		_stream.puts(convert(v));
		return *this;
	}
	FormattedOutput<STREAM>& operator << (unsigned int v)
	{
		_stream.puts(convert(v));
		return *this;
	}
	FormattedOutput<STREAM>& operator << (long v)
	{
		_stream.puts(convert(v));
		return *this;
	}
	FormattedOutput<STREAM>& operator << (unsigned long v)
	{
		_stream.puts(convert(v));
		return *this;
	}
	FormattedOutput<STREAM>& operator << (double v)
	{
		_stream.puts(convert(v));
		return *this;
	}
	
	typedef void (*Manipulator)(FormattedOutput<STREAM>&);
	FormattedOutput<STREAM>& operator << (Manipulator f)
	{
		f(*this);
		return *this;
	}
	
private:
	STREAM& _stream;
	
	template<typename FSTREAM> friend FSTREAM& bin(FSTREAM&);
	template<typename FSTREAM> friend FSTREAM& oct(FSTREAM&);
	template<typename FSTREAM> friend FSTREAM& dec(FSTREAM&);
	template<typename FSTREAM> friend FSTREAM& hex(FSTREAM&);

	template<typename FSTREAM> friend FSTREAM& flush(FSTREAM&);
	template<typename FSTREAM> friend FSTREAM& endl(FSTREAM&);
};

template<typename STREAM>
class FormattedInput: public FormatBase
{
public:
	FormattedInput(STREAM& stream): _stream(stream) {}

	int available() const
	{
		return _stream.available();
	}
	int get()
	{
		return _stream.get();
	}
	int get(char* content, size_t size)
	{
		return _stream.get(content, size);
	}
	int gets(char* str, size_t max)
	{
		return _stream.gets(str, max);
	}
	
	FormattedInput<STREAM>& operator >> (int& v)
	{
		char buffer[sizeof(int) * 8 + 1];
		convert(_stream.scan(buffer, sizeof buffer), v);
		return *this;
	}
	FormattedInput<STREAM>& operator >> (unsigned int& v)
	{
		char buffer[sizeof(int) * 8 + 1];
		convert(_stream.scan(buffer, sizeof buffer), v);
		return *this;
	}
	FormattedInput<STREAM>& operator >> (long& v)
	{
		char buffer[sizeof(long) * 8 + 1];
		convert(_stream.scan(buffer, sizeof buffer), v);
		return *this;
	}
	FormattedInput<STREAM>& operator >> (unsigned long& v)
	{
		char buffer[sizeof(long) * 8 + 1];
		convert(_stream.scan(buffer, sizeof buffer), v);
		return *this;
	}
	FormattedInput<STREAM>& operator >> (double& v)
	{
		char buffer[MAX_BUF_LEN];
		convert(_stream.scan(buffer, sizeof buffer), v);
		return *this;
	}
	
	typedef void (*Manipulator)(FormattedInput<STREAM>&);
	FormattedInput<STREAM>& operator >> (Manipulator f)
	{
		f(*this);
		return *this;
	}
	
private:
	STREAM& _stream;
	
	template<typename FSTREAM> friend FSTREAM& bin(FSTREAM&);
	template<typename FSTREAM> friend FSTREAM& oct(FSTREAM&);
	template<typename FSTREAM> friend FSTREAM& dec(FSTREAM&);
	template<typename FSTREAM> friend FSTREAM& hex(FSTREAM&);
};

template<typename FSTREAM>
inline void bin(FSTREAM& stream)
{
	stream.base(FormatBase::Base::bin);
}

template<typename FSTREAM>
inline void oct(FSTREAM& stream)
{
	stream.base(FormatBase::Base::oct);
}

template<typename FSTREAM>
inline void dec(FSTREAM& stream)
{
	stream.base(FormatBase::Base::dec);
}

template<typename FSTREAM>
inline void hex(FSTREAM& stream)
{
	stream.base(FormatBase::Base::hex);
}

template<typename FSTREAM>
inline void flush(FSTREAM& stream)
{
	stream.flush();
}

template<typename FSTREAM>
inline void endl(FSTREAM& stream)
{
	stream.put('\n');
}

#endif	/* STREAMS_HH */
