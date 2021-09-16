//   Copyright 2016-2021 Jean-Francois Poilpret
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

/*
 * Static checks of MCP3x0x constexpr formulas that may look hazardous.
 * This program does nothing and is not aimed at deploying to anything.
 */

#include <fastarduino/defines.h>
#include <fastarduino/boards/board.h>
#include <fastarduino/types_traits.h>
#include <fastarduino/utilities.h>

template<uint16_t MASK, uint8_t RSHIFT, typename TYPE> struct MCP3x0x
{
	using TRAIT = types_traits::Type_trait<TYPE>;
	static_assert(TRAIT::IS_INT && (TRAIT::SIZE == 2), "TYPE must be uint16_t or int16_t");
	static constexpr const bool IS_SIGNED = TRAIT::IS_SIGNED;
	static constexpr const uint16_t SIGN_MASK = ((MASK >> RSHIFT) + 1) >> 1;
	static constexpr const uint16_t NEGATIVE = 0xFFFF & ~(MASK >> RSHIFT);

	template<uint8_t byte1, uint8_t byte2>
	static constexpr TYPE read_channel()
	{
		// Convert bytes pair to N-bits result
		constexpr uint16_t value = (utils::as_uint16_t(byte1, byte2) & MASK) >> RSHIFT;
		if (IS_SIGNED)
		{
			if (value & SIGN_MASK)
				// value is negative, change it to negative int16_t
				return TYPE(NEGATIVE | value);
			else
				// value is positive, directly cast it to int16_t
				return TYPE(value);
		}
		else
			return value;
	}
};

// Check unsigned 10-bits values 
using MCP3001 = MCP3x0x<0x03FF, 0, uint16_t>;
static_assert(MCP3001::read_channel<0, 0>() == 0, "");
static_assert(MCP3001::read_channel<0xFF, 0xFF>() == 1023, "");
static_assert(MCP3001::read_channel<0x00, 0xFF>() == 255, "");

// Check unsigned 12-bits values 
using MCP3201 = MCP3x0x<0x1FFE, 1, uint16_t>;
static_assert(MCP3201::read_channel<0, 0>() == 0, "");
static_assert(MCP3201::read_channel<0xFF, 0xFF>() == 4095, "");
static_assert(MCP3201::read_channel<0x00, 0xFF>() == 127, "");

// Check signed 13-bits values 
using MCP3301 = MCP3x0x<0x1FFF, 0, int16_t>;
static_assert(MCP3301::read_channel<0x0F, 0xFF>() == 4095, "");
static_assert(MCP3301::read_channel<0x0F, 0xFE>() == 4094, "");
static_assert(MCP3301::read_channel<0x00, 0x02>() == 2, "");
static_assert(MCP3301::read_channel<0x00, 0x01>() == 1, "");
static_assert(MCP3301::read_channel<0, 0>() == 0, "");
static_assert(MCP3301::read_channel<0x1F, 0xFF>() == -1, "");
static_assert(MCP3301::read_channel<0x1F, 0xFE>() == -2, "");
static_assert(MCP3301::read_channel<0x10, 0x01>() == -4095, "");
static_assert(MCP3301::read_channel<0x10, 0x00>() == -4096, "");
static_assert(MCP3301::read_channel<0x00, 0xFF>() == 255, "");

int main()
{
}
