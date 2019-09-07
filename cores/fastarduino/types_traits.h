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

/**
 * Defines traits and utility methods for standard types, like `uint16_t`.
 * Note that this namespace is used internally by FastArduino API and will generally
 * not be useful to usual programs. It is however made part of the core API because
 * it might be useful in some occasions.
 */
namespace types_traits
{
	/**
	 * This trait allows static checks (at compile-time) of properties of various
	 * types.
	 * Currently, this is limited to a few properties related to integer types,
	 * but this may be enhanced with more properties in the future, when the needs
	 * occur.
	 * @tparam T the type for which we want to know the traits
	 */
	template<typename T> struct Type_trait
	{
		/** Indicates if `T` is an integer type. */
		static constexpr bool IS_INT = false;
		/** Indicates if `T` is a signed integer type. */
		static constexpr bool IS_SIGNED = false;
		/** Indicates the size in bytes of `T`. */
		static constexpr size_t SIZE = 0;
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

	/**
	 * Check if a given type is `uint8_t` or `uint16_t`.
	 * This is a `constexpr` template function, hence it can be used in
	 * compile-time checks with `static_assert()`.
	 */
	template<typename T> static constexpr bool is_uint8_or_uint16()
	{
		using TRAIT = Type_trait<T>;
		return TRAIT::IS_INT && (TRAIT::SIZE <= sizeof(uint16_t)) && (!TRAIT::IS_SIGNED);
	}

	/**
	 * Utility class that checks, at compile-time, that type @p T is a subtype of
	 * type @p B.
	 * This is inspired from https://stackoverflow.com/a/3178315.
	 * Trying to instantiate this type when T is not a subtype of B will fail
	 * compilation.
	 */
	template<class T, class B> struct derives_from
	{
		/// @cond notdocumented
		static void constraints(T* p)
		{
			UNUSED B* pb = p;
		}
		constexpr derives_from()
		{
			UNUSED void (*p)(T*) = constraints;
		}
		/// @endcond
	};
}

#endif /* TYPES_TRAITS_HH */
/// @endcond
