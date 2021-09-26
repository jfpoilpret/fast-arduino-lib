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
#include "../streams.h"
#include "../utilities.h"
#include "vl53l0x_registers.h"

//TODO DOCS for each type
namespace devices::vl53l0x
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

	/// @cond notdocumented
	class TimeoutUtilities
	{
	private:
		static constexpr uint32_t PLL_PERIOD_PS = 1655UL;
		static constexpr uint32_t MACRO_PERIOD_VCLKS = 2304UL;

	public:
		// timeout in macro periods must be encoded on 16 bits, as (LSB.2^MSB)+1
		static constexpr uint16_t encode_timeout(uint32_t timeout_macro_clks)
		{
			if (timeout_macro_clks == 0UL) return 0U;
			uint32_t lsb = timeout_macro_clks - 1UL;
			uint16_t msb = 0;
			while (lsb & 0xFFFFFF00UL)
			{
				lsb >>= 1;
				++msb;
			}
			return utils::as_uint16_t(uint8_t(msb), uint8_t(lsb & 0xFFUL));
		}

		// timeout in macro periods is encoded on 16 bits, as (LSB.2^MSB)+1
		static constexpr uint32_t decode_timeout(uint16_t encoded_timeout)
		{
			const uint32_t lsb = utils::low_byte(encoded_timeout);
			const uint32_t msb = utils::high_byte(encoded_timeout);
			return (lsb << msb) + 1UL;
		}

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
	/// @endcond

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
	streams::ostream& operator<<(streams::ostream&, DeviceError);

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
	streams::ostream& operator<<(streams::ostream&, DeviceStatus);

	enum class PowerMode : uint8_t
	{
		STANDBY = 0,
		IDLE = 1
	};
	streams::ostream& operator<<(streams::ostream&, PowerMode);

	enum class GPIOFunction : uint8_t
	{
		DISABLED = 0x00,
		LEVEL_LOW = 0x01,
		LEVEL_HIGH = 0x02,
		OUT_OF_WINDOW = 0x03,
		SAMPLE_READY = 0x04
	};
	streams::ostream& operator<<(streams::ostream&, GPIOFunction);

	class GPIOSettings
	{
	public:
		constexpr GPIOSettings() = default;
		constexpr GPIOSettings(GPIOFunction function, bool high_polarity, 
			uint16_t low_threshold = 0, uint16_t high_threshold = 0)
			:	function_{function}, high_polarity_{high_polarity}, 
				low_threshold_{low_threshold}, high_threshold_{high_threshold} {}

		static constexpr GPIOSettings sample_ready(bool high_polarity = false)
		{
			return GPIOSettings{GPIOFunction::SAMPLE_READY, high_polarity};
		}
		static constexpr GPIOSettings low_threshold(uint16_t threshold, bool high_polarity = false)
		{
			return GPIOSettings{GPIOFunction::LEVEL_LOW, high_polarity, threshold};
		}
		static constexpr GPIOSettings high_threshold(uint16_t threshold, bool high_polarity = false)
		{
			return GPIOSettings{GPIOFunction::LEVEL_HIGH, high_polarity, 0, threshold};
		}
		static constexpr GPIOSettings out_of_window(
			uint16_t low_threshold, uint16_t high_threshold, bool high_polarity = false)
		{
			return GPIOSettings{GPIOFunction::OUT_OF_WINDOW, high_polarity, low_threshold, high_threshold};
		}

		GPIOFunction function() const
		{
			return function_;
		}
		bool high_polarity() const
		{
			return high_polarity_;
		}
		uint16_t low_threshold() const
		{
			return low_threshold_;
		}
		uint16_t high_threshold() const
		{
			return high_threshold_;
		}

	private:
		GPIOFunction function_ = GPIOFunction::DISABLED;
		bool high_polarity_ = false;
		uint16_t low_threshold_ = 0;
		uint16_t high_threshold_ = 0;
	};
	streams::ostream& operator<<(streams::ostream&, const GPIOSettings&);

	class InterruptStatus
	{
	public:
		InterruptStatus() = default;
		operator uint8_t() const
		{
			return status_ & 0x07;
		}

	private:
		uint8_t status_ = 0;
	};

	class SPADReference
	{
	public:
		SPADReference() = default;
		SPADReference(const uint8_t spad_refs[6])
		{
			memcpy(spad_refs_, spad_refs, 6);
		}
		const uint8_t* spad_refs() const
		{
			return spad_refs_;
		}
		uint8_t* spad_refs()
		{
			return spad_refs_;
		}

	private:
		uint8_t spad_refs_[6];
	};

	enum class SingleRefCalibrationTarget : uint8_t
	{
		PHASE_CALIBRATION = 0x01,
		VHV_CALIBRATION = 0x41
	};

	enum class VcselPeriodType : uint8_t
	{
		PRE_RANGE = uint8_t(vl53l0x::Register::PRE_RANGE_CONFIG_VCSEL_PERIOD),
		FINAL_RANGE = uint8_t(vl53l0x::Register::FINAL_RANGE_CONFIG_VCSEL_PERIOD)
	};

	//TODO Document each step: what it does, its impact on measurements and timing
	class SequenceSteps
	{
	private:
		static constexpr uint8_t FORCED_BITS = bits::BV8(5);
		// TCC Target Center Check
		static constexpr uint8_t TCC = bits::BV8(4);
		// DSS Dynamic Spad Selection
		static constexpr uint8_t DSS = bits::BV8(3);
		// MSRC Minimum Signal Rate Check
		static constexpr uint8_t MSRC = bits::BV8(2);
		// PRE_RANGE Pre-Range Check
		static constexpr uint8_t PRE_RANGE = bits::BV8(6);
		// FINAL_RANGE Final-Range Check
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

		constexpr SequenceSteps tcc() const
		{
			return SequenceSteps{uint8_t(steps_ | TCC)};
		}
		constexpr SequenceSteps dss() const
		{
			return SequenceSteps{uint8_t(steps_ | DSS)};
		}
		constexpr SequenceSteps msrc() const
		{
			return SequenceSteps{uint8_t(steps_ | MSRC)};
		}
		constexpr SequenceSteps pre_range() const
		{
			return SequenceSteps{uint8_t(steps_ | PRE_RANGE)};
		}
		constexpr SequenceSteps final_range() const
		{
			return SequenceSteps{uint8_t(steps_ | FINAL_RANGE)};
		}

		constexpr SequenceSteps no_tcc() const
		{
			return SequenceSteps{uint8_t(steps_ & ~TCC)};
		}
		constexpr SequenceSteps no_dss() const
		{
			return SequenceSteps{uint8_t(steps_ & ~DSS)};
		}
		constexpr SequenceSteps no_msrc() const
		{
			return SequenceSteps{uint8_t(steps_ & ~MSRC)};
		}
		constexpr SequenceSteps no_pre_range() const
		{
			return SequenceSteps{uint8_t(steps_ & ~PRE_RANGE)};
		}
		constexpr SequenceSteps no_final_range() const
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
		constexpr SequenceSteps(uint8_t steps) : steps_{uint8_t(steps | FORCED_BITS)} {}

		uint8_t steps_ = FORCED_BITS;

		template<typename MANAGER> friend class VL53L0X;
	};

	streams::ostream& operator<<(streams::ostream&, SequenceSteps);

	class SequenceStepsTimeout
	{
	public:
		SequenceStepsTimeout() = default;

		uint8_t pre_range_vcsel_period_pclks() const
		{
			return pre_range_vcsel_period_pclks_;
		}
		uint8_t final_range_vcsel_period_pclks() const
		{
			return final_range_vcsel_period_pclks_;
		}
		uint16_t msrc_dss_tcc_mclks() const
		{
			return msrc_dss_tcc_mclks_ + 1;
		}
		uint16_t pre_range_mclks() const
		{
			return TimeoutUtilities::decode_timeout(pre_range_mclks_);
		}
		uint16_t final_range_mclks(bool is_pre_range) const
		{
			uint16_t temp_final_range_mclks = TimeoutUtilities::decode_timeout(final_range_mclks_);
			return (is_pre_range ? temp_final_range_mclks - pre_range_mclks() : temp_final_range_mclks);
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

		uint32_t final_range_us(bool is_pre_range) const
		{
			return TimeoutUtilities::calculate_timeout_us(
				final_range_mclks(is_pre_range), final_range_vcsel_period_pclks());
		}

	private:
		SequenceStepsTimeout(uint8_t pre_range_vcsel_period_pclks, uint8_t final_range_vcsel_period_pclks,
			uint8_t msrc_dss_tcc_mclks, uint16_t pre_range_mclks, uint16_t final_range_mclks)
			:	pre_range_vcsel_period_pclks_{pre_range_vcsel_period_pclks},
				final_range_vcsel_period_pclks_{final_range_vcsel_period_pclks},
				msrc_dss_tcc_mclks_{msrc_dss_tcc_mclks},
				pre_range_mclks_{pre_range_mclks},
				final_range_mclks_{final_range_mclks} {}

		uint8_t pre_range_vcsel_period_pclks_ = 0;
		uint8_t final_range_vcsel_period_pclks_ = 0;

		uint8_t msrc_dss_tcc_mclks_ = 0;
		uint16_t pre_range_mclks_ = 0;
		uint16_t final_range_mclks_ = 0;

		template<typename MANAGER> friend class VL53L0X;
	};

	streams::ostream& operator<<(streams::ostream&, const SequenceStepsTimeout&);

	class SPADInfo
	{
	private:
		static constexpr uint8_t APERTURE = bits::BV8(7);
		static constexpr uint8_t COUNT = bits::CBV8(7);

	public:
		SPADInfo() = default;
		SPADInfo(uint8_t info) : info_{info} {}

		bool is_aperture() const
		{
			return info_ & APERTURE;
		}

		uint8_t count() const
		{
			return info_ & COUNT;
		}

	private:
		uint8_t info_ = 0;
	};

	streams::ostream& operator<<(streams::ostream&, SPADInfo);
}

#endif /* VL53L0X_TYPES_H */
/// @endcond
