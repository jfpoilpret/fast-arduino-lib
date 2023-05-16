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

/**
 * @file
 * Useful functors that can be used as template arguments, particularly in I2C
 * device utilities.
 */
#ifndef FUNCTORS_HH
#define FUNCTORS_HH

#include "utilities.h"
#include "types_traits.h"

/**
 * This namespace defines a few useful functors.
 * All functors defined here are classes with `operator()`.
 * Each functor also has the following available types defined:
 * - `ARG_TYPE` the type of its argument
 * - `RES_TYPE` the type of its result
 * 
 * These functors are heavily used in I2C devices, applied to Futures classes.
 */
namespace functor
{
	// Private class to hide internals used for public API implementation (declared friend)
	/// @cond notdocumented
	class functor_impl
	{
		// Change endianness of a single integer (T must be an integral type)
		template<typename T> class ChangeEndiannessOneImpl
		{
			using T_TRAIT = types_traits::Type_trait<T>;
			static_assert(T_TRAIT::IS_INT, "T must be an integral type");
			static_assert(T_TRAIT::SIZE > 1, "T must be at least 2 bytes long");
		public:
			using ARG_TYPE = T;
			using RES_TYPE = T;
			T operator()(const T& value) const
			{
				return utils::change_endianness(value);
			}
		};

		// Change endianness of a several integers embedded in another type
		template<typename T, typename TT> class ChangeEndiannessManyImpl
		{
			static_assert(sizeof(T) > sizeof(TT), "T must contain at least 2 TT");
			using TT_TRAIT = types_traits::Type_trait<TT>;
			static_assert(TT_TRAIT::IS_INT, "TT must be an integral type");
			static_assert(TT_TRAIT::SIZE > 1, "TT must be at least 2 bytes long");
		public:
			using ARG_TYPE = T;
			using RES_TYPE = T;
			T operator()(const T& value) const
			{
				constexpr uint8_t COUNT = sizeof(T) / sizeof(TT);
				T result = value;
				TT* ptr = reinterpret_cast<TT*>(&result);
				TT* last = ptr + COUNT;
				while (ptr < last)
					utils::swap_bytes(*ptr++);
				return result;
			}
		};

		// Friends
		template<typename T, typename TT> friend class ChangeEndianness;
	};
	/// @endcond
	
	/**
	 * Identity functor: always returns its argument, unchanged.
	 * Useful as a default argument to templates requiring a functor.
	 * @tparam T any type
	 */
	template<typename T> class Identity
	{
	public:
		/// @cond notdocumented
		using ARG_TYPE = T;
		using RES_TYPE = T;
		RES_TYPE operator()(const ARG_TYPE& value) const
		{
			return value;
		}
		/// @endcond
	};

	/**
	 * Constant functor: always returns a constant value.
	 * @tparam T any type which values are suitable as template arguments (e.g.
	 * integers, char, enum)
	 * @tparam VAL the constant value always returned by this functor
	 */
	template<typename T, T VAL> class Constant
	{
	public:
		/// @cond notdocumented
		using ARG_TYPE = T;
		using RES_TYPE = T;
		RES_TYPE operator()(UNUSED const ARG_TYPE& value) const
		{
			return VAL;
		}
		/// @endcond
	};

	/**
	 * Cast functor: returns its argument of type @p ARG casted to type @p RES.
	 * @tparam ARG the type of input argument
	 * @tparam RES the type of result; an implicit or explicit cast operator must
	 * exist from @p ARG to @p RES.
	 */
	template<typename ARG, typename RES> class Cast
	{
	public:
		/// @cond notdocumented
		using ARG_TYPE = ARG;
		using RES_TYPE = RES;
		RES_TYPE operator()(const ARG_TYPE& value) const
		{
			return (RES_TYPE) value;
		}
		/// @endcond
	};

	/**
	 * Composition functor: applies 2 functors one after each other.
	 * @tparam F1 last functor to apply (F1 o F2)
	 * @tparam F2 first functor to apply (F1 o F2)
	 */
	template<typename F1, typename F2> class Compose
	{
	public:
		/// @cond notdocumented
		using ARG_TYPE = typename F2::ARG_TYPE;
		using RES_TYPE = typename F1::RES_TYPE;
		RES_TYPE operator()(const ARG_TYPE& value) const
		{
			F1 f1;
			F2 f2;
			return f1(f2(value));
		}
		/// @endcond
	};

	/**
	 * Endianness change functor: will change from big to little or little to big
	 * endian format on integer types.
	 * It also supports arrays (or struct) of several integral values to change
	 * endianness for each of them.
	 * @tparam T the input type (can be integral or complex)
	 * @tparam TT the integral type embedded in type @p T
	 */
	template<typename T, typename TT = T> class ChangeEndianness
	{
	public:
		/// @cond notdocumented
		using ARG_TYPE = T;
		using RES_TYPE = T;
		T operator()(const T& value) const
		{
			return value;
		}
		/// @endcond
	};

	/// @cond notdocumented
	template<typename T> class ChangeEndianness<T, int16_t> : 
		public functor_impl::ChangeEndiannessManyImpl<T, int16_t>{};
	template<typename T> class ChangeEndianness<T, uint16_t> : 
		public functor_impl::ChangeEndiannessManyImpl<T, uint16_t>{};
	template<typename T> class ChangeEndianness<T, int32_t> : 
		public functor_impl::ChangeEndiannessManyImpl<T, int32_t>{};
	template<typename T> class ChangeEndianness<T, uint32_t> : 
		public functor_impl::ChangeEndiannessManyImpl<T, uint32_t>{};
	template<typename T> class ChangeEndianness<T, int64_t> : 
		public functor_impl::ChangeEndiannessManyImpl<T, int64_t>{};
	template<typename T> class ChangeEndianness<T, uint64_t> : 
		public functor_impl::ChangeEndiannessManyImpl<T, uint64_t>{};
	
	template<> class ChangeEndianness<int16_t, int16_t> : 
		public functor_impl::ChangeEndiannessOneImpl<int16_t>{};
	template<> class ChangeEndianness<uint16_t, uint16_t> : 
		public functor_impl::ChangeEndiannessOneImpl<uint16_t>{};
	template<> class ChangeEndianness<int32_t, int32_t> : 
		public functor_impl::ChangeEndiannessOneImpl<int32_t>{};
	template<> class ChangeEndianness<uint32_t, uint32_t> : 
		public functor_impl::ChangeEndiannessOneImpl<uint32_t>{};
	template<> class ChangeEndianness<int64_t, int64_t> : 
		public functor_impl::ChangeEndiannessOneImpl<int64_t>{};
	template<> class ChangeEndianness<uint64_t, uint64_t> : 
		public functor_impl::ChangeEndiannessOneImpl<uint64_t>{};
	/// @endcond

	/**
	 * Utility to instantiate and execute a functor from its type.
	 * @code
	 * using namespace functor;
	 * using Converter = Cast<char, uint8_t>;
	 * uint8_t x = Functor<Converter>::call('a');
	 * @endcode
	 */
	template<typename FUNCTOR> struct Functor
	{
		/// @cond notdocumented
		using ARG_TYPE = typename FUNCTOR::ARG_TYPE;
		using RES_TYPE = typename FUNCTOR::RES_TYPE;
		static RES_TYPE call(const ARG_TYPE& value)
		{
			return FUNCTOR{}(value);
		}
		/// @endcond
	};
}

#endif /* FUNCTORS_HH */
/// @endcond
