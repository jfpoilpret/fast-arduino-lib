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

/**
 * @file 
 * General utilities API that have broad application in programs.
 */
#ifndef UTILITIES_HH
#define UTILITIES_HH

#include "defines.h"
#include "boards/board.h"
#include <util/atomic.h>

/**
 * Make a block of code a critical section (not interruptible).
 * Example:
 * @code
 * // value may be initialized through an ISR
 * volatile int value;
 * 
 * void f()
 * {
 *     synchronized
 *     {
 *         // This code must not be interrupted (reading an int is not atomic on AVR)
 *         if (value > 1000)
 *             value = 0;
 *     }
 * }
 * @endcode
 */
#define synchronized _Pragma("GCC diagnostic ignored \"-Wreturn-type\"") ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

/**
 * Contains all generic utility methods.
 */
namespace utils
{
	/**
	 * Constrain @p value to be greater than or equal to @p min and lower than 
	 * or equal to @p max
	 * @tparam T the type of @p value (must be comparable: int, float...)
	 * @param value the value to constrain
	 * @param min the minimum allowed value
	 * @param max the maximum allowed value
	 * @return the value constrained to be between @p min and @p max
	 */
	template<typename T> constexpr T constrain(T value, T min, T max)
	{
		if (value < min)
			return min;
		else if (value > max)
			return max;
		else
			return value;
	}

	/**
	 * Linearly transform @p value from range [@p input_min ; @p input_max] to
	 * range [@p output_min ; @p output_max].
	 * Note that the transformed value is not explicitly constrained to range 
	 * [@p output_min ; @p output_max], hence if you want it to be with that range, 
	 * you should also use `constrain()` on the returned value.
	 * 
	 * @tparam T0 the type of the target range and value
	 * @tparam T1 the type of the source range and value
	 * @param value the value to transform
	 * @param input_min the minimum value of the input range
	 * @param input_max the maximum value of the input range
	 * @param output_min the minimum value of the output range
	 * @param output_max the maximum value of the output range
	 * @return transformed @p value
	 */
	template<typename TI, typename TO>
	constexpr TO map(TI value, TI input_min, TI input_max, TO output_min, TO output_max)
	{
		return output_min + (value - input_min) * (output_max - output_min) / (input_max - input_min);
	}

	// NOTE: we use prefixes that can be covered in a uint32
	/**
	 * Common prefixes for measurement units. Used by `map_raw_to_physical()` and
	 * `map_physical_to_raw()` for units conversion.
	 * To avoid large arithmetic calculation, we limit these prefixes to power
	 * of 10 that can hold within 32 bits; this is why TERA or PICO are absent.
	 */
	enum class UnitPrefix : int8_t
	{
		GIGA = 9,
		MEGA = 6,
		KILO = 3,
		HECTO = 2,
		DECA = 1,
		NONE = 0,
		DECI = -1,
		CENTI = -2,
		MILLI = -3,
		MICRO = -6,
		NANO = -9,
	};

	/**
	 * Calculate a power of 10 at compile-time, provided that @p n is a constant
	 * at call time. This is to avoid dragging huge mathematics libraries if 
	 * this can be avoided.
	 * @param n the power of exponent to apply to 10; if negative, then its absolute
	 * value is used instead.
	 * @return 10 ^ |@p n|
	 */
	constexpr uint32_t power_of_10(int8_t n)
	{
		if (n > 0)
			return 10UL * power_of_10(n - 1);
		else if (n < 0)
			return power_of_10(-n);
		else
			return 1;
	}

