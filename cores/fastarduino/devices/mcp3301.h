//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * API to handle MicroChip ADC chip MCP3301.
 */
#ifndef MCP3301_HH
#define MCP3301_HH

#include "mcp3x0x.h"

namespace devices::mcp3x0x
{
	/**
	 * List of channels supported by MCP3301.
	 */
	enum class MCP3301Channel : uint8_t
	{
		// differential input
		DIFF = 0x00
	};

	/**
	 * Device class supporting MCP3301 ADC chip.
	 * @warning Support for this device has not been tested by author.
	 * @sa MCP3302
	 * @sa MCP3304
	 */
	template<board::DigitalPin CS>
	using MCP3301 = MCP3x0x<CS, MCP3301Channel, 0x1FFF, 0, int16_t>;
}

#endif /* MCP3301_HH */
/// @endcond
