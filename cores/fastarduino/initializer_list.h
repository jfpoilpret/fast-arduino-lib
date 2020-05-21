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

#include <stddef.h>

/**
 * Similar to standard C++ `std` namespace, this namespace is used by FastArduino
 * library to implement various types of the standard C++ library when this is useful.
 * For these standard types, FastArduino strives to keep the original API as much
 * as possible, but be aware that FastArduino API may not have 100% fidelity to
 * C++ standard API.
 */
namespace std
{
	/**
	 * C++ standard support for "initializer_list".
	 * This creates a temporary array in C++11 list-initialization and then 
	 * references it.
	 * An initializer_list instance is automatically constructed when a 
	 * braced-init-list is used to list-initialize an object, where the 
	 * corresponding constructor accepts an initializer_list parameter.
	 * 
	 * Using an initializer_ist argument is done through a standard C++ iterator loop:
	 * @code
	 * void f(initializer_list<int> list) {
	 *     for (auto x : list)
	 *         cout << *x << endl;
	 * }
	 * @endcode
	 * where the iterator type is simply `const T*`.
	 * That code could also be written in a more traditional and more verbose way:
	 * @code
	 * void f(initializer_list<int> list) {
	 *     for (const int* x = list.begin(); x != list.end(); ++x)
	 *         cout << *x << endl;
	 * }
	 * @endcode
	 * 
	 * @tparam T the type of objects in this initializer_list
	 */
	template<class T> class initializer_list
	{
	private:
		const T* array_ = nullptr;
		size_t len_ = 0;

		// The compiler can (and will) call a private constructor.
		// This constructor will get called when the compiler encounters a braced
		// list expression with elements of type T, like in f({t1, t2, t3}) and
		// an initializer_list<T> is expected by f function.
		constexpr initializer_list(const T* array, size_t len) : array_{array}, len_{len} {}

	public:
		/** Create an empty initializer list. */
		constexpr initializer_list() = default;

		/** The size of this initializer_list.  */
		constexpr size_t size() const
		{
			return len_;
		}

		/** The first element of this initializer_list. */
		constexpr const T* begin() const
		{
			return array_;
		}

		/** One pas the last element of this initializer_list. */
		constexpr const T* end() const
		{
			return begin() + size();
		}
	};
}

#endif /* INITIALIZER_LIST_HH */
/// @endcond
