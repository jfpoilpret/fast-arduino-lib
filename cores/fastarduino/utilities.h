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
	
	uint8_t bcd_to_binary(uint8_t bcd) INLINE;
	uint8_t bcd_to_binary(uint8_t bcd)
	{
		uint8_t tens = bcd >> 4;
		// We avoid tens * 10 to avoid adding library for multiplication
		return (tens * 8) + (tens * 2) + (bcd & 0x0F);
	}
	uint8_t binary_to_bcd(uint8_t binary) INLINE;
	uint8_t binary_to_bcd(uint8_t binary)
	{
		uint8_t bcd = 0;
		while (binary >= 10)
		{
			bcd += 0x10;
			binary -= 10;
		}
		return bcd + binary;
	}
}

#endif	/* UTILITIES_HH */
