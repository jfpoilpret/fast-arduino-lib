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
#ifndef ITERATOR_HH
#define ITERATOR_HH

#include <stddef.h>
#include <stdint.h>
#include "initializer_list.h"

namespace utils
{
	//TODO DOC
	template<typename T> class range
	{
	public:
		range(const T* begin, const T* end) : begin_{begin}, end_{end} {}
		range(const T* begin, uint8_t size) : begin_{begin}, end_{begin + size} {}
		range(std::initializer_list<T> list) : begin_{list.begin()}, end_{list.end()} {}
		template<uint8_t SIZE> range(T (&content)[SIZE]) : begin_{content}, end_{content + SIZE} {}

		const T* begin() const
		{
			return begin_;
		}
		const T* end() const
		{
			return end_;
		}

		uint8_t size() const
		{
			return end_ - begin_;
		}

	private:
		const T* const begin_;
		const T* const end_;
	};
}

#endif /* ITERATOR_HH */
/// @endcond
