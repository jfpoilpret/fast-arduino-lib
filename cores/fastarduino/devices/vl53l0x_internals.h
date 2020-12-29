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
#include "../i2c_device_utilities.h"
#include "vl53l0x_registers.h"
#include "vl53l0x_types.h"

namespace devices::vl53l0x_internals
{
	namespace regs = vl53l0x_registers;
	namespace actions = i2c::actions;

	// List of includable futures
	static constexpr uint8_t INCLUDE_DEVICE_STROBE_WAIT = 1;
	static constexpr uint8_t INCLUDE_LOAD_TUNING_SETTINGS = 2;
	static constexpr uint8_t INCLUDE_GET_SPAD_INFO = 3;
	static constexpr uint8_t INCLUDE_SET_GPIO_SETTINGS = 4;
	static constexpr uint8_t INCLUDE_GET_MEASUREMENT_TIMING = 5;
	static constexpr uint8_t INCLUDE_SET_MEASUREMENT_TIMING = 6;
	static constexpr uint8_t INCLUDE_GET_SEQUENCE_STEPS = 7;
	static constexpr uint8_t INCLUDE_SET_SEQUENCE_STEPS = 8;
	static constexpr uint8_t INCLUDE_GET_SEQUENCE_STEPS_TIMEOUT = 9;
	static constexpr uint8_t INCLUDE_PERFORM_REF_VHV_CALIBRATION = 10;
	static constexpr uint8_t INCLUDE_PERFORM_REF_PHASE_CALIBRATION = 11;

	//TODO remove extra overwritten bytes from BUFFERs and remove extra next_byte()

