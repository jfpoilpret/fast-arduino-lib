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

/// @cond notdocumented

#ifndef VL53L0X_INTERNALS_H
#define VL53L0X_INTERNALS_H

#include "../flash.h"
#include "../i2c_device_utilities.h"
#include "vl53l0x_registers.h"
#include "vl53l0x_types.h"

namespace devices::vl53l0x_internals
{
	using Register = devices::vl53l0x::Register;

	namespace stop_variable
	{
		// Write buffers
		static constexpr uint8_t PRE_BUFFER[] PROGMEM =
		{
			uint8_t(Register::POWER_MANAGEMENT), 0x01,
			0xFF, 0x01,
			uint8_t(Register::SYSRANGE_START), 0x00,
		};
		static constexpr uint8_t POST_BUFFER[] PROGMEM =
		{
			uint8_t(Register::SYSRANGE_START), 0x01,
			0xFF, 0x00,
			uint8_t(Register::POWER_MANAGEMENT), 0x00,
		};

		// Size of write buffers
		static constexpr uint8_t PRE_BUFFER_SIZE = sizeof(PRE_BUFFER);
		static constexpr uint8_t POST_BUFFER_SIZE = sizeof(POST_BUFFER);
	}

	namespace stop_continuous_ranging
	{
		// Write buffers
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			uint8_t(Register::SYSRANGE_START), 0x01,
			0xFF, 0x01,
			uint8_t(Register::SYSRANGE_START), 0x00,
			uint8_t(Register::SYSTEM_STOP_VARIABLE), 0x00,
			uint8_t(Register::SYSRANGE_START), 0x01,
			0xFF, 0x00,
		};

		// Size of write buffers
		static constexpr uint8_t BUFFER_SIZE = sizeof(BUFFER);
	}

	// Constants for get_SPAD_info() method
	//--------------------------------------
	namespace spad_info
	{
		// Write buffers
		static constexpr uint8_t BUFFER1[] PROGMEM =
		{
			uint8_t(Register::POWER_MANAGEMENT), 0x01,
			0xFF, 0x01,
			uint8_t(Register::SYSRANGE_START), 0x00,
			0xFF, 0x06,
		};

		static constexpr uint8_t BUFFER2[] PROGMEM =
		{
			0xFF, 0x07,
			0x81, 0x01,
			uint8_t(Register::POWER_MANAGEMENT), 0x01,
			0x94, 0x6B,
		};

		static constexpr uint8_t BUFFER3[] PROGMEM =
		{
			0x81, 0x00,
			0xFF, 0x06,
		};

		static constexpr uint8_t BUFFER4[] PROGMEM =
		{
			0xFF, 0x01,
			uint8_t(Register::SYSRANGE_START), 0x01,
			0xFF, 0x00,
			uint8_t(Register::POWER_MANAGEMENT), 0x00,
		};

		// Size of write buffers
		static constexpr uint8_t BUFFER1_SIZE = sizeof(BUFFER1);
		static constexpr uint8_t BUFFER2_SIZE = sizeof(BUFFER2);
		static constexpr uint8_t BUFFER3_SIZE = sizeof(BUFFER3);
		static constexpr uint8_t BUFFER4_SIZE = sizeof(BUFFER4);
	}
	
	// Constants for load_tuning_settings() method
	//---------------------------------------------
	namespace load_tuning_settings
	{
		// Write buffer
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			// load tuning settings (hard-coded defaults)
			0xFF, 0x01,
			uint8_t(Register::SYSRANGE_START), 0x00,
			0xFF, 0x00,
			0x09, 0x00,
			0x10, 0x00,
			0x11, 0x00,
			0x24, 0x01,
			0x25, 0xFF,
			0x75, 0x00,
			0xFF, 0x01,
			0x4E, 0x2C,
			0x48, 0x00,
			0x30, 0x20,
			0xFF, 0x00,
			0x30, 0x09,
			0x54, 0x00,
			0x31, 0x04,
			0x32, 0x03,
			0x40, 0x83,
			0x46, 0x25,
			0x60, 0x00,
			0x27, 0x00,
			0x50, 0x06,
			0x51, 0x00,
			0x52, 0x96,
			0x56, 0x08,
			0x57, 0x30,
			0x61, 0x00,
			0x62, 0x00,
			0x64, 0x00,
			0x65, 0x00,
			0x66, 0xA0,
			0xFF, 0x01,
			0x22, 0x32,
			0x47, 0x14,
			0x49, 0xFF,
			0x4A, 0x00,
			0xFF, 0x00,
			0x7A, 0x0A,
			0x7B, 0x00,
			0x78, 0x21,
			0xFF, 0x01,
			0x23, 0x34,
			0x42, 0x00,
			0x44, 0xFF,
			0x45, 0x26,
			0x46, 0x05,
			0x40, 0x40,
			0x0E, 0x06,
			0x20, 0x1A,
			0x43, 0x40,
			0xFF, 0x00,
			0x34, 0x03,
			0x35, 0x44,
			0xFF, 0x01,
			0x31, 0x04,
			0x4B, 0x09,
			0x4C, 0x05,
			0x4D, 0x04,
			0xFF, 0x00,
			0x44, 0x00,
			0x45, 0x20,
			0x47, 0x08,
			0x48, 0x28,
			0x67, 0x00,
			0x70, 0x04,
			0x71, 0x01,
			0x72, 0xFE,
			0x76, 0x00,
			0x77, 0x00,
			0xFF, 0x01,
			0x0D, 0x01,
			0xFF, 0x00,
			uint8_t(Register::POWER_MANAGEMENT), 0x01,
			0x01, 0xF8,
			0xFF, 0x01,
			0x8E, 0x01,
			uint8_t(Register::SYSRANGE_START), 0x01,
			0xFF, 0x00,
			uint8_t(Register::POWER_MANAGEMENT), 0x00
		};
		// Size of write buffer
		static constexpr uint8_t BUFFER_SIZE = sizeof(BUFFER);
	}

	// Constants for set_reference_SPADs() method
	//--------------------------------------------
	namespace set_reference_spads
	{
		// Write buffer
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			0xFF, 0x01,
			uint8_t(Register::DYNAMIC_SPAD_REF_EN_START_OFFSET), 0x00,
			uint8_t(Register::DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD), 0x2C,
			0xFF, 0x00,
			uint8_t(Register::GLOBAL_CONFIG_REF_EN_START_SELECT), 0xB4
		};

		// Size of write buffer
		static constexpr uint8_t BUFFER_SIZE = sizeof(BUFFER);
	}
}
#endif /* VL53L0X_INTERNALS_H */
/// @endcond
