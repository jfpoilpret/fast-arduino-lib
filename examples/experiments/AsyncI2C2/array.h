//   Copyright 2016-2020 Jean-Francois Poilpret
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
#ifndef ARRAY_HH
#define ARRAY_HH

#include <stdint.h>
#include "initializer_list.h"

//TODO DOCS
//TODO namespace
template<typename T_, uint8_t N_>
class array
{
public:
	using T = T_;
	using TREF = T_&;
	using CTREF = const T_&;
	using TPTR = T_*;
	using CTPTR = const T_*;
	static constexpr uint8_t N = N_;

	array(T buffer[N])
	{
		memcpy(buffer_, buffer, N * sizeof(T));
	}
	array(std::initializer_list<T> list)
	{
		T* ptr = buffer_;
		for (auto i = list.begin(); i != list.end(); ++i)
			*ptr++ = *i;
	}
	
	array<T, N>& operator=(const T buffer[N])
	{
		memcpy(buffer_, buffer, N * sizeof(T));
		return *this;
	}

	array(const array<T, N>& that)
	{
		memcpy(buffer_, that.buffer_, N * sizeof(T));
	}
	array<T, N>& operator=(const array<T, N>& that)
	{
		memcpy(buffer_, that.buffer_, N * sizeof(T));
		return *this;
	}

	uint8_t size() const
	{
		return N;
	}
	CTREF operator[](uint8_t index) const
	{
		return buffer_[index];
	}
	TREF operator[](uint8_t index)
	{
		return buffer_[index];
	}
	operator CTPTR() const
	{
		return buffer_;
	}
	operator TPTR()
	{
		return buffer_;
	}

private:
	T buffer_[N] = {};
};

#endif /* ARRAY_HH */
/// @endcond
