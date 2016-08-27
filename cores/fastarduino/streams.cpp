#include <ctype.h>
#include "streams.hh"

int InputBuffer::get()
{
	if (_blocking)
	{
		return ::pull<char>(*this);
	}
	else
	{
		char value;
		if (pull(value)) return value;
		return EOF;
	}
}

int InputBuffer::get(char* content, size_t size)
{
	for (size_t i = 0; i < size; ++i)
	{
		if (!pull(*content++))
			// reached end of stream, return size read so far
			return i;
	}
	return size;
}

int InputBuffer::gets(char* str, size_t max, char end)
{
	size_t size = 0;
	while (size < max - 1)
	{
		char value;
		if (pull(value))
		{
			*str++ = value;
			++size;
			if (value == end)
				break;
		} 
		else if (!_blocking)
			break;
	}
	*str = 0;
	return ++size;
}

void InputBuffer::scan(char* str, size_t max)
{
	while (max > 0)
	{
		char value;
		if (!pull(value) or isspace(value))
			break;
	}
	*str = 0;
}
