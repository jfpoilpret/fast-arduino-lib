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

namespace devices::vl53l0x_internals
{
	namespace registers
	{
		//TODO review list to put aside unused registers
		//TODO possibly review names to make them clearer
		static constexpr const uint8_t REG_SYSRANGE_START                              = 0x00;
		static constexpr const uint8_t REG_SYSTEM_THRESH_HIGH                          = 0x0C;
		static constexpr const uint8_t REG_SYSTEM_THRESH_LOW                           = 0x0E;
		static constexpr const uint8_t REG_SYSTEM_SEQUENCE_CONFIG                      = 0x01;
		static constexpr const uint8_t REG_SYSTEM_RANGE_CONFIG                         = 0x09;
		static constexpr const uint8_t REG_SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04;
		static constexpr const uint8_t REG_SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A;
		static constexpr const uint8_t REG_GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84;
		static constexpr const uint8_t REG_SYSTEM_INTERRUPT_CLEAR                      = 0x0B;
		static constexpr const uint8_t REG_RESULT_INTERRUPT_STATUS                     = 0x13;
		static constexpr const uint8_t REG_RESULT_RANGE_STATUS                         = 0x14;
		static constexpr const uint8_t REG_RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC;
		static constexpr const uint8_t REG_RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0;
		static constexpr const uint8_t REG_RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0;
		static constexpr const uint8_t REG_RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4;
		static constexpr const uint8_t REG_RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6;
		static constexpr const uint8_t REG_ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28;
		static constexpr const uint8_t REG_I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A;
		static constexpr const uint8_t REG_MSRC_CONFIG_CONTROL                         = 0x60;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_MIN_SNR                    = 0x27;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57;
		static constexpr const uint8_t REG_PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64;
		static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67;
		static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47;
		static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48;
		static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52;
		static constexpr const uint8_t REG_SYSTEM_HISTOGRAM_BIN                        = 0x81;
		static constexpr const uint8_t REG_HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33;
		static constexpr const uint8_t REG_HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55;
		static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70;
		static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71;
		static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72;
		static constexpr const uint8_t REG_CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20;
		static constexpr const uint8_t REG_MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46;
		static constexpr const uint8_t REG_SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF;
		static constexpr const uint8_t REG_IDENTIFICATION_MODEL_ID                     = 0xC0;
		static constexpr const uint8_t REG_IDENTIFICATION_REVISION_ID                  = 0xC2;
		static constexpr const uint8_t REG_OSC_CALIBRATE_VAL                           = 0xF8;
		static constexpr const uint8_t REG_GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32;
		static constexpr const uint8_t REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0;
		static constexpr const uint8_t REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1;
		static constexpr const uint8_t REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2;
		static constexpr const uint8_t REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3;
		static constexpr const uint8_t REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4;
		static constexpr const uint8_t REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5;
		static constexpr const uint8_t REG_GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6;
		static constexpr const uint8_t REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E;
		static constexpr const uint8_t REG_DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F;
		static constexpr const uint8_t REG_POWER_MANAGEMENT                            = 0x80;
		static constexpr const uint8_t REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV            = 0x89;
		static constexpr const uint8_t REG_ALGO_PHASECAL_LIM                           = 0x30;
		static constexpr const uint8_t REG_ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30;
		static constexpr const uint8_t REG_DEVICE_STROBE                               = 0x83;
	}

	namespace regs = registers;
	namespace actions = i2c::actions;

	// List of includable futures
	static constexpr uint8_t INCLUDE_DEVICE_STROBE_WAIT = 1;


	// Constants for set_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>() method
	//---------------------------------------------------------------------------
	//TODO
	namespace vcsel_period_data_pre_range
	{
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			// Get Sequence steps enable (include)

			// Get Sequence steps timeouts (include)

			// Write PRE_RANGE_CONFIG_VALID_PHASE (2 bytes)

			// Apply VCSEL period (PRE_RANGE_CONFIG_VCSEL_PERIOD)

			// set sequence step timeout (PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI)

