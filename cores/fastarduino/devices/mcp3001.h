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
 * API to handle MicroChip ADC chip MCP3001.
 */
#ifndef MCP3001_HH
#define MCP3001_HH

#include "mcp3x0x.h"

namespace devices::mcp3x0x
{
	/**
	 * List of channels supported by MCP3001.
	 */
	enum class MCP3001Channel : uint8_t
	{
		// single-ended input
		CH0 = 0x00
	};

	/**
	 * Device class supporting MCP3001 ADC chip.
	 * @warning Support for this device has not been tested by author.
	 */
	template<board::DigitalPin CS>
	using MCP3001 = MCP3x0x<CS, MCP3001Channel, 0x1FF8, 3>;
}

#endif /* MCP3001_HH */
/// @endcond