	/**
	 * Convert the raw @p value, obtained from an electronics device, using
	 * @p precision_bit number of bits (that defines the input range) into a
	 * physical measure for which @p range defines the complete output range for
	 * such value, adjusted according to the unit @p prefix that we want in the
	 * resulting measure.
	 * This is useful when you need to display real measurement from raw values
	 * provided by a sensor.
	 * Note however, that in most cases, this method will be evaluated at runtime
	 * only, and thus will drag all arithmetic computation libraries.
	 * If you need to know the physical measure from a raw value, only to compare
	 * it against some constant physical value, then it is preferable to convert
	 * the latter, with `map_raw_to_physical()` which will be evaluated at compile
	 * time, and only compare raw values in your program: that will help decrease 
	 * code size and improve code speed (no runtime conversions needed).
	 * 
	 * @code
	 * // This sample code use MPU6050 (accelerometer-gyroscope) support
	 * using namespace devices::magneto;
	 * using namespace utils;
	 * // These are the accel and gyro ranges used in this code
	 * static constexpr const GyroRange GYRO_RANGE = GyroRange::RANGE_250;
	 * static constexpr const AccelRange ACCEL_RANGE = AccelRange::RANGE_2G;
	 * 
	 * // This function converts a raw gyro axis measure into centi-degrees per second
	 * // so that it can be displayed to an LCD.
	 * inline int16_t gyro(int16_t value)
	 * {
	 *     // GYRO_RANGE_DPS calculates the gyro maximum range (in degrees per second)
	 *     // 15 is the number of bits of precision (on positive range only).
	 *     return map_raw_to_physical(value, UnitPrefix::CENTI, GYRO_RANGE_DPS(GYRO_RANGE), 15);
	 * }
	 * 
	 * // This function converts a raw accelerometer axis measure into milli-g
	 * // so that it can be displayed to an LCD.
	 * inline int16_t accel(int16_t value)
	 * {
	 *     // ACCEL_RANGE_G calculates the accel maximum range (in g)
	 *     // 15 is the number of bits of precision (on positive range only).
	 *     return map_raw_to_physical(value, UnitPrefix::MILLI, ACCEL_RANGE_G(ACCEL_RANGE), 15);
	 * }
	 * @endcode
	 * 
	 * @param value the raw value to convert
	 * @param prefix the unit scale prefix to use to compute the physical value
	 * @param range the physical measure matching the maximum raw value
	 * @param precision_bits the number of significant bits of the raw value; only
	 * positive values are accounted, hence if a raw measure can be any value in
	 * [-32768;+32767], then @p precision_bits is `15`.
	 * @return the physical value calculated from @p value, scaled according to
	 * @p prefix
	 * @sa map_physical_to_raw()
	 */
	constexpr int16_t map_raw_to_physical(int16_t value, UnitPrefix prefix, int16_t range, uint8_t precision_bits)
	{
		// Here we approximate the calculation by using 2^n instead of (2^n - 1) as input range
		const int8_t prefix_value = int8_t(prefix);
		if (prefix_value > 0)
			return (int32_t(value) * int32_t(range) / int32_t(power_of_10(prefix_value))) >> precision_bits;
		else
			return (int32_t(value) * int32_t(range) * int32_t(power_of_10(prefix_value))) >> precision_bits;
	}

	/// @cond notdocumented
	// This is intermediate function used by map_physical_to_raw() but without overflow issue (int32 only)
	constexpr int32_t map_physical_to_raw_(int32_t value, int8_t prefix, int32_t range, uint8_t precision_bits)
	{
		// Here we approximate the calculation by using 2^n instead of (2^n - 1) as input range
		if (prefix >= 0)
			return (value << precision_bits) * int32_t(power_of_10(prefix)) / range;
		else
			return (value << precision_bits) / int32_t(power_of_10(prefix)) / range;
	}
	/// @endcond

