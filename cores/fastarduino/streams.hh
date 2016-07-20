#ifndef STREAMS_HH
#define	STREAMS_HH

#include <stddef.h>
#include <stdlib.h>
#include "Queue.hh"

class OutputBuffer:public Queue<char>
{
public:
	template<uint8_t SIZE>
	OutputBuffer(char (&buffer)[SIZE]): Queue<char>(buffer)
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}
	
	void flush()
	{
		on_flush();
	}
	void put(char c, bool flush = true)
	{
		if (!push(c)) on_overflow(c);
		if (flush) on_flush();
	}
	void put(const char* content, size_t size)
	{
		while (size--) put(*content++, false);
		on_flush();
	}
	void puts(const char* str)
	{
		while (*str) put(*str++, false);
		on_flush();
	}
//	void puts_P();
	
protected:
	// Listeners of events on the buffer
	virtual void on_overflow(__attribute__((unused)) char c) {}
	virtual void on_flush() {}
};

//TODO Handle generic errors coming from UART RX (eg Parity...)
//TODO allow blocking input somehow
class InputBuffer: public Queue<char>
{
public:
	static const int EOF = -1;
	
	template<uint8_t SIZE>
	InputBuffer(char (&buffer)[SIZE], bool blocking = false): Queue<char>(buffer), _blocking(blocking)
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}

	int available() const
	{
		return items();
	}
	int get();
	int get(char* content, size_t size);
	int gets(char* str, size_t max);
	
protected:
	// Listeners of events on the buffer
	virtual void on_empty() {}
	virtual void on_get(__attribute__((unused)) char c) {}
	
	void scan(char* str, size_t max);

private:
	const bool _blocking;
};

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
		char s[8 * sizeof(int) + 1];
		return itoa(v, s, (uint8_t) _base);
	}
	const char* convert(unsigned int v)
	{
		char s[8 * sizeof(int) + 1];
		return utoa(v, s, (uint8_t) _base);
	}
	const char* convert(long v)
	{
		char s[8 * sizeof(long) + 1];
		return ltoa(v, s, (uint8_t) _base);
	}
	const char* convert(unsigned long v)
	{
		char s[8 * sizeof(long) + 1];
		return ultoa(v, s, (uint8_t) _base);
	}
	const char* convert(double v)
	{
		char s[MAX_BUF_LEN];
		return dtostrf(v, _width, _precision, s);
	}

	static const uint8_t MAX_BUF_LEN = 64;
	
private:
	int8_t _width;
	uint8_t _precision;
	Base _base;
};

template<typename STREAM>
class FormattedOutput: public FormatBase
{
public:
	FormattedOutput(STREAM& stream): _stream(stream) {}

	void flush()
	{
		_stream.flush();
	}
	void put(char c, bool flush = true)
	{
		_stream.put(c, flush);
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
	
	//TODO others? eg void*, PSTR, Manipulator...
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
