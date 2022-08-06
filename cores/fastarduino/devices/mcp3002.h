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
 * API to handle MicroChip ADC chip MCP3002.
 */
#ifndef MCP3002_HH
#define MCP3002_HH

#include "mcp3x0x.h"

namespace devices::mcp3x0x
{
	/**
	 * List of channels supported by MCP3002.
	 */
	enum class MCP3002Channel : uint8_t
	{
		// single-ended input
		CH0 = 0x40,
		CH1 = 0x50,
		// differential input
		CH0_CH1 = 0x60,
		CH1_CH0 = 0x70
	};

	/**
	 * Device class supporting MCP3002 ADC chip.
	 * @warning Support for this device has not been tested by author.
	 * @sa MCP3001
	 * @sa MCP3004
	 * @sa MCP3008
	 */
	template<board::DigitalPin CS>
	using MCP3002 = MCP3x0x<CS, MCP3002Channel, 0x03FF, 0>;
}

#endif /* MCP3002_HH */
/// @endcond
