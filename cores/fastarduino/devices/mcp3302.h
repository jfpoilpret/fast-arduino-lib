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
 * API to handle MicroChip ADC chip MCP3302.
 */
#ifndef MCP3302_HH
#define MCP3302_HH

#include "mcp3x0x.h"

namespace devices::mcp3x0x
{
	/**
	 * List of channels supported by MCP3302.
	 */
	enum class MCP3302Channel : uint16_t
	{
		// single-ended input
		CH0 = 0x0C00,
		CH1 = 0x0C80,
		CH2 = 0x0D00,
		CH3 = 0x0D80,
		// differential input
		CH0_CH1 = 0x0800,
		CH1_CH0 = 0x0880,
		CH2_CH3 = 0x0900,
		CH3_CH2 = 0x0980
	};

	/**
	 * Device class supporting MCP3302 ADC chip.
	 * @warning Support for this device has not been tested by author.
	 * @sa MCP3301
	 * @sa MCP3304
	 */
	template<board::DigitalPin CS>
	using MCP3302 = MCP3x0x<CS, MCP3302Channel, 0x1FFF, 0, int16_t>;
}

#endif /* MCP3302_HH */
/// @endcond
