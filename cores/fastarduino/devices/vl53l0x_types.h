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
 * API to handle Time-of-Flight ranging sensor VL53L0X I2C chip.
 * This header defines all specific types used by VL53L0X device class.
 * Note that most API here has been adapted and improved from official 
 * STMicroelectronics C-library API; this was necessary as the device datasheet
 * does not describe the internals (registers) of the chip, the only way to
 * understand how it works was thus to analyze the API source code.
 * 
 * @sa https://www.st.com/content/st_com/en/products/embedded-software/proximity-sensors-software/stsw-img005.html
 */

#ifndef VL53L0X_TYPES_H
#define VL53L0X_TYPES_H

#include "../bits.h"
#include "vl53l0x_internals.h"

namespace devices::vl53l0x
{
	/// @cond notdocumented
	namespace regs = vl53l0x_internals::registers;
	/// @endcond

	// static utilities to support fixed point 9/7 bits used by VL53L0X chip
	class FixPoint9_7
	{
	public:
		static constexpr bool is_valid(float value)
		{
			return ((value >= 0.0) && (value < float(1 << INTEGRAL_BITS)));
		}

		static constexpr uint16_t convert(float value)
		{
			return is_valid(value) ? uint16_t(value * (1 << DECIMAL_BITS)) : 0U;
		}

		static constexpr float convert(uint16_t value)
		{
			return value / float(1 << DECIMAL_BITS);
		}

	private:
		static constexpr uint16_t INTEGRAL_BITS = 9;
		static constexpr uint16_t DECIMAL_BITS = 7;
	};

	enum class DeviceError : uint8_t
	{
		NONE                           = 0,
		VCSEL_CONTINUITY_TEST_FAILURE  = 1,
		VCSEL_WATCHDOG_TEST_FAILURE    = 2,
		NO_VHV_VALUE_FOUND             = 3,
		MSRC_NO_TARGET                 = 4,
		SNR_CHECK                      = 5,
		RANGE_PHASE_CHECK              = 6,
		SIGMA_THRESHOLD_CHECK          = 7,
		TCC                            = 8,
		PHASE_CONSISTENCY              = 9,
		MIN_CLIP                       = 10,
		RANGE_COMPLETE                 = 11,
		ALGO_UNDERFLOW                 = 12,
		ALGO_OVERFLOW                  = 13,
		RANGE_IGNORE_THRESHOLD         = 14,
		UNKNOWN                        = 15
	};

	class DeviceStatus
	{
	public:
		DeviceStatus() = default;
		DeviceError error() const
		{
			return DeviceError((status_ >> 3) & 0x0F);
		}
		bool data_ready() const
		{
			return status_ & 0x01;
		}

	private:
		uint8_t status_ = 0;
	};

	enum class PowerMode : uint8_t
	{
		STANDBY = 0,
		IDLE = 1
	};

	//TODO improve by setting list of available values for period_pclks?
	enum class VcselPeriodType : uint8_t
	{
		PRE_RANGE = regs::REG_PRE_RANGE_CONFIG_VCSEL_PERIOD,
		FINAL_RANGE = regs::REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD
	};

	class SequenceSteps
	{
	private:
		static constexpr uint8_t TCC = bits::BV8(4);
		static constexpr uint8_t DSS = bits::BV8(3);
		static constexpr uint8_t MSRC = bits::BV8(2);
		static constexpr uint8_t PRE_RANGE = bits::BV8(6);
		static constexpr uint8_t FINAL_RANGE = bits::BV8(7);

	public:
		static constexpr SequenceSteps create()
		{
			return SequenceSteps{};
		}
		static constexpr SequenceSteps all()
		{
			return SequenceSteps{TCC | DSS | MSRC | PRE_RANGE | FINAL_RANGE};
		}

		constexpr SequenceSteps() = default;

		constexpr SequenceSteps tcc()
		{
			return SequenceSteps{uint8_t(steps_ | TCC)};
		}
		constexpr SequenceSteps dss()
		{
			return SequenceSteps{uint8_t(steps_ | DSS)};
		}
		constexpr SequenceSteps msrc()
		{
			return SequenceSteps{uint8_t(steps_ | MSRC)};
		}
		constexpr SequenceSteps pre_range()
		{
			return SequenceSteps{uint8_t(steps_ | PRE_RANGE)};
		}
		constexpr SequenceSteps final_range()
		{
			return SequenceSteps{uint8_t(steps_ | FINAL_RANGE)};
		}