			// set sequence step timeout (MSRC_CONFIG_TIMEOUT_MACROP)
		};

	}

	// Constants for set_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>() method
	//-----------------------------------------------------------------------------
	//TODO
	namespace vcsel_period_data_pre_range
	{
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
		// Marker for looping until register 0x83 is not 00
		static constexpr uint8_t MARKER_WAIT_REG_DEVICE_STROBE = 2;
		// Marker after reading register 0x92 (SPAD info byte)
		static constexpr uint8_t MARKER_READ_SPAD_INFO = 3;

		// Write buffer
		static constexpr uint8_t BUFFER[] PROGMEM =
		{
			// get SPAD info
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
			// device strobe wait
			// actions::write(1), regs::REG_DEVICE_STROBE, 0x00,
			// // wait until done
			// actions::LOOP,
			// actions::read(1), regs::REG_DEVICE_STROBE, // 1 BYTE READ (loop wait until != 0x00 or "timeout")
			// actions::MARKER, MARKER_WAIT_REG_DEVICE_STROBE,
			// actions::write(1), regs::REG_DEVICE_STROBE, 0x01,

			actions::read(1), 0x92, // 1 BYTE READ: SPAD info byte
			actions::MARKER, MARKER_READ_SPAD_INFO,
			actions::write(1), 0x81, 0x00,
			actions::write(1), 0xFF, 0x06,
			actions::read(1), regs::REG_DEVICE_STROBE, // 1 BYTE READ
			actions::MARKER, MARKER_OVERWRITE_REG_DEVICE_STROBE,
			actions::write(1), regs::REG_DEVICE_STROBE, 0x04, // OVERWRITTEN BYTE
			actions::write(1), 0xFF, 0x01,
			actions::write(1), regs::REG_SYSRANGE_START, 0x01,
			actions::write(1), 0xFF, 0x00,
			actions::write(2, true), regs::REG_POWER_MANAGEMENT, 0x00,
			actions::END
		};

		// Value to force (OR) into register 0x83 at 2 occurrences
		static constexpr uint8_t REG_DEVICE_STROBE_FORCED_VALUE = 0x04;
	}
	
	// Constants for init_static() method
	//------------------------------------
	namespace init_static
	{
		// Write buffer
		static constexpr uint8_t INIT_STATIC_BUFFER[] PROGMEM =
		{
			// get SPAD info
			regs::REG_POWER_MANAGEMENT, 0x01,
			0xFF, 0x01,
			regs::REG_SYSRANGE_START, 0x00,
			0xFF, 0x06,
			0x83, // 1 BYTE READ
			0x83, 0x04, // OVERWRITTEN BYTE
			0xFF, 0x07,
			0x81, 0x01,
			regs::REG_POWER_MANAGEMENT, 0x01,
			0x94, 0x6B,
			0x83, 0x00,
			// wait until done
			0x83, // 1 BYTE READ (loop wait until != 0x00 or "timeout")
			0x83, 0x01,
			0x92, // 1 BYTE READ: SPAD info byte
			0x81, 0x00,
			0xFF, 0x06,
			0x83, // 1 BYTE READ
			0x83, 0x04, // OVERWRITTEN BYTE
			0xFF, 0x01,
			regs::REG_SYSRANGE_START, 0x01,
			0xFF, 0x00,
			regs::REG_POWER_MANAGEMENT, 0x00,

			// get reference SPADs from NVM
			regs::REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0, // 6 BYTES READ: RefGoodSpadMap

			// set reference SPADs (after calculation)
			0xFF, 0x01,
			regs::REG_DYNAMIC_SPAD_REF_EN_START_OFFSET, 0x00,
			regs::REG_DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD, 0x2C,
			0xFF, 0x00,
			regs::REG_GLOBAL_CONFIG_REF_EN_START_SELECT, 0xB4,
			regs::REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

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
			regs::REG_POWER_MANAGEMENT, 0x00,

			// set interrupt settings or not? what default?
			regs::REG_SYSTEM_INTERRUPT_CONFIG_GPIO, 0x04,
			regs::REG_GPIO_HV_MUX_ACTIVE_HIGH, // 1 BYTE READ
			regs::REG_GPIO_HV_MUX_ACTIVE_HIGH, 0x10, // OVERWRITTEN BYTE
			regs::REG_SYSTEM_INTERRUPT_CLEAR, 0x01,

			//TODO get current timing budget
				// get sequence steps
				// get sequence timeouts
			

			// set sequence steps: disable MSRC and TCC by default
			regs::REG_SYSTEM_SEQUENCE_CONFIG, 0xE8,

			//TODO recalculate timing budget
				// get sequence steps
				// get sequence timeouts
			regs::REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, 0x00, 0x00,
		};

		// Size of write buffer TODO maybe useless
		static constexpr uint8_t INIT_STATIC_BUFFER_SIZE = sizeof(INIT_STATIC_BUFFER);
		// List of futures and bytes count:
		// - negative means write 1 byte (register) and read N bytes
		// - positive means write 1 byte (register) followed by N bytes
		//FIXME list of futures...
		static constexpr int8_t INIT_STATIC_FUTURES[] PROGMEM = {
			// get SPAD info
			1, 1, 1, 1, -1, 1, 1, 1, 1, 1, 1, -1, 1, -1, 1, 1, -1, 1, 1, 1, 1, 1,
			// get reference SPADs from NVM
			-6,
			// set reference SPADs (after calculation)
			1, 1, 1, 1, 1, 6,
			// load tuning settings (hard-coded defaults) 80 writes
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
			// set interrupt settings?
			1, -1, 1, 1,
			// get current timing budget TODO

			// set sequence steps
			1,
			// recalculate timing budget TODO
		};
		static constexpr uint8_t INIT_STATIC_FUTURES_SIZE = sizeof(INIT_STATIC_FUTURES);
		//TODO INDEX...
	}
	
	// TODO define specific namespace for each specific Future data

}
#endif /* VL53L0X_INTERNALS_H */
/// @endcond
