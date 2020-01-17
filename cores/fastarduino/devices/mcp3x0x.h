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
 * API to handle SPI-based MicroChip ADC chips family (MCP3001-2-4-8, 
 * MCP3201-2-4-8, MCP3301-2-4).
 */
#ifndef MCP3X0X_HH
#define MCP3X0X_HH

#include "../spi.h"
#include "../time.h"
#include "../types_traits.h"
#include "../utilities.h"

namespace devices
{
	/**
	 * Defines the API for MicroChip ADC chips family support.
	 */
	namespace mcp3x0x
	{
	}
}

namespace devices::mcp3x0x
{
	/**
	 * Generic class to support (almost) any chip of the SPI-based MicroChip ADC
	 * chips family (MCP3001-2-4-8, MCP3201-2-4-8, MCP3301-2-4).
	 * You would never directly use this class in your programs but rather use
	 * class aliases with the proper template parameter values.
	 * 
	 * @tparam CS the output pin used for Chip Selection of the MCP chip on
	 * the SPI bus.
	 * @tparam CHANNEL_ an `enum class` type defining all possible analog input 
	 * channels handled by the device; this is used as an argument in `read_channel()`;
	 * it must be castable to a `uint8_t` or `uint16_t` that will be used as the first
	 * or two first bytes in the transmission to the chip.
	 * @tparam MASK the mask to use on the 2 bytes received from the chip to get the 
	 * analog value read; this mask shall match the number of bits returned by the chip.
	 * @tparam RSHIFT the number of bits to shift right on the analog value read by the 
	 * chip; some MCP devices actually return values that are not right-aligned; for
	 * these devices you need a non-0 @p RSHIFT.
	 * @tparam TYPE_ the analog output type, typically `uint16_t`, or `int16_t` for 
	 * some devices.
	 * 
	 * @sa MCP3008
	 * @sa MCP3208
	 * @sa MCP3304
	 */
	template<board::DigitalPin CS, typename CHANNEL_, uint16_t MASK, uint8_t RSHIFT, typename TYPE_ = uint16_t>
	class MCP3x0x : public spi::SPIDevice<
		CS, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(3'600'000UL), 
		spi::Mode::MODE_0, spi::DataOrder::MSB_FIRST>
	{
		using TRAIT = types_traits::Type_trait<TYPE_>;
		static_assert(TRAIT::IS_INT && (TRAIT::SIZE == sizeof(uint16_t)), "TYPE must be uint16_t or int16_t");
		static constexpr const bool IS_SIGNED = TRAIT::IS_SIGNED;
		static constexpr const uint16_t SIGN_MASK = ((MASK >> RSHIFT) + 1) >> 1;
		static constexpr const uint16_t NEGATIVE = 0xFFFF & ~(MASK >> RSHIFT);

	public:
		/**
		 * The `enum class` type defining all possible analog input 
		 * channels handled by the device.
		 * @sa read_channel()
		 */
		using CHANNEL = CHANNEL_;

		/**
		 * The analog output type for this device, either `uint16_t` or `int16_t`.
		 * @sa read_channel()
		 */
		using TYPE = TYPE_;

		/**
		 * Create a new device driver for an MCP chip.
		 */
		MCP3x0x() = default;

		/**
		 * Read an analog channel from this device.
		 * @param channel the channel to read from this device
		 * @return the analog value; the bits precision depends on each device
		 * (typically 10, 12 or 13 bits).
		 */
		TYPE read_channel(CHANNEL channel)
		{
			uint8_t result1;
			uint8_t result2;
			this->start_transfer();
			if (sizeof(CHANNEL) == sizeof(uint16_t))
			{
				this->transfer(utils::high_byte(uint16_t(channel)));
				result1 = this->transfer(utils::low_byte(uint16_t(channel)));
			}
			else
				result1 = this->transfer(uint8_t(channel));
			result2 = this->transfer(0x00);
			this->end_transfer();
			// Convert bytes pair to N-bits result
			uint16_t value = (utils::as_uint16_t(result1, result2) & MASK) >> RSHIFT;
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
}

#endif /* MCP3X0X_HH */
/// @endcond