		constexpr SequenceSteps no_tcc()
		{
			return SequenceSteps{uint8_t(steps_ & ~TCC)};
		}
		constexpr SequenceSteps no_dss()
		{
			return SequenceSteps{uint8_t(steps_ & ~DSS)};
		}
		constexpr SequenceSteps no_msrc()
		{
			return SequenceSteps{uint8_t(steps_ & ~MSRC)};
		}
		constexpr SequenceSteps no_pre_range()
		{
			return SequenceSteps{uint8_t(steps_ & ~PRE_RANGE)};
		}
		constexpr SequenceSteps no_final_range()
		{
			return SequenceSteps{uint8_t(steps_ & ~FINAL_RANGE)};
		}

		uint8_t value() const
		{
			return steps_;
		}
		bool is_tcc() const
		{
			return steps_ & TCC;
		}
		bool is_dss() const
		{
			return steps_ & DSS;
		}
		bool is_msrc() const
		{
			return steps_ & MSRC;
		}
		bool is_pre_range() const
		{
			return steps_ & PRE_RANGE;
		}
		bool is_final_range() const
		{
			return steps_ & FINAL_RANGE;
		}

	private:
		constexpr SequenceSteps(uint8_t steps) : steps_{steps} {}

		uint8_t steps_ = 0;
	};

	class SequenceStepsTimeout
	{
	public:
		//TODO static method to calculate SequenceStepsTimeout from other values
		SequenceStepsTimeout() = default;
		//FIXME should be private and VL53L0X class declared friend
		SequenceStepsTimeout(uint8_t pre_range_vcsel_period_pclks, uint8_t final_range_vcsel_period_pclks,
			uint8_t msrc_dss_tcc_mclks, uint16_t pre_range_mclks, uint16_t final_range_mclks)
			:	pre_range_vcsel_period_pclks_{pre_range_vcsel_period_pclks},
				final_range_vcsel_period_pclks_{final_range_vcsel_period_pclks},
				msrc_dss_tcc_mclks_{msrc_dss_tcc_mclks},
				pre_range_mclks_{pre_range_mclks},
				final_range_mclks_{final_range_mclks} {}

		uint8_t pre_range_vcsel_period_pclks() const
		{
			return (pre_range_vcsel_period_pclks_ + 1) << 1;
		}
		uint8_t final_range_vcsel_period_pclks() const
		{
			return (final_range_vcsel_period_pclks_ + 1) << 1;
		}
		uint16_t msrc_dss_tcc_mclks() const
		{
			return msrc_dss_tcc_mclks_ + 1;
		}
		uint16_t pre_range_mclks() const
		{
			return pre_range_mclks_ + 1;
		}
		uint16_t final_range_mclks() const
		{
			return final_range_mclks_ + 1;
		}

		// Following values are calculated from others
		uint32_t msrc_dss_tcc_us() const
		{
			return calculate_timeout_us(msrc_dss_tcc_mclks(), pre_range_vcsel_period_pclks());
		}

		uint32_t pre_range_us() const
		{
			return calculate_timeout_us(pre_range_mclks(), pre_range_vcsel_period_pclks());
		}

		uint32_t final_range_us() const
		{
			return calculate_timeout_us(final_range_mclks(), final_range_vcsel_period_pclks());
		}

	private:
		static constexpr uint32_t PLL_PERIOD_PS = 1655UL;
		static constexpr uint32_t MACRO_PERIOD_VCLKS = 2304UL;

		static constexpr uint32_t calculate_macro_period_ps(uint8_t vcsel_period_pclks)
		{
			return ((PLL_PERIOD_PS * MACRO_PERIOD_VCLKS * vcsel_period_pclks) + 500UL) / 1000UL;
		}
		static constexpr uint32_t calculate_timeout_us(uint16_t timeout_period_mclks, uint8_t vcsel_period_pclks)
		{
			const uint32_t macro_period_ns = calculate_macro_period_ps(vcsel_period_pclks);
			return ((timeout_period_mclks * macro_period_ns) + 500UL) / 1000UL;
		}
		static constexpr uint32_t calculate_timeout_mclks(uint16_t timeout_period_us, uint8_t vcsel_period_pclks)
		{
			const uint32_t macro_period_ns = calculate_macro_period_ps(vcsel_period_pclks);
			return ((timeout_period_us * 1000UL) + (macro_period_ns / 2)) / macro_period_ns;
		}

		uint8_t pre_range_vcsel_period_pclks_ = 0;
		uint8_t final_range_vcsel_period_pclks_ = 0;

		uint8_t msrc_dss_tcc_mclks_ = 0;
		uint16_t pre_range_mclks_ = 0;
		uint16_t final_range_mclks_ = 0;
	};
}

#endif /* VL53L0X_TYPES_H */
/// @endcond
