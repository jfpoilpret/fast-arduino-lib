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
 * API to handle MCP3008 ADC chip.
 */
#ifndef MCP3008_HH
#define MCP3008_HH

#include "../spi.h"
#include "../time.h"
#include "../utilities.h"

namespace devices::mcp3x0x
{
	//TODO add generic support for whole family: number of bits, number of channels...
	//TODO Add APIDOC

	enum class MCP3008Channel : uint8_t
	{
		// singled-ended input
		CH0 = 0x80,
		CH1 = 0x90,
		CH2 = 0xA0,
		CH3 = 0xB0,
		CH4 = 0xC0,
		CH5 = 0xD0,
		CH6 = 0xE0,
		CH7 = 0xF0,
		// differential input
		CH0_CH1 = 0x00,
		CH1_CH0 = 0x10,
		CH2_CH3 = 0x20,
		CH3_CH2 = 0x30,
		CH4_CH5 = 0x40,
		CH5_CH4 = 0x50,
		CH6_CH7 = 0x60,
		CH7_CH6 = 0x70
	};

	template<board::DigitalPin CS>
	class MCP3008 : public spi::SPIDevice<
		CS, spi::ChipSelect::ACTIVE_LOW, spi::compute_clockrate(3'600'000UL), 
		spi::Mode::MODE_0, spi::DataOrder::MSB_FIRST>
	{
	public:
		MCP3008() = default;

		uint16_t read_channel(MCP3008Channel channel)
		{
			this->start_transfer();
			this->transfer(0x01);
			uint8_t result1 = this->transfer(uint8_t(channel));
			uint8_t result2 = this->transfer(0x00);
			this->end_transfer();
			// Convert bytes pair to 10 bits result
			return utils::as_uint16_t(result1 & 0x03, result2);
		}
	};
}

#endif /* MCP3008_HH */
/// @endcond
