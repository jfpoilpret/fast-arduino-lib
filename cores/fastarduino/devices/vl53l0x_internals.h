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
	}

	namespace regs = registers;

	// Constants for init_data() method
	//----------------------------------
	// Write buffer
	static constexpr uint8_t INIT_DATA_BUFFER[] PROGMEM =
	{
		// read/write VHV_CONFIG_PAD_SCL_SDA__EXTSUP_HV to force 2.8V or keep 1.8V default
		regs::REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, // 1 BYTE READ
		regs::REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV, 0x01, // OVERWRITTEN BYTE
		// set I2C standard mode
		0x88, 0x00,
		regs::REG_POWER_MANAGEMENT, 0x01,
		0xFF, 0x01,
		regs::REG_SYSRANGE_START, 0x00,
		// read stop variable here
		0x91, // 1 BYTE READ
		regs::REG_SYSRANGE_START, 0x01,
		0xFF, 0x00,
		regs::REG_POWER_MANAGEMENT, 0x00,
		// read/write REG_MSRC_CONFIG_CONTROL to disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE limit checks
		regs::REG_MSRC_CONFIG_CONTROL, // 1 BYTE READ
		regs::REG_MSRC_CONFIG_CONTROL, 0x12, // OVERWRITTEN BYTE
		// set signal rate limit to 0.25 MCPS (million counts per second) in FP9.7 format
		regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, 0x00, 0x20,
		// enable all sequence steps by default
		regs::REG_SYSTEM_SEQUENCE_CONFIG, 0xFF
	};
	// Size of write buffer
	static constexpr uint8_t INIT_DATA_BUFFER_WRITE_SIZE = sizeof(INIT_DATA_BUFFER);
	// Size of read buffer
	static constexpr uint8_t INIT_DATA_BUFFER_READ_SIZE = 3;
	// List of bytes count for each read/write (read is negative, write is positive)
	static constexpr int8_t INIT_DATA_BUFFER_R_W[] PROGMEM = {1, -1, 2, 2, 2, 2, 2, 1, -1, 2, 2, 2, 1, -1, 2, 3, 2};
	static constexpr uint8_t INIT_DATA_BUFFER_RW_SIZE = sizeof(INIT_DATA_BUFFER_R_W);

	// Index of value to read in future output buffer for REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV
	static constexpr uint8_t INIT_DATA_BUFFER_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_READ_INDEX = 0;
	// Index of value to write in future input buffer for REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV
	static constexpr uint8_t INIT_DATA_BUFFER_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_WRITE_INDEX = 2;

	// Index of value to read in future output buffer for stop variable
	static constexpr uint8_t INIT_DATA_BUFFER_STOP_VARIABLE_READ_INDEX = 1;

	// Index of value to read in future output buffer for REG_MSRC_CONFIG_CONTROL
	static constexpr uint8_t INIT_DATA_BUFFER_MSRC_CONFIG_CONTROL_READ_INDEX = 2;
	// Index of value to write in future input buffer for REG_MSRC_CONFIG_CONTROL
	static constexpr uint8_t INIT_DATA_BUFFER_MSRC_CONFIG_CONTROL_WRITE_INDEX = 20;

	// Value to force 2V8 in REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV register
	static constexpr uint8_t VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_SET_2V8 = 0x01;
	// Value to disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE in REG_MSRC_CONFIG_CONTROL register
	static constexpr uint8_t MSRC_CONFIG_CONTROL_INIT = 0x12;

	// Constants for init_static() method
	//-----------------------------------
	// Write buffer
	static constexpr uint8_t INIT_STATIC_BUFFER[] PROGMEM =
	{
		
	};
}

#endif /* VL53L0X_INTERNALS_H */
/// @endcond