	/**
	 * Convert an absolute physical @p value, expressed in some given measurement 
	 * unit, scaled with @p prefix, into a raw measurement as if obtained
	 * from a electronics device, using @p precision_bit number of bits 
	 * (that defines the device raw measure range); for this device, physical 
	 * measures are within @p range.
	 * Computations done by this method will be performed at compile-time as long
	 * as all provided arguments are constants; this is important as this will
	 * help optimize code size and execution time.
	 * This is useful when you want to compare physical values against meaningful
	 * limits, and perform actions based on these comparisons; instead of
	 * always converting measured raw values into physical ones and then compare
	 * with a physical limit, you do the opposite, compare the measured raw values
	 * against the raw limits (converted, at compile-time, from physical limits
	 * constants).
	 * 
	 * @code
	 * // This sample code use MPU6050 (accelerometer-gyroscope) support
	 * using namespace devices::magneto;
	 * using namespace utils;
	 * // These is the accelerometer range used in this code
	 * static constexpr const AccelRange ACCEL_RANGE = AccelRange::RANGE_2G;
	 * static constexpr const uint16_t ACCEL_RANGE_IN_G = ACCEL_RANGE_G(ACCEL_RANGE);
	 * // These are specific threshold values for acceleration
	 * static constexpr const int16_t ACCEL_1 = map_physical_to_raw(500, UnitPrefix::MILLI, ACCEL_RANGE_IN_G, 15);
	 * static constexpr const int16_t ACCEL_2 = map_physical_to_raw(1000, UnitPrefix::MILLI, ACCEL_RANGE_IN_G, 15);
	 * 
	 * // This function performs an action based on current accelerometer value.
	 * void check_accel(int16_t raw)
	 * {
	 *     // We consider accelerations the same whatever the sign
	 *     if (raw < 0) raw = -raw;
	 *     if (raw < ACCEL_1)
	 *         act_when_low_accel();
	 *     else if (raw < ACCEL_2)
	 *         act_when_mid_accel();
	 *     else if (raw < ACCEL_2)
	 *         act_when_high_accel();
	 * }
	 * @endcode
	 * 
	 * @param value the physical value to convert
	 * @param prefix the unit scale prefix in which @p value is expressed
	 * @param range the physical measure matching the maximum raw value
	 * @param precision_bits the number of significant bits of the raw value; only
	 * positive values are accounted, hence if a raw measure can be any value in
	 * [-32768;+32767], then @p precision_bits is `15`.
	 * @return the raw value calculated from @p value, as if directly returned by
	 * the device sensor
	 * @sa map_raw_to_physical()
	 */
	constexpr int16_t map_physical_to_raw(int16_t value, UnitPrefix prefix, int16_t range, uint8_t precision_bits)
	{
		// We first perform calculation as int32
		const int32_t output = map_physical_to_raw_(value, int8_t(prefix), range, precision_bits);
		//Then we deal with boundary cases before conversion to int16
		if (output > INT16_MAX)
			return INT16_MAX;
		else if (output <= INT16_MIN)
			return INT16_MIN;
		else
			return output;
	}

	/**
	 * Extract the low order byte of a 16-bits word.
	 */
	constexpr uint8_t low_byte(uint16_t word)
	{
		return uint8_t(word & 0xFF);
	}

	/**
	 * Extract the high order byte of a 16-bits word.
	 */
	constexpr uint8_t high_byte(uint16_t word)
	{
		return uint8_t(word >> 8);
	}

	/**
	 * Convert 2 bytes into an unsigned int.
	 * @param high the high byte
	 * @param low the low byte
	 * @return the unsigned integer computed from @p high and @p low
	 */
	constexpr uint16_t as_uint16_t(uint8_t high, uint8_t low)
	{
		return uint16_t(uint16_t(high) << 8) | uint16_t(low);
	}

	/**
	 * Replace @p value by @p default_value if not "true" (also known as "Elvis 
	 * operator").
	 * @param value the value to check
	 * @param default_value the value to replace 0
	 * @return @p value if @p value is not false (or 0) otherwise @p default_value
	 */
	template<typename T> constexpr T is_zero(T value, T default_value)
	{
		return (value ? value : default_value);
	}

	/**
	 * Common utility to force a part of the value of a register, designated by a 
	 * bit mask.
	 * @tparam T the type of values to handle
	 * @param reg the value to change  part of
	 * @param mask the bit mask indicating which bits shall change
	 * @param value the new value for @p reg
	 */
	template<typename T> void set_mask(volatile T& reg, T mask, T value)
	{
		reg = (reg & ~mask) | (value & mask);
	}

	/**
	 * Common utility to check if 2 values are equal according to a mask.
	 * @tparam T the type of values to compare
	 * @param actual the actual value to compare
	 * @param mask the bit mask indicating which bits shall change
	 * @param expected the expected value
	 */
	template<typename T> constexpr bool is_mask_equal(T actual, T mask, T expected)
	{
		return (actual & mask) == (expected & mask);
	}

