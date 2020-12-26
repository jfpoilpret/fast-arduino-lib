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

namespace containers
{
	/**
	 * Container that encapsulates a fixed size array.
	 * This can be used as would be a C-style array (same operators) without any
	 * impact on performance.
	 * This also allows, for instance, to use a braced-list as a way to initialiaze or
	 * replace the values in this array.
	 * @tparam T_ the type of elements in this array
	 * @tparam N_ the number of elements in this array
	 */
	template<typename T_, uint8_t N_>
	class array
	{
	public:
		/** The type of elements held by this array. */
		using T = T_;
		/** The number of elements in this array. */
		static constexpr uint8_t N = N_;
		/** The reference type to `T`. */
		using TREF = T_&;
		/** The const reference type to `T`. */
		using CTREF = const T_&;
		/** The pointer type to `T`. */
		using TPTR = T_*;
		/** The const pointer type to `T`. */
		using CTPTR = const T_*;

		/**
		 * Create a default array of @p N elements of type @p T.
		 */
		array() = default;

		/**
		 * Convert a C-style array to an array.
		 */
		array(T buffer[N])
		{
			T* dst = buffer_;
			const T* src = buffer;
			for (uint8_t i = 0; i < N; ++i)
				*dst++ = *src++;
		}

		/**
		 * Create an array from a braced-list of elements.
		 * If @p list has less than @p N elements, all remaining elements from
		 * this array will be reset to their default.
		 */
		array(std::initializer_list<T> list)
		{
			T* dst = buffer_;
			auto src = list.begin();
			for (uint8_t i = 0; i < N; ++i)
			{
				if (src != list.end())
					*dst++ = *src++;
				else
					*dst++ = T{};
			}
		}

		/**
		 * Copy @p that array.
		 */
		array(const array<T, N>& that)
		{
			T* dst = buffer_;
			const T* src = that.buffer_;
			for (uint8_t i = 0; i < N; ++i)
				*dst++ = *src++;
		}

		/**
		 * Overwrite every element of this array with every element of @p buffer.
		 */
		array<T, N>& operator=(const T buffer[N])
		{
			T* dst = buffer_;
			const T* src = buffer;
			for (uint8_t i = 0; i < N; ++i)
				*dst++ = *src++;
			return *this;
		}

		/**
		 * Overwrite every element of this array with every element of @p list.
		 * If @p list has less than @p N elements, all remaining elements from
		 * this array will be reset to their default.
		 */
		array<T, N>& operator=(std::initializer_list<T> list)
		{
			T* dst = buffer_;
			auto src = list.begin();
			for (uint8_t i = 0; i < N; ++i)
			{
				if (src != list.end())
					*dst++ = *src++;
				else
					*dst++ = T{};
			}
		}

		/**
		 * Overwrite every element of this array with every element of @p that.
		 */
		array<T, N>& operator=(const array<T, N>& that)
		{
			T* dst = buffer_;
			const T* src = that.buffer_;
			for (uint8_t i = 0; i < N; ++i)
				*dst++ = *src++;
			return *this;
		}

		/**
		 * Get a const pointer to the first element of this array.
		 */
		CTPTR data() const
		{
			return buffer_;
		}

		/**
		 * Get a pointer to the first element of this array.
		 */
		TPTR data()
		{
			return buffer_;
		}

		/**
		 * Get the number of elements in this array.
		 * @sa N
		 */
		constexpr uint8_t size() const
		{
			return N;
		}

		/**
		 * Get a const reference to the element at @p index in this array.
		 * This can be used as an rvalue.
		 * @warning There is no control of boundaries, hence the caller must
		 * ensure that @p index is strictly less than @p N, otherwise behavior is
		 * undefined.
		 */
		CTREF operator[](uint8_t index) const
		{
			return buffer_[index];
		}

		/**
		 * Get a reference to the element at @p index in this array.
		 * This can be used as an lvalue.
		 * @warning There is no control of boundaries, hence the caller must
		 * ensure that @p index is strictly less than @p N, otherwise behavior is
		 * undefined.
		 */
		TREF operator[](uint8_t index)
		{
			return buffer_[index];
		}

		/** Return an iterator to the first element of this array. */
		TPTR begin()
		{
			return buffer_;
		}

		/** Return an iterator to the last element of this array. */
		TPTR end()
		{
			return buffer_ + N;
		}

		/** Return a const iterator to the first element of this array. */
		CTPTR begin() const
		{
			return buffer_;
		}

		/** Return a const iterator to the last element of this array. */
		CTPTR end() const
		{
			return buffer_ + N;
		}

		/**
		 * Replace @p NN elements of this array, starting at @p index element, with
		 * elements from @p buffer.
		 */
		template<uint8_t NN>
		void set(uint8_t index, const T (&buffer)[NN])
		{
			if (index >= N) return;
			const uint8_t nn = ((N - index) < NN) ? (N - index) : NN;
			T* dst = &buffer_[index];
			const T* src = buffer;
			for (uint8_t i = 0; i < nn; ++i)
				*dst++ = *src++;
		}

		/**
		 * Replace @p NN elements of this array, starting at @p index element, with
		 * elements from @p buffer.
		 */
		template<uint8_t NN>
		void set(uint8_t index, const array<T, NN>& buffer)
		{
			if (index >= N) return;
			const uint8_t nn = ((N - index) < NN) ? (N - index) : NN;
			T* dst = &buffer_[index];
			const T* src = buffer.data();
			for (uint8_t i = 0; i < nn; ++i)
				*dst++ = *src++;
		}

	private:
		T buffer_[N] = {};
	};
}
#endif /* ARRAY_HH */
/// @endcond
