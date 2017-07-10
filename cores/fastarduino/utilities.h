//   Copyright 2016-2017 Jean-Francois Poilpret
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

#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/io.h>
#include <util/atomic.h>

#ifndef UNUSED
#define UNUSED __attribute__((unused))
#endif

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif

#define synchronized \
_Pragma ("GCC diagnostic ignored \"-Wreturn-type\"") \
ATOMIC_BLOCK(ATOMIC_RESTORESTATE)

namespace utils
{
	template<typename T>
	constexpr T constrain(T value, T min, T max)
	{
		return value < min ? min : value > max ? max : value;
	}
	template<typename TI, typename TO>
	constexpr TO map(TI value, TI input_min, TI input_max, TO output_min, TO output_max)
	{
		return output_min + (value - input_min) * (output_max - output_min) / (input_max - input_min);
	}

	// NOTE: we use prefixes that can be covered in a uint32
	enum class UnitPrefix: int8_t
	{
		GIGA	= 9,
		MEGA	= 6,
		KILO	= 3,
		HECTO	= 2,
		DECA	= 1,
		NONE	= 0,
		DECI	= -1,
		CENTI	= -2,
		MILLI	= -3,
		MICRO	= -6,
		NANO	= -9,
	};
	
	constexpr uint32_t power_of_10(int8_t n)
	{
		return (n > 0 ? 10UL * power_of_10(n - 1) :
				n == 0 ? 1 :
				power_of_10(-n));
	}
	
	//TODO make it templatized on input type? output type?
	constexpr int16_t map_raw_to_physical(int16_t value, UnitPrefix prefix, int16_t range, uint8_t precision_bits)
	{
		// Here we approximate the calculation by using 2^n instead of (2^n - 1) as input range
		return (int8_t(prefix) > 0 ? 
				value * range / power_of_10(int8_t(prefix)) / (1UL << precision_bits) :
				value * range * power_of_10(int8_t(prefix)) / (1UL << precision_bits));
	}
	constexpr int16_t map_physical_to_raw(int16_t value, UnitPrefix prefix, int16_t range, uint8_t precision_bits)
	{
		// Here we approximate the calculation by using 2^n instead of (2^n - 1) as input range
		return (int8_t(prefix) > 0 ? 
				value * (1UL << precision_bits) * power_of_10(int8_t(prefix)) / range :
				value * (1UL << precision_bits) / power_of_10(int8_t(prefix)) / range);
	}
	
	constexpr uint16_t as_uint16_t(uint8_t high, uint8_t low)
	{
		return (high << 8) | low;
	}

	template<typename T>
	constexpr T is_zero(T value, T default_value)
	{
		return (value ? value : default_value);
	}

	template<typename T>
	void set_mask(volatile T& reg, T mask, T value)
	{
		reg = (reg & ~mask) | (value & mask);
	}
	
	inline uint8_t bcd_to_binary(uint8_t bcd)
	{
		uint8_t tens = bcd >> 4;
		// We avoid tens * 10 to avoid adding library for multiplication
		return (tens * 8) + (tens * 2) + (bcd & 0x0F);
	}
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
	
	inline void swap_bytes(uint16_t& value)
	{
		value = (value >> 8) | (value << 8);
	}
	inline void swap_bytes(int16_t& value)
	{
		swap_bytes((uint16_t&)value);
	}
	
	
	template<typename T>
	union ToUint8
	{
		static_assert(sizeof(T) == 1, "T must be a one-byte size type");
		ToUint8(T value): value(value) {}
		T value;
		uint8_t as_uint8;
	};
	template<typename T>
	constexpr uint8_t as_uint8_t(T input)
	{
		return ToUint8<T>(input).as_uint8;
	}
	
	constexpr uint8_t calculate_delay1_count(float time_us)
	{
		return uint8_t(F_CPU / 1000000UL / 3.0 * time_us);
	}
}

#endif	/* UTILITIES_HH */
