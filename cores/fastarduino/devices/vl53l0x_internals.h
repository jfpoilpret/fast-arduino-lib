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

/// @cond notdocumented

#ifndef VL53L0X_INTERNALS_H
#define VL53L0X_INTERNALS_H

#include "../flash.h"

namespace devices::vl53l0x_internals
{
	// Constants for init_data() method
	//----------------------------------
	// Write buffer
	static constexpr uint8_t INIT_DATA_BUFFER[] PROGMEM =
	{
		0x88, 0x00,
		0x80, 0x01,
		0xFF, 0x01,
		0x00, 0x00,
		// Read stop variable here
		0x91,
		0x00, 0x01,
		0xFF, 0x00,
		0x80, 0x00,
		// Read REG_MSRC_CONFIG_CONTROL
		0x60,
		// Write REG_MSRC_CONFIG_CONTROL (disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE limit checks)
		0x60, 0x12,
		// set REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT to 0.5
		0x44, 0x00, 0x40,
		// SYSTEM_SEQUENCE_CONFIG
		0x01, 0xFF
	};
	// Size of write buffer
	static constexpr uint8_t INIT_DATA_BUFFER_WRITE_SIZE = sizeof(INIT_DATA_BUFFER);
	// Size of read buffer
	static constexpr uint8_t INIT_DATA_BUFFER_READ_SIZE = 2;
	// List of bytes count for each read/write (read is negative, write is positive)
	static constexpr int8_t INIT_DATA_BUFFER_R_W[] PROGMEM = {2, 2, 2, 2, 1, -1, 2, 2, 2, 1, -1, 2, 3, 2};
	//TODO Index of value to write for REG_MSRC_CONFIG_CONTROL
	static constexpr uint8_t INIT_DATA_BUFFER_MSRC_CONFIG_CONTROL_INDEX = 17;
}

#endif /* VL53L0X_INTERNALS_H */
/// @endcond
