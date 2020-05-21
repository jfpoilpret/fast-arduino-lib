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
#ifndef INITIALIZER_LIST_HH
#define INITIALIZER_LIST_HH

#pragma GCC visibility push(default)

namespace std
{
	template<class T> class initializer_list
	{
	private:
		const T* array_ = nullptr;
		size_t len_ = 0;

		// The compiler can call a private constructor.
		constexpr initializer_list(const T* array, size_t len) : array_{array}, len_{len} {}

	public:
		constexpr initializer_list() = default;

		constexpr size_t size() const
		{
			return len_;
		}

		// First element.
		constexpr const T* begin() const
		{
			return array_;
		}

		// One past the last element.
		constexpr const T* end() const
		{
			return begin() + size();
		}
	};
}

#pragma GCC visibility pop

#endif /* INITIALIZER_LIST_HH */
/// @endcond
