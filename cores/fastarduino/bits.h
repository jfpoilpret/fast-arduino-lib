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

// This file wraps "avr/io.h" header, in order to override all _SFR_XXX() macros
// to allow usage of all defines (such as PINB) to work in constexpr evalutations.

#ifndef BITS_HH
#define BITS_HH

#include <stdint.h>

//TODO Add docs
namespace bits
{
	static constexpr uint8_t BV8(uint8_t bit)
	{
		return uint8_t(1 << bit);
	}
	static constexpr uint8_t BV8(uint8_t bit1, uint8_t bit2)
	{
		return uint8_t(BV8(bit1) | BV8(bit2));
	}
	static constexpr uint8_t BV8(uint8_t bit1, uint8_t bit2, uint8_t bit3)
	{
		return uint8_t(BV8(bit1, bit2) | BV8(bit3));
	}
	static constexpr uint8_t BV8(uint8_t bit1, uint8_t bit2, uint8_t bit3, uint8_t bit4)
	{
		return uint8_t(BV8(bit1, bit2) | BV8(bit3, bit4));
	}

	static constexpr uint8_t CBV8(uint8_t bit)
	{
		return uint8_t(~BV8(bit));
	}

	static constexpr uint16_t BV16(uint8_t bit)
	{
		return uint16_t(1 << bit);
	}
	static constexpr uint16_t CBV16(uint8_t bit)
	{
		return uint16_t(~BV16(bit));
	}

	static constexpr uint8_t COMPL(uint8_t value)
	{
		return uint8_t(~value);
	}

	static constexpr uint8_t LOW_BYTE(uint16_t value)
	{
		return uint8_t(value & 0x00FFU);
	}
	static constexpr uint8_t HIGH_BYTE(uint16_t value)
	{
		return uint8_t(value >> 8);
	}
}

#endif /* BITS_HH */
