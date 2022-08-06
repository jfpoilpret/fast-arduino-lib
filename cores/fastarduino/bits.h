//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * Useful bits manipulation utilities.
 */
#ifndef BITS_HH
#define BITS_HH

#include <stdint.h>

/**
 * Defines utility methods for bits manipulation.
 * Most functions defined here are `constexpr`, which means that they can be 
 * evaluated at compile-time when constant arguments.
 * These functions are used to ensure proper casting of results to the required 
 * types, i.e. `uint8_t` or `uint16_t`.
 */
namespace bits
{
	/**
	 * Create a `uint8_t` bitmask for the given @p bit number.
	 * Concretely, this is simply the `1 << bit` expression.
	 * @param bit the bit number for which to produce a bitmask, should be between
	 * `0` and `7`.
	 */
	static constexpr uint8_t BV8(uint8_t bit)
	{
		return uint8_t(1 << bit);
	}

	/**
	 * Create a `uint8_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2)` expression.
	 * @param bit1 the first bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit2 the second bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 */
	static constexpr uint8_t BV8(uint8_t bit1, uint8_t bit2)
	{
		return uint8_t(BV8(bit1) | BV8(bit2));
	}

	/**
	 * Create a `uint8_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2) ...` expression.
	 * @param bit1 the first bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit2 the second bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit3 the third bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 */
	static constexpr uint8_t BV8(uint8_t bit1, uint8_t bit2, uint8_t bit3)
	{
		return uint8_t(BV8(bit1, bit2) | BV8(bit3));
	}

	/**
	 * Create a `uint8_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2) ...` expression.
	 * @param bit1 the first bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit2 the second bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit3 the third bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit4 the fourth bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 */
	static constexpr uint8_t BV8(uint8_t bit1, uint8_t bit2, uint8_t bit3, uint8_t bit4)
	{
		return uint8_t(BV8(bit1, bit2) | BV8(bit3, bit4));
	}

	/**
	 * Create a `uint8_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2) ...` expression.
	 * @param bit1 the first bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit2 the second bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit3 the third bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit4 the fourth bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit5 the fifth bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 */
	static constexpr uint8_t BV8(uint8_t bit1, uint8_t bit2, uint8_t bit3, uint8_t bit4, uint8_t bit5)
	{
		return uint8_t(BV8(bit1, bit2, bit3) | BV8(bit4, bit5));
	}

	/**
	 * Create a `uint8_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2) ...` expression.
	 * @param bit1 the first bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit2 the second bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit3 the third bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit4 the fourth bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit5 the fifth bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 * @param bit6 the sixth bit number for which to produce a bitmask, should 
	 * be between `0` and `7`.
	 */
	static constexpr uint8_t BV8(uint8_t bit1, uint8_t bit2, uint8_t bit3, uint8_t bit4, uint8_t bit5, uint8_t bit6)
	{
		return uint8_t(BV8(bit1, bit2, bit3) | BV8(bit4, bit5, bit6));
	}

	/**
	 * Create a `uint8_t` inverted bitmask for the given @p bit number.
	 * Concretely, this is simply the `~(1 << bit)` expression.
	 * @param bit the bit number for which to produce a bitmask, should be between
	 * `0` and `7`.
	 */
	static constexpr uint8_t CBV8(uint8_t bit)
	{
		return uint8_t(~BV8(bit));
	}

	/**
	 * Create a `uint16_t` bitmask for the given @p bit number.
	 * Concretely, this is simply the `1 << bit` expression.
	 * @param bit the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 */
	static constexpr uint16_t BV16(uint8_t bit)
	{
		return uint16_t(1 << bit);
	}

	/**
	 * Create a `uint16_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2)` expression.
	 * @param bit1 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit2 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 */
	static constexpr uint16_t BV16(uint8_t bit1, uint8_t bit2)
	{
		return uint16_t(BV16(bit1) | BV16(bit2));
	}

	/**
	 * Create a `uint16_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2) ...` expression.
	 * @param bit1 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit2 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit3 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 */
	static constexpr uint16_t BV16(uint8_t bit1, uint8_t bit2, uint8_t bit3)
	{
		return uint16_t(BV16(bit1, bit2) | BV16(bit3));
	}

	/**
	 * Create a `uint16_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2) ...` expression.
	 * @param bit1 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit2 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit3 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit4 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 */
	static constexpr uint16_t BV16(uint8_t bit1, uint8_t bit2, uint8_t bit3, uint8_t bit4)
	{
		return uint16_t(BV16(bit1, bit2) | BV16(bit3, bit4));
	}

	/**
	 * Create a `uint16_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2) ...` expression.
	 * @param bit1 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit2 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit3 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit4 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit5 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 */
	static constexpr uint16_t BV16(uint8_t bit1, uint8_t bit2, uint8_t bit3, uint8_t bit4, uint8_t bit5)
	{
		return uint16_t(BV16(bit1, bit2, bit3) | BV16(bit4, bit5));
	}

