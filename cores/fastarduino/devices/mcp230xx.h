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
 * API to handle the MCP23008/23017 chips (8 & 16-Bit I/O Expanders with I2C interface).
 */
#ifndef MCP230XX_H
#define MCP230XX_H

#include <stdint.h>

namespace devices
{
	/**
	 * Defines the API for MCP23008/MCP23017 chips support.
	 * @sa devices::mcp230xx::MCP23008
	 * @sa devices::mcp230xx::MCP23017
	 */
	namespace mcp230xx
	{
	}
}

namespace devices::mcp230xx
{
	/**
	 * The polarity of the MCP23008/MCP23017 INT pins.
	 */
	enum class InterruptPolarity : uint8_t
	{
		/**
		 * The INT pins shall be active low, ie they are high by default, and 
		 * changed to low when an interrupt is triggered.
		 */
		ACTIVE_LOW = 0,
		/**
		 * The INT pins shall be active high, ie they are low by default, and 
		 * changed to high when an interrupt is triggered.
		 */
		ACTIVE_HIGH = 1
	};
}

#endif /* MCP230XX_H */
/// @endcond
