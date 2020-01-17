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
 * API to handle MicroChip ADC chip MCP3208.
 */
#ifndef MCP3208_HH
#define MCP3208_HH

#include "mcp3x0x.h"

namespace devices::mcp3x0x
{
	/**
	 * List of channels supported by MCP3208.
	 */
	enum class MCP3208Channel : uint16_t
	{
		// single-ended input
		CH0 = 0x0600,
		CH1 = 0x0640,
		CH2 = 0x0680,
		CH3 = 0x06C0,
		CH4 = 0x0700,
		CH5 = 0x0740,
		CH6 = 0x0780,
		CH7 = 0x07C0,

		// differential input
		CH0_CH1 = 0x0400,
		CH1_CH0 = 0x0440,
		CH2_CH3 = 0x0480,
		CH3_CH2 = 0x04C0,
		CH4_CH5 = 0x0500,
		CH5_CH4 = 0x0540,
		CH6_CH7 = 0x0580,
		CH7_CH6 = 0x05C0
	};

	/**
	 * Device class supporting MCP3208 ADC chip.
	 * @warning Support for this device has not been tested by author.
	 * @sa MCP3201
	 * @sa MCP3202
	 * @sa MCP3204
	 */
	template<board::DigitalPin CS>
	using MCP3208 = MCP3x0x<CS, MCP3208Channel, 0x0FFF, 0>;
}

#endif /* MCP3208_HH */
/// @endcond
