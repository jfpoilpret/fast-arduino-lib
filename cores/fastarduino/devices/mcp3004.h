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
 * API to handle MicroChip ADC chip MCP3004.
 */
#ifndef MCP3004_HH
#define MCP3004_HH

#include "mcp3x0x.h"

namespace devices::mcp3x0x
{
	/**
	 * List of channels supported by MCP3004.
	 */
	enum class MCP3004Channel : uint16_t
	{
		// single-ended input
		CH0 = 0x0180,
		CH1 = 0x0190,
		CH2 = 0x01A0,
		CH3 = 0x01B0,
		// differential input
		CH0_CH1 = 0x0100,
		CH1_CH0 = 0x0110,
		CH2_CH3 = 0x0120,
		CH3_CH2 = 0x0130
	};

	/**
	 * Device class supporting MCP3004 ADC chip.
	 * @warning Support for this device has not been tested by author.
	 */
	template<board::DigitalPin CS>
	using MCP3004 = MCP3x0x<CS, MCP3004Channel, 0x03FF, 0>;
}

#endif /* MCP3004_HH */
/// @endcond
