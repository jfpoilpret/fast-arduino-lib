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
