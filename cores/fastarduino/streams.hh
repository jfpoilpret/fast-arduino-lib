#ifndef STREAMS_HH
#define	STREAMS_HH

#include <stddef.h>
#include <stdlib.h>
#include "Queue.hh"

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

private:
	static const uint8_t MAX_BUF_LEN = 64;
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
//	void puts_P();
	
	//TODO others? eg void*, PSTR, Manipulator...
	FormattedOutput<STREAM>& operator << (const char* s)
	{
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
	
private:
	STREAM& _stream;	
};

class OutputBuffer:public Queue<char>
{
public:
	template<uint8_t SIZE>
	OutputBuffer(char (&buffer)[SIZE]): Queue<char>(buffer, SIZE)
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}
	
	template<uint8_t SIZE>
	static OutputBuffer create(char buffer[SIZE])
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
		return OutputBuffer(buffer, SIZE);
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
	OutputBuffer(char* buffer, uint8_t size): Queue<char>(buffer, size) {}
	// Listeners of events on the buffer
	virtual void on_overflow(__attribute__((unused)) char c) {}
	virtual void on_flush() {}
};

//TODO Handle generic errors coming from UART RX (eg Parity...)
class InputBuffer: public Queue<char>
{
public:
	static const int EOF = -1;
	
	template<uint8_t SIZE>
	InputBuffer(char (&buffer)[SIZE]): Queue<char>(buffer, SIZE)
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
	}

	template<uint8_t SIZE>
	static InputBuffer create(char buffer[SIZE])
	{
		static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
		return InputBuffer(buffer, SIZE);
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
	//TODO
	int gets(char* str, size_t max);
	
//	InputBuffer& operator >> (bool& b);
//	InputBuffer& operator >> (char& c);
//	InputBuffer& operator >> (char* s);
//	InputBuffer& operator >> (int& d);
//	InputBuffer& operator >> (unsigned int& d);
//	InputBuffer& operator >> (long& d);
//	InputBuffer& operator >> (unsigned long& d);
//	InputBuffer& operator >> (float& f);
//	InputBuffer& operator >> (double& f);
	
protected:
	InputBuffer(char* buffer, uint8_t size): Queue<char>(buffer, size) {}
	// Listeners of events on the buffer
	virtual void on_empty() {}
	virtual void on_get(__attribute__((unused)) char c) {}
};

#endif	/* STREAMS_HH */