	/**
	 * Create a `uint16_t` bitmask for the given bits numbers.
	 * Concretely, this is simply the `(1 << bit1) | (1 << bit2) ...` expression.
	 * @param bit1 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit2 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit3 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit4 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit5 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 * @param bit6 the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 */
	static constexpr uint16_t BV16(uint8_t bit1, uint8_t bit2, uint8_t bit3, uint8_t bit4, uint8_t bit5, uint8_t bit6)
	{
		return uint16_t(BV16(bit1, bit2, bit3) | BV16(bit4, bit5, bit6));
	}

	/**
	 * Create a `uint16_t` inverted bitmask for the given @p bit number.
	 * Concretely, this is simply the `~(1 << bit)` expression.
	 * @param bit the bit number for which to produce a bitmask, should be between
	 * `0` and `15`.
	 */
	static constexpr uint16_t CBV16(uint8_t bit)
	{
		return uint16_t(~BV16(bit));
	}

	/**
	 * Return the `uint8_t` 2-complement of a byte.
	 * Concretely, this is simply the `~value` expression.
	 */
	static constexpr uint8_t COMPL(uint8_t value)
	{
		return uint8_t(~value);
	}

	/**
	 * Extract the low byte (aka Least Significant Byte, LSB) of a `uint16_t` value.
	 */
	static constexpr uint8_t LOW_BYTE(uint16_t value)
	{
		return uint8_t(value & uint16_t(UINT8_MAX));
	}

