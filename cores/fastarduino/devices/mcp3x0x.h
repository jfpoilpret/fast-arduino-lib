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
 * API to handle MicroChip ADC chips family (MCP3001-2-4-8, MCP3201-2-4-8, MCP3301-2-4).
 */
#ifndef MCP3X0X_HH
#define MCP3X0X_HH

#include "../spi.h"
#include "../time.h"
#include "../utilities.h"

namespace devices::mcp3x0x
{
	//TODO Add APIDOC
	template<board::DigitalPin CS, typename CHANNEL, uint16_t MASK, uint8_t RSHIFT>
	class MCP3x0x : public spi::SPIDevice<
		CS, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(3'600'000UL), 
		spi::Mode::MODE_0, spi::DataOrder::MSB_FIRST>
	{
	public:
		MCP3x0x() = default;

		uint16_t read_channel(CHANNEL channel)
		{
			uint8_t result1;
			uint8_t result2;
			this->start_transfer();
			if (sizeof(CHANNEL) == 2)
			{
				this->transfer(utils::high_byte(uint16_t(channel)));
				result1 = this->transfer(utils::low_byte(uint16_t(channel)));
			}
			else
				result1 = this->transfer(uint8_t(channel));
			result2 = this->transfer(0x00);
			this->end_transfer();
			// Convert bytes pair to N-bits result
			return (utils::as_uint16_t(result1, result2) & MASK) >> RSHIFT;
		}
	};
}

#endif /* MCP3X0X_HH */
/// @endcond