	// Constants for set_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>() method
	//---------------------------------------------------------------------------
	//TODO LATER
	namespace vcsel_period_data_pre_range
	{
		// Marker before writing REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH
		static constexpr uint8_t MARKER_PHASE_CHECK_LIMIT = 1;
		// Marker before writing REG_PRE_RANGE_CONFIG_VCSEL_PERIOD
		static constexpr uint8_t MARKER_VSEL_PERIOD = 2;
		// Marker before writing REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI
		static constexpr uint8_t MARKER_PRE_RANGE_TIMEOUT = 3;
		// Marker before writing REG_MSRC_CONFIG_TIMEOUT_MACROP
		static constexpr uint8_t MARKER_MSRC_TIMEOUT = 4;
		// Marker before writing to REG_SYSTEM_SEQUENCE_CONFIG (restore sequence steps)
		static constexpr uint8_t MARKER_RESTORE_SEQUENCE = 5;

		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			// Get Sequence steps enable (include)
			actions::INCLUDE, INCLUDE_GET_SEQUENCE_STEPS,
			// Get Sequence steps timeouts (include)
			actions::INCLUDE, INCLUDE_GET_SEQUENCE_STEPS_TIMEOUT,
			// Write PRE_RANGE_CONFIG_VALID_PHASE (2 bytes)
			actions::MARKER, MARKER_PHASE_CHECK_LIMIT,
			actions::write(1), regs::REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH, 0x00,
			actions::write(1), regs::REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW, 0x08,
			// Apply VCSEL period (PRE_RANGE_CONFIG_VCSEL_PERIOD)
			actions::MARKER, MARKER_VSEL_PERIOD,
			actions::write(1), regs::REG_PRE_RANGE_CONFIG_VCSEL_PERIOD, 0x00,
			// Set sequence step timeout (PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI)
			actions::MARKER, MARKER_PRE_RANGE_TIMEOUT,
			actions::write(2), regs::REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, 0x00, 0x00,
			// Set sequence step timeout (MSRC_CONFIG_TIMEOUT_MACROP)
			actions::MARKER, MARKER_MSRC_TIMEOUT,
			actions::write(1), regs::REG_MSRC_CONFIG_TIMEOUT_MACROP,
			// Re-apply timing budget (FIXME where from?)
			actions::INCLUDE, INCLUDE_SET_MEASUREMENT_TIMING,
			//TODO Perform phase calibration
			actions::read(1), regs::REG_SYSTEM_SEQUENCE_CONFIG,
			actions::write(1), regs::REG_SYSTEM_SEQUENCE_CONFIG, 0x02,
			//TODO note the include is wrong
			actions::INCLUDE, INCLUDE_PERFORM_REF_PHASE_CALIBRATION,
			actions::MARKER, MARKER_RESTORE_SEQUENCE,
			actions::write(1, true), regs::REG_SYSTEM_SEQUENCE_CONFIG, 0x00
		};
	}

	// Constants for set_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>() method
	//-----------------------------------------------------------------------------
	//TODO LATER
	namespace vcsel_period_data_final_range
	{
	}

	// Constants for perform_ref_calibration() method
	//------------------------------------------------
	namespace perform_ref_calibration
	{
		// Write buffer
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			// Read sequence steps
			actions::read(1), regs::REG_SYSTEM_SEQUENCE_CONFIG,
			// Set steps for VHV calibration
			actions::write(1), regs::REG_SYSTEM_SEQUENCE_CONFIG, 0x01,
			// Perform single VHV calibration
			actions::INCLUDE, INCLUDE_PERFORM_REF_VHV_CALIBRATION,
			// Set steps for Phase calibration
			actions::write(1), regs::REG_SYSTEM_SEQUENCE_CONFIG, 0x02,
			// Perform single Phase calibration
			actions::INCLUDE, INCLUDE_PERFORM_REF_PHASE_CALIBRATION,
			// Restore sequence steps (NOTE: 0x00 is used as marker by the future to actually restore saved sequence)
			actions::write(1, true), regs::REG_SYSTEM_SEQUENCE_CONFIG, 0x00,
			actions::END
		};
	}

	// Constants for init_data() method
	//----------------------------------
	namespace init_data
	{
		// Marker after reading REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV
		static constexpr uint8_t MARKER_VHV_CONFIG = 1;
		// Marker after reading stop_variable
		static constexpr uint8_t MARKER_STOP_VARIABLE = 2;
		// Marker after reading REG_MSRC_CONFIG_CONTROL
		static constexpr uint8_t MARKER_MSRC_CONFIG_CONTROL = 3;

		// Write buffer
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			// read/write VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV to force 2.8V or keep 1.8V default
			actions::read(1), regs::REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, // 1 BYTE READ
			actions::MARKER, MARKER_VHV_CONFIG,
			actions::write(1), regs::REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, 0x01, // OVERWRITTEN BYTE
			// set I2C standard mode
			actions::write(1), 0x88, 0x00,
			actions::write(1), regs::REG_POWER_MANAGEMENT, 0x01,
			actions::write(1), 0xFF, 0x01,
			actions::write(1), regs::REG_SYSRANGE_START, 0x00,
			// read stop variable here
			actions::read(1), 0x91, // 1 BYTE READ
			actions::MARKER, MARKER_STOP_VARIABLE,
			actions::write(1), regs::REG_SYSRANGE_START, 0x01,
			actions::write(1), 0xFF, 0x00,
			actions::write(1), regs::REG_POWER_MANAGEMENT, 0x00,
			// read/write REG_MSRC_CONFIG_CONTROL to disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE limit checks
			actions::read(1), regs::REG_MSRC_CONFIG_CONTROL, // 1 BYTE READ
			actions::MARKER, MARKER_MSRC_CONFIG_CONTROL,
			actions::write(1), regs::REG_MSRC_CONFIG_CONTROL, 0x12, // OVERWRITTEN BYTE
			// set signal rate limit to 0.25 MCPS (million counts per second) in FP9.7 format
			actions::write(2), regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x00, 0x20,
			// enable all sequence steps by default
			actions::write(1, true), regs::REG_SYSTEM_SEQUENCE_CONFIG, 0xFF,
			actions::END
		};

		// Value to force (OR) 2V8 in REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV register
		static constexpr uint8_t VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_SET_2V8 = 0x01;
		// Value to disable (OR?) SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE in REG_MSRC_CONFIG_CONTROL register
		static constexpr uint8_t MSRC_CONFIG_CONTROL_INIT = 0x12;
	}

	// Constants for get_SPAD_info() method
	//--------------------------------------
	namespace spad_info
	{
		// Marker after reading register 0x83 before owverwriting it
		static constexpr uint8_t MARKER_OVERWRITE_REG_DEVICE_STROBE = 1;
		// Marker after reading register 0x92 (SPAD info byte)
		static constexpr uint8_t MARKER_READ_SPAD_INFO = 2;

		// Write buffer
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			actions::write(1), regs::REG_POWER_MANAGEMENT, 0x01,
			actions::write(1), 0xFF, 0x01,
			actions::write(1), regs::REG_SYSRANGE_START, 0x00,
			actions::write(1), 0xFF, 0x06,
			actions::read(1), regs::REG_DEVICE_STROBE, // 1 BYTE READ
			actions::MARKER, MARKER_OVERWRITE_REG_DEVICE_STROBE,
			actions::write(1), regs::REG_DEVICE_STROBE, 0x04, // OVERWRITTEN BYTE
			actions::write(1), 0xFF, 0x07,
			actions::write(1), 0x81, 0x01,
			actions::write(1), regs::REG_POWER_MANAGEMENT, 0x01,
			actions::write(1), 0x94, 0x6B,

			actions::INCLUDE, INCLUDE_DEVICE_STROBE_WAIT,

			actions::read(1), regs::REG_SPAD_INFO, // 1 BYTE READ: SPAD info byte
			actions::MARKER, MARKER_READ_SPAD_INFO,
			actions::write(1), 0x81, 0x00,
			actions::write(1), 0xFF, 0x06,
			actions::read(1), regs::REG_DEVICE_STROBE, // 1 BYTE READ
			actions::MARKER, MARKER_OVERWRITE_REG_DEVICE_STROBE,
			actions::write(1), regs::REG_DEVICE_STROBE, 0x04, // OVERWRITTEN BYTE
			actions::write(1), 0xFF, 0x01,
			actions::write(1), regs::REG_SYSRANGE_START, 0x01,
			actions::write(1), 0xFF, 0x00,
			actions::write(1, true), regs::REG_POWER_MANAGEMENT, 0x00,
			actions::END
		};

		// Value to force (OR) into register 0x83 at 2 occurrences
		static constexpr uint8_t REG_DEVICE_STROBE_FORCED_VALUE = 0x04;
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
			regs::REG_SYSRANGE_START, 0x00,
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
			regs::REG_POWER_MANAGEMENT, 0x01,
			0x01, 0xF8,
			0xFF, 0x01,
			0x8E, 0x01,
			regs::REG_SYSRANGE_START, 0x01,
			0xFF, 0x00,
			regs::REG_POWER_MANAGEMENT, 0x00
		};
		// Size of write buffer
		static constexpr uint8_t BUFFER_SIZE = sizeof(BUFFER);
	}

	// Constants for init_static() method
	//------------------------------------
	namespace init_static
	{
		// Marker after reading register 0x83 before owverwriting it
		static constexpr uint8_t MARKER_GET_REFERENCE_SPADS = 1;

		// Write buffer
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			// get SPAD info
			actions::INCLUDE, INCLUDE_GET_SPAD_INFO,

			// get reference SPADs from NVM
			actions::read(6), regs::REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0, // 6 BYTES READ: RefGoodSpadMap

			// set reference SPADs (after calculation)
			actions::write(1), 0xFF, 0x01,
			actions::write(1), regs::REG_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00,
			actions::write(1), regs::REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C,
			actions::write(1), 0xFF, 0x00,
			actions::write(1), regs::REG_GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4,
			actions::MARKER, MARKER_GET_REFERENCE_SPADS,
			actions::write(6), regs::REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

			// load tuning settings (hard-coded defaults)
			actions::INCLUDE, INCLUDE_LOAD_TUNING_SETTINGS,

			// set interrupt settings with default?
			actions::INCLUDE, INCLUDE_SET_GPIO_SETTINGS,

			// get current timing budget
			actions::INCLUDE, INCLUDE_GET_MEASUREMENT_TIMING,

			// set sequence steps: disable MSRC and TCC by default
			actions::write(1), regs::REG_SYSTEM_SEQUENCE_CONFIG, 0xE8,

			// recalculate timing budget
			actions::INCLUDE, INCLUDE_SET_MEASUREMENT_TIMING,

			actions::END
		};
	}
	
}
#endif /* VL53L0X_INTERNALS_H */
/// @endcond