	/**
	 * Extract the high byte (aka Most Significant Byte, MSB) of a `uint16_t` value.
	 */
	static constexpr uint8_t HIGH_BYTE(uint16_t value)
	{
		return uint8_t(value >> 8);
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands.
	 * This method exists to ensure proper result cast to `uint8_t`.
	 * Concretely, this is simply the `val1 | val2` expression.
	 * @param val1 the first byte to be ORed.
	 * @param val2 the second byte to be ORed.
	 */
	static constexpr uint8_t OR8(uint8_t val1, uint8_t val2)
	{
		return uint8_t(val1 | val2);
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands.
	 * This method exists to ensure proper result cast to `uint8_t`.
	 * @param val1 the first byte to be ORed.
	 * @param val2 the second byte to be ORed.
	 * @param val3 the third byte to be ORed.
	 */
	static constexpr uint8_t OR8(uint8_t val1, uint8_t val2, uint8_t val3)
	{
		return uint8_t(val1 | OR8(val2, val3));
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands.
	 * This method exists to ensure proper result cast to `uint8_t`.
	 * @param val1 the first byte to be ORed.
	 * @param val2 the second byte to be ORed.
	 * @param val3 the third byte to be ORed.
	 * @param val4 the third byte to be ORed.
	 */
	static constexpr uint8_t OR8(uint8_t val1, uint8_t val2, uint8_t val3, uint8_t val4)
	{
		return uint8_t(OR8(val1, val2) | OR8(val3, val4));
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands.
	 * This method exists to ensure proper result cast to `uint8_t`.
	 * @param val1 the first byte to be ORed.
	 * @param val2 the second byte to be ORed.
	 * @param val3 the third byte to be ORed.
	 * @param val4 the third byte to be ORed.
	 * @param val5 the third byte to be ORed.
	 */
	static constexpr uint8_t OR8(uint8_t val1, uint8_t val2, uint8_t val3, uint8_t val4, uint8_t val5)
	{
		return uint8_t(val1 | OR8(val2, val3, val4, val5));
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands.
	 * This method exists to ensure proper result cast to `uint8_t`.
	 * @param val1 the first byte to be ORed.
	 * @param val2 the second byte to be ORed.
	 * @param val3 the third byte to be ORed.
	 * @param val4 the third byte to be ORed.
	 * @param val5 the third byte to be ORed.
	 * @param val6 the third byte to be ORed.
	 */
	static constexpr uint8_t OR8(uint8_t val1, uint8_t val2, uint8_t val3, uint8_t val4, uint8_t val5, uint8_t val6)
	{
		return uint8_t(OR8(val1, val2, val3) | OR8(val4, val5, val6));
	}

	/**
	 * return @p val if @p flag is `true`, otherwise `0`, as an `uint8_t`.
	 * Concretely, this is simply the `(flag ? val : 0U)` expression.
	 * This method exists to ensure proper result cast to `uint8_t`.
	 * @param flag the flag used to determine if @p val should be returned or `0`
	 * @param val the value returned if @p flag is `true`
	 */
	static constexpr uint8_t IF8(bool flag, uint8_t val)
	{
		return uint8_t(flag ? val : 0U);
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands,
	 * conditioned by `bool` flags.
	 * Concretely, this is simply the `(flag1 ? val1 : 0) | (flag2 ? val2 : 0)` expression.
	 * @param flag1 the flag used to determine if @p val1 should be used or `0`
	 * @param val1 the first byte to be ORed.
	 * @param flag2 the flag used to determine if @p val2 should be used or `0`
	 * @param val2 the second byte to be ORed.
	 */
	static constexpr uint8_t ORIF8(bool flag1, uint8_t val1, bool flag2, uint8_t val2)
	{
		return OR8(IF8(flag1, val1), IF8(flag2, val2));
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands,
	 * conditioned by `bool` flags.
	 * @param flag1 the flag used to determine if @p val1 should be used or `0`
	 * @param val1 the first byte to be ORed.
	 * @param flag2 the flag used to determine if @p val2 should be used or `0`
	 * @param val2 the second byte to be ORed.
	 * @param flag3 the flag used to determine if @p val3 should be used or `0`
	 * @param val3 the second byte to be ORed.
	 */
	static constexpr uint8_t ORIF8(bool flag1, uint8_t val1, bool flag2, uint8_t val2, bool flag3, uint8_t val3)
	{
		return OR8(IF8(flag1, val1), IF8(flag2, val2), IF8(flag3, val3));
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands,
	 * conditioned by `bool` flags.
	 * @param flag1 the flag used to determine if @p val1 should be used or `0`
	 * @param val1 the first byte to be ORed.
	 * @param flag2 the flag used to determine if @p val2 should be used or `0`
	 * @param val2 the second byte to be ORed.
	 * @param flag3 the flag used to determine if @p val3 should be used or `0`
	 * @param val3 the second byte to be ORed.
	 * @param flag4 the flag used to determine if @p val4 should be used or `0`
	 * @param val4 the second byte to be ORed.
	 */
	static constexpr uint8_t ORIF8(
		bool flag1, uint8_t val1, bool flag2, uint8_t val2, bool flag3, uint8_t val3, bool flag4, uint8_t val4)
	{
		return OR8(IF8(flag1, val1), IF8(flag2, val2), IF8(flag3, val3), IF8(flag4, val4));
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands,
	 * conditioned by `bool` flags.
	 * @param flag1 the flag used to determine if @p val1 should be used or `0`
	 * @param val1 the first byte to be ORed.
	 * @param flag2 the flag used to determine if @p val2 should be used or `0`
	 * @param val2 the second byte to be ORed.
	 * @param flag3 the flag used to determine if @p val3 should be used or `0`
	 * @param val3 the second byte to be ORed.
	 * @param flag4 the flag used to determine if @p val4 should be used or `0`
	 * @param val4 the second byte to be ORed.
	 * @param flag5 the flag used to determine if @p val5 should be used or `0`
	 * @param val5 the second byte to be ORed.
	 */
	static constexpr uint8_t ORIF8(bool flag1, uint8_t val1, bool flag2, uint8_t val2, bool flag3, uint8_t val3, 
		bool flag4, uint8_t val4, bool flag5, uint8_t val5)
	{
		return OR8(IF8(flag1, val1), IF8(flag2, val2), IF8(flag3, val3), IF8(flag4, val4), IF8(flag5, val5));
	}

	/**
	 * Create a `uint8_t` bitwise OR boolean operation between `uint8_t` operands,
	 * conditioned by `bool` flags.
	 * @param flag1 the flag used to determine if @p val1 should be used or `0`
	 * @param val1 the first byte to be ORed.
	 * @param flag2 the flag used to determine if @p val2 should be used or `0`
	 * @param val2 the second byte to be ORed.
	 * @param flag3 the flag used to determine if @p val3 should be used or `0`
	 * @param val3 the second byte to be ORed.
	 * @param flag4 the flag used to determine if @p val4 should be used or `0`
	 * @param val4 the second byte to be ORed.
	 * @param flag5 the flag used to determine if @p val5 should be used or `0`
	 * @param val5 the second byte to be ORed.
	 * @param flag6 the flag used to determine if @p val6 should be used or `0`
	 * @param val6 the second byte to be ORed.
	 */
	static constexpr uint8_t ORIF8(bool flag1, uint8_t val1, bool flag2, uint8_t val2, bool flag3, uint8_t val3, 
		bool flag4, uint8_t val4, bool flag5, uint8_t val5, bool flag6, uint8_t val6)
	{
		return OR8(IF8(flag1, val1), IF8(flag2, val2), IF8(flag3, val3), 
			IF8(flag4, val4), IF8(flag5, val5), IF8(flag6, val6));
	}
}

#endif /* BITS_HH */
/// @endcond
