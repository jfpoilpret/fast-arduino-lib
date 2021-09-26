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

#ifndef VL53L0X_REGISTERS_H
#define VL53L0X_REGISTERS_H

#include <stdint.h>

namespace devices::vl53l0x
{
	enum class Register : uint8_t
	{
		SYSRANGE_START                              = 0x00,
		SYSTEM_SEQUENCE_CONFIG                      = 0x01,
		SYSTEM_INTERMEASUREMENT_PERIOD              = 0x04,
		SYSTEM_RANGE_CONFIG                         = 0x09,
		SYSTEM_INTERRUPT_CONFIG_GPIO                = 0x0A,
		SYSTEM_INTERRUPT_CLEAR                      = 0x0B,
		SYSTEM_THRESH_HIGH                          = 0x0C,
		SYSTEM_THRESH_LOW                           = 0x0E,
		RESULT_INTERRUPT_STATUS                     = 0x13,
		RESULT_RANGE_STATUS                         = 0x14,
		RESULT_EFFECTIVE_SPAD_RTN_COUNT				= 0x16,
		RESULT_PEAK_SIGNAL_COUNT_RATE_RTN_MCPS		= 0x1A,
		RESULT_AMBIENT_RATE_RTN_MCPS				= 0x1C,
		RESULT_RANGE_MILLIMETER						= 0x1E,
		CROSSTALK_COMPENSATION_PEAK_RATE_MCPS       = 0x20,
		PRE_RANGE_CONFIG_MIN_SNR                    = 0x27,
		ALGO_PART_TO_PART_RANGE_OFFSET_MM           = 0x28,
		ALGO_PHASECAL_LIM                           = 0x30, // Extended register index (surround access with 0xFF-0x01/0xFF-0x00)
		ALGO_PHASECAL_CONFIG_TIMEOUT                = 0x30,
		GLOBAL_CONFIG_VCSEL_WIDTH                   = 0x32,
		HISTOGRAM_CONFIG_INITIAL_PHASE_SELECT       = 0x33,
		FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT = 0x44,
		MSRC_CONFIG_TIMEOUT_MACROP                  = 0x46,
		FINAL_RANGE_CONFIG_VALID_PHASE_LOW          = 0x47,
		FINAL_RANGE_CONFIG_VALID_PHASE_HIGH         = 0x48,
		DYNAMIC_SPAD_NUM_REQUESTED_REF_SPAD         = 0x4E, // Extended register index (surround access with 0xFF-0x01/0xFF-0x00)
		DYNAMIC_SPAD_REF_EN_START_OFFSET            = 0x4F, // Extended register index (surround access with 0xFF-0x01/0xFF-0x00)
		PRE_RANGE_CONFIG_VCSEL_PERIOD               = 0x50,
		PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI          = 0x51,
		PRE_RANGE_CONFIG_TIMEOUT_MACROP_LO          = 0x52,
		HISTOGRAM_CONFIG_READOUT_CTRL               = 0x55,
		PRE_RANGE_CONFIG_VALID_PHASE_LOW            = 0x56,
		PRE_RANGE_CONFIG_VALID_PHASE_HIGH           = 0x57,
		MSRC_CONFIG_CONTROL                         = 0x60,
		PRE_RANGE_CONFIG_SIGMA_THRESH_HI            = 0x61,
		PRE_RANGE_CONFIG_SIGMA_THRESH_LO            = 0x62,
		PRE_RANGE_MIN_COUNT_RATE_RTN_LIMIT          = 0x64,
		FINAL_RANGE_CONFIG_MIN_SNR                  = 0x67,
		FINAL_RANGE_CONFIG_VCSEL_PERIOD             = 0x70,
		FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI        = 0x71,
		FINAL_RANGE_CONFIG_TIMEOUT_MACROP_LO        = 0x72,
		POWER_MANAGEMENT                            = 0x80,
		SYSTEM_HISTOGRAM_BIN                        = 0x81,
		DEVICE_STROBE                               = 0x83,
		GPIO_HV_MUX_ACTIVE_HIGH                     = 0x84,
		SYSTEM_CONFIG_I2C_MODE						= 0x88,
		VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV            = 0x89,
		I2C_SLAVE_DEVICE_ADDRESS                    = 0x8A,
		SYSTEM_STOP_VARIABLE						= 0x91,
		SPAD_INFO                                   = 0x92,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_0            = 0xB0,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_1            = 0xB1,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_2            = 0xB2,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_3            = 0xB3,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_4            = 0xB4,
		GLOBAL_CONFIG_SPAD_ENABLES_REF_5            = 0xB5,
		GLOBAL_CONFIG_REF_EN_START_SELECT           = 0xB6,
		RESULT_PEAK_SIGNAL_RATE_REF                 = 0xB6, // Extended register index (surround access with 0xFF-0x01/0xFF-0x00)
		RESULT_CORE_AMBIENT_WINDOW_EVENTS_RTN       = 0xBC,
		SOFT_RESET_GO2_SOFT_RESET_N                 = 0xBF,
		RESULT_CORE_RANGING_TOTAL_EVENTS_RTN        = 0xC0,	// Unused in original STM API (conflicts with next, maybe extended register index)
		IDENTIFICATION_MODEL_ID                     = 0xC0,
		IDENTIFICATION_REVISION_ID                  = 0xC2,
		RESULT_CORE_AMBIENT_WINDOW_EVENTS_REF       = 0xD0,
		RESULT_CORE_RANGING_TOTAL_EVENTS_REF        = 0xD4,
		OSC_CALIBRATE_VAL                           = 0xF8,
	};
}

#endif /* VL53L0X_REGISTERS_H */
/// @endcond
