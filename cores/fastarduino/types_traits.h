//   Copyright 2016-2019 Jean-Francois Poilpret
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

/**
 * @file
 * Useful traits for common types.
 * These can be used in various template classes to ensure parameter types
 * are acceptable or not (with `static_assert()`).
 */
#ifndef TYPES_TRAITS_HH
#define TYPES_TRAITS_HH

#include <stddef.h>
#include <stdint.h>

//TODO document (not this is an API used internally and will not usually be needed in programs but you never know)
namespace types_traits
{
	template<typename T> struct Type_trait
	{
		static constexpr bool IS_INT = false;
		static constexpr bool IS_SIGNED = false;
		static constexpr size_t SIZE = 0;
		//TODO more to put here: pointer? reference? boolean? char? array?
	};

	/// @cond notdocumented
	template<typename T_, bool IS_SIGNED_> struct Type_trait_int
	{
		static constexpr bool IS_INT = true;
		static constexpr bool IS_SIGNED = IS_SIGNED_;
		static constexpr size_t SIZE = sizeof(T_);
	};

	template<> struct Type_trait<uint8_t> : Type_trait_int<uint8_t, false> {};
	template<> struct Type_trait<uint16_t> : Type_trait_int<uint16_t, false> {};
	template<> struct Type_trait<uint32_t> : Type_trait_int<uint32_t, false> {};
	template<> struct Type_trait<uint64_t> : Type_trait_int<uint64_t, false> {};
	template<> struct Type_trait<int8_t> : Type_trait_int<int8_t, true> {};
	template<> struct Type_trait<int16_t> : Type_trait_int<int16_t, true> {};
	template<> struct Type_trait<int32_t> : Type_trait_int<int32_t, true> {};
	template<> struct Type_trait<int64_t> : Type_trait_int<int64_t, true> {};
	/// @endcond

	template<typename T> static constexpr bool is_uint8_or_uint16()
	{
		using TRAIT = Type_trait<T>;
		return TRAIT::IS_INT && TRAIT::SIZE <= 2 && !TRAIT::IS_SIGNED;
	}
}

#endif /* TYPES_TRAITS_HH */
/// @endcond
