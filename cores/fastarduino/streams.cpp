#include <ctype.h>
#include "streams.hh"

char get(InputBuffer& in)
{
	return ::pull<char>(in);
}

char* get(InputBuffer& in, char* content, size_t size)
{
	char* current = content;
	for (size_t i = 0; i < size; ++i)
		*current++ = ::pull(in);
	return content;
}

int gets(InputBuffer& in, char* str, size_t max, char end)
{
	size_t size = 0;
	while (size < max - 1)
	{
		char value = ::pull(in);
		*str++ = value;
		++size;
		if (value == end)
			break;
	}
	*str = 0;
	return size;
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
