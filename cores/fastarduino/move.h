//   Copyright 2016-2023 Jean-Francois Poilpret
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
#ifndef MOVE_HH
#define MOVE_HH

#include "types_traits.h"

namespace std
{
	/**
	 * Obtain an "rvalue" reference.
	 * This function is used to indicate that the object @p t can be "moved from";
	 * this will enforce usage of move-constructor or move-assignment operator
	 * of the lvalue in the code snipept below:
	 * @code
	 * T right = ...;
	 * T left = std::move(right);
	 * ...
	 * right = std::move(left);
	 * @endcode
	 * In that snippet resources initially held by `right` variable are first
	 * transferred to `left` variable through `T` move-constructor; then resources
	 * held by `left` are transferred back to `right` variable through `T` 
	 * move-assignment operator.
	 */
	template<typename T>
	constexpr typename types_traits::remove_reference<T>::type&& move(T&& t)
	{
		return static_cast<typename types_traits::remove_reference<T>::type&&>(t);
	}
}

#endif /* MOVE_HH */
/// @endcond
