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

#ifndef VL53L0X_HELPERS_H
#define VL53L0X_HELPERS_H

#include <stdint.h>
#include "../bits.h"

namespace devices::vl53l0x_helpers
{
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

	// Bits definitions for sequence steps
	struct Steps
	{
		static constexpr uint8_t TCC = bits::BV8(4);
		static constexpr uint8_t DSS = bits::BV8(3);
		static constexpr uint8_t MSRC = bits::BV8(2);
		static constexpr uint8_t PRE_RANGE = bits::BV8(6);
		static constexpr uint8_t FINAL_RANGE = bits::BV8(7);
	};

	// Utility methods to convert timeouts mclks<->us
	class TimeoutUtilities
	{
	private:
		static constexpr const uint32_t PLL_PERIOD_PS        = 1655UL;
		static constexpr const uint32_t MACRO_PERIOD_VCLKS   = 2304UL;

	public:
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
	};

	//FIXME remove this redundant class
	// Sequence steps timeout values and methods
	class StepsTimeout
	{
	public:
		StepsTimeout() = default;
		StepsTimeout(uint8_t pre_range_vcsel_period_pclks, uint8_t final_range_vcsel_period_pclks,
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
			//FIXME actual result depends on step enable pre_range
			// if (is_pre_range())
			// 	return final_range_mclks_ - pre_range_mclks_;
			return final_range_mclks_ + 1;
		}

		// Following values are calculated from others
		uint32_t msrc_dss_tcc_us() const
		{
			return TimeoutUtilities::calculate_timeout_us(msrc_dss_tcc_mclks(), pre_range_vcsel_period_pclks());
		}

		uint32_t pre_range_us() const
		{
			return TimeoutUtilities::calculate_timeout_us(pre_range_mclks(), pre_range_vcsel_period_pclks());
		}

		uint32_t final_range_us() const
		{
			return TimeoutUtilities::calculate_timeout_us(final_range_mclks(), final_range_vcsel_period_pclks());
		}

	private:
		uint8_t pre_range_vcsel_period_pclks_ = 0;
		uint8_t final_range_vcsel_period_pclks_ = 0;

		uint8_t msrc_dss_tcc_mclks_ = 0;
		uint16_t pre_range_mclks_ = 0;
		uint16_t final_range_mclks_ = 0;
	};

	// Find another place (depends on vl53l0x_types.h!)
	class TimingBudgetUtilities
	{
	private:
		static constexpr const uint32_t MIN_TIMING_BUDGET    = 20000UL;
		static constexpr const uint16_t START_OVERHEAD       = 1910U;
		static constexpr const uint16_t END_OVERHEAD         = 960U;
		static constexpr const uint16_t MSRC_OVERHEAD        = 660U;
		static constexpr const uint16_t TCC_OVERHEAD         = 590U;
		static constexpr const uint16_t DSS_OVERHEAD         = 690U;
		static constexpr const uint16_t PRE_RANGE_OVERHEAD   = 660U;
		static constexpr const uint16_t FINAL_RANGE_OVERHEAD = 550U;

	public:
		static uint32_t calculate_measurement_timing_budget_us(
			uint8_t steps, const StepsTimeout& timeouts)
		{
			// start and end overhead times always present
			uint32_t budget_us = START_OVERHEAD + END_OVERHEAD;

			if (steps & Steps::TCC)
				budget_us += timeouts.msrc_dss_tcc_us() + TCC_OVERHEAD;

			if (steps & Steps::DSS)
				budget_us += 2 * (timeouts.msrc_dss_tcc_us() + DSS_OVERHEAD);
			else if (steps & Steps::MSRC)
				budget_us += timeouts.msrc_dss_tcc_us() + MSRC_OVERHEAD;

			if (steps & Steps::PRE_RANGE)
				budget_us += timeouts.pre_range_us() + PRE_RANGE_OVERHEAD;

			if (steps & Steps::FINAL_RANGE)
				budget_us += timeouts.final_range_us() + FINAL_RANGE_OVERHEAD;

			return budget_us;
		}

		static uint16_t calculate_final_range_timeout_mclks(
			uint8_t steps, const StepsTimeout& timeouts, uint32_t budget_us)
		{
			// Requested budget must be be above minimum allowed
			if (budget_us < MIN_TIMING_BUDGET) return 0;
			// This calculation is useless if there is no final range step
			if (!(steps & Steps::FINAL_RANGE)) return 0;

			// Calculate current used budget without final range
			uint32_t used_budget_us = calculate_measurement_timing_budget_us(
				(steps & (~Steps::FINAL_RANGE)), timeouts);

			// Now include final range and calculate difference
			used_budget_us += FINAL_RANGE_OVERHEAD;
			// Requested budget must be above calculated budget for all other steps
			if (used_budget_us > budget_us) return 0;

			// Calculate final range timeout in us
			const uint32_t final_range_timeout_us = budget_us - used_budget_us;

			//TODO finalize
			// Deduce final range timeout in mclks
			uint32_t final_range_timeout_mclks = 0;
		}

	};

}
#endif /* VL53L0X_HELPERS_H */
/// @endcond