	/**
	 * Convert Binary-coded decimal byte (each nibble is a digit from 0 to 9) into
	 * a natural byte.
	 * @param bcd the input using BCD format
	 * @return @p bcd converted to natural integer
	 */
	inline uint8_t bcd_to_binary(uint8_t bcd)
	{
		uint8_t tens = bcd >> 4;
		// We avoid tens * 10 to avoid adding library for multiplication
		return (tens * 8) + (tens * 2) + (bcd & 0x0F);
	}

	/**
	 * Convert a natural integers to a BCD byte (2 digits).
	 * Behavior is undefined if @p binary `>99`.
	 * @param binary the natural integer value to convert to BCD; must be in
	 * range [0..99]
	 * @return @p binary converted to BCD
	 */
	inline uint8_t binary_to_bcd(uint8_t binary)
	{
		uint8_t bcd = 0;
		while (binary >= 10)
		{
			bcd += 0x10;
			binary -= 10;
		}
		return bcd + binary;
	}

	/**
	 * Swap 2 bytes of a 2-bytes integer. Useful to convert from big-endian to 
	 * small-endian (AVR).
	 * @param value value to convert in place (reference)
	 */
	inline void swap_bytes(uint16_t& value)
	{
		value = (value >> 8) | (value << 8);
	}

	/**
	 * Swap 2 bytes of a 2-bytes integer. Useful to convert from big-endian to 
	 * small-endian (AVR).
	 * @param value value to convert in place (reference)
	 */
	inline void swap_bytes(int16_t& value)
	{
		swap_bytes((uint16_t&) value);
	}

	/// @cond notdocumented
	template<typename T> union ToUint8
	{
		static_assert(sizeof(T) == 1, "T must be a one-byte size type");
		explicit ToUint8(T value) : value(value) {}
		T value;
		uint8_t as_uint8;
	};

	template<typename T> union ToArray
	{
		explicit ToArray(const T& value): value{value} {}
		T value;
		uint8_t as_array[sizeof(T)];
	};
	/// @endcond

	/**
	 * Cast a one byte long bit-fields struct into a byte.
	 * Useful when dealing with devices registers (bytes) where each bit has a
	 * specific maening which you want to clarify through a bitfield struct.
	 * @param input the bit field struct value to convert
	 * @return @p input casted as a byte
	 */
	template<typename T> constexpr uint8_t as_uint8_t(T input)
	{
		return ToUint8<T>(input).as_uint8;
	}

	/**
	 * Cast an instance of type @p T to an array of `uint8_t` of the size of @p T.
	 * @tparam T the type of @p input
	 * @param input a constant reference to the instance to cast to an array
	 * @param output a pointer to an array of the size of @p T
	 */
	template<typename T, typename U = uint8_t[sizeof(T)]>
	constexpr void as_array(const T& input, U output)
	{
		memcpy(output, ToArray<T>(input).as_array, sizeof(T));
	} 

	/**
	 * Calculate the count to pass to `delay1()` in order to reach @p time_us 
	 * microseconds delay. Calculation is performed at compile-time, provided
	 * that @p time_us is a constant when the method is called.
	 * This is to avoid dragging huge mathematics libraries if it can be avoided.
	 * @param time_us the time to reach through `delay1()` AVR function
	 * @return the count to pass to `delay1()` AVR function
	 */
	constexpr uint8_t calculate_delay1_count(float time_us)
	{
		return uint8_t(INST_PER_US / 3.0 * time_us);
	}

	/**
	 * Calculate the number of `1` bits in a byte. Calculation is performed at 
	 * compile-time, provided that @p mask is a constant when the method is called.
	 * @param mask the byte which you want to count the number of `1` bits
	 */
	constexpr uint8_t num_bits(uint8_t mask, uint8_t num = 0)
	{
		if (mask == 0)
			return num;
		else if (mask & 1)
			return num_bits(mask >> 1, num + 1);
		else
			return num_bits(mask >> 1, num);
	}
}

#endif /* UTILITIES_HH */
/// @endcond
