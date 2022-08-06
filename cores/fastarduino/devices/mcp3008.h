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
 * API to handle MicroChip ADC chip MCP3008.
 */
#ifndef MCP3008_HH
#define MCP3008_HH

#include "mcp3x0x.h"

namespace devices::mcp3x0x
{
	/**
	 * List of channels supported by MCP3008.
	 */
	enum class MCP3008Channel : uint16_t
	{
		// single-ended input
		CH0 = 0x0180,
		CH1 = 0x0190,
		CH2 = 0x01A0,
		CH3 = 0x01B0,
		CH4 = 0x01C0,
		CH5 = 0x01D0,
		CH6 = 0x01E0,
		CH7 = 0x01F0,
		// differential input
		CH0_CH1 = 0x0100,
		CH1_CH0 = 0x0110,
		CH2_CH3 = 0x0120,
		CH3_CH2 = 0x0130,
		CH4_CH5 = 0x0140,
		CH5_CH4 = 0x0150,
		CH6_CH7 = 0x0160,
		CH7_CH6 = 0x0170
	};

	/**
	 * Device class supporting MCP3008 ADC chip.
	 * @sa MCP3001
	 * @sa MCP3002
	 * @sa MCP3004
	 */
	template<board::DigitalPin CS>
	using MCP3008 = MCP3x0x<CS, MCP3008Channel, 0x03FF, 0>;
}

#endif /* MCP3008_HH */
/// @endcond
