//   Copyright 2016-2021 Jean-Francois Poilpret
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
 * Most types defined here have an associated `operator<<` to display them
 * clearly to an output stream.
 * 
 * @sa https://www.st.com/content/st_com/en/products/embedded-software/proximity-sensors-software/stsw-img005.html
 */

#ifndef VL53L0X_TYPES_H
#define VL53L0X_TYPES_H

#include "../bits.h"
#include "../streams.h"
#include "../utilities.h"
#include "vl53l0x_registers.h"

namespace devices::vl53l0x
{
	/**
	 * Helper class to handle VL53L0X special fix-point 9.7 values.
	 * It provides 3 utility methods for conversion and validity checks.
	 * This class is used internally by `vl53l0x::VL53L0X` class and is
	 * normally not useful to application developers.
	 * Representation of 9.7 fix-point values is done as an `uint16_t`.
	 */
	class FixPoint9_7
	{
	public:
		/**
		 * Check that a `float` value is valid for conversion to 9.7 fix-point.
		 * @param value the value to check (as float)
		 * @retval true if @p value is convertible to 9.7 fix-point.
		 * @retval false if @p value is not convertible to 9.7 fix-point (negative 
		 * or too big).
		 */
		static constexpr bool is_valid(float value)
		{
			return ((value >= 0.0) && (value < float(1 << INTEGRAL_BITS)));
		}

		/**
		 * Convert a float value into an 9.7 fix-point.
		 * @param value the value to covnert (as float)
		 * @retval 0 if @p value is not covnertible to 9.7 fix-point
		 * @return uint16_t 9.7 fix-point conversion of @p value if valid
		 * 
		 * @sa is_valid()
		 */
		static constexpr uint16_t convert(float value)
		{
			return is_valid(value) ? uint16_t(value * (1 << DECIMAL_BITS)) : 0U;
		}

		/**
		 * Convert an 9.7 fix-point value into a float.
		 * @param value uint16_t 9.7 fix-point value to convert to float
		 * @return float representation of @p value
		 */
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
			while (lsb & 0xFF'FF'FF'00UL)
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

	/**
	 * Possible error codes returned by VL53L0X device.
	 * 
	 * @sa DeviceStatus
	 * @sa VL53L0X::get_range_status()
	 */
	enum class DeviceError : uint8_t
	{
		/** No error */
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
		/** Range completed, range value is available for reading. */
		RANGE_COMPLETE                 = 11,
		ALGO_UNDERFLOW                 = 12,
		ALGO_OVERFLOW                  = 13,
		RANGE_IGNORE_THRESHOLD         = 14,
		/** Unknown error. */
		UNKNOWN                        = 15
	};
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, DeviceError error);
	/// @endcond

	/**
	 * Status of device as retrieved by `VL53L0X::get_range_status()`.
	 * 
	 * @sa VL53L0X::get_range_status()
	 */
	class DeviceStatus
	{
	public:
		/// @cond notdocumented
		DeviceStatus() = default;
		/// @endcond

		/**
		 * Device error.
		 */
		DeviceError error() const
		{
			return DeviceError((status_ >> 3) & 0x0F);
		}

		/**
		 * Indicate if data (range) is ready for reading.
		 * @retval true if data can be read from the device
		 */
		bool data_ready() const
		{
			return status_ & 0x01;
		}

	private:
		uint8_t status_ = 0;
	};
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, DeviceStatus status);
	/// @endcond

	/**
	 * Possible power modes of VL53L0X device as returned by `VL53L0X::get_power_mode()`.
	 * @sa VL53L0X::get_power_mode()
	 */
	enum class PowerMode : uint8_t
	{
		STANDBY = 0,
		IDLE = 1
	};
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, PowerMode mode);
	/// @endcond

	/**
	 * Possible triggers for VL53L0X GPIO pin.
	 * @sa GPIOSettings
	 * @sa VL53L0X::get_GPIO_settings()
	 * @sa VL53L0X::set_GPIO_settings()
	 */
	enum class GPIOFunction : uint8_t
	{
		/** No interrupt triggered on GPIO pin. */
		DISABLED = 0x00,
		/** Interrupt triggered when range is under a low threshold. */
		LEVEL_LOW = 0x01,
		/** Interrupt triggered when range is above a high threshold. */
		LEVEL_HIGH = 0x02,
		/** Interrupt triggered when range is outside a window between low and high thresholds. */
		OUT_OF_WINDOW = 0x03,
		/** Interrupt triggered when a range is ready to read. */
		SAMPLE_READY = 0x04
	};
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, GPIOFunction function);
	/// @endcond

	/**
	 * Settings for behavior of VL53L0X GPIO pin.
	 * @sa GPIOFunction
	 * @sa VL53L0X::get_GPIO_settings()
	 * @sa VL53L0X::set_GPIO_settings()
	 */
	class GPIOSettings
	{
	public:
		/// @cond notdocumented
		constexpr GPIOSettings() = default;
		constexpr GPIOSettings(GPIOFunction function, bool high_polarity, 
			uint16_t low_threshold = 0, uint16_t high_threshold = 0)
			:	function_{function}, high_polarity_{high_polarity}, 
				low_threshold_{low_threshold}, high_threshold_{high_threshold} {}
		/// @endcond

		/**
		 * Create GPIOSettings for interrupt triggered when range sample is ready.
		 * @param high_polarity force GPIO interrupt polarity to HIGH; this is not
		 * advised as most breakouts include a pullup resistor.
		 */
		static constexpr GPIOSettings sample_ready(bool high_polarity = false)
		{
			return GPIOSettings{GPIOFunction::SAMPLE_READY, high_polarity};
		}

		/**
		 * Create GPIOSettings for interrupt triggered when range is under 
		 * @p threshold.
		 * @param threshold the low threshold (in mm) under which an interrupt 
		 * shall be triggered
		 * @param high_polarity force GPIO interrupt polarity to HIGH; this is not
		 * advised as most breakouts include a pullup resistor.
		 */
		static constexpr GPIOSettings low_threshold(uint16_t threshold, bool high_polarity = false)
		{
			return GPIOSettings{GPIOFunction::LEVEL_LOW, high_polarity, threshold};
		}

		/**
		 * Create GPIOSettings for interrupt triggered when range is above
		 * @p threshold.
		 * @param threshold the high threshold (in mm) above which an interrupt 
		 * shall be triggered
		 * @param high_polarity force GPIO interrupt polarity to HIGH; this is not
		 * advised as most breakouts include a pullup resistor.
		 */
		static constexpr GPIOSettings high_threshold(uint16_t threshold, bool high_polarity = false)
		{
			return GPIOSettings{GPIOFunction::LEVEL_HIGH, high_polarity, 0, threshold};
		}

		/**
		 * Create GPIOSettings for interrupt triggered when range is outside a 
		 * window between @p low_threshold and @p high_threshold.
		 * @param low_threshold the low threshold (in mm) under which an interrupt 
		 * shall be triggered
		 * @param high_threshold the high threshold (in mm) above which an interrupt 
		 * shall be triggered
		 * @param high_polarity force GPIO interrupt polarity to HIGH; this is not
		 * advised as most breakouts include a pullup resistor.
		 */
		static constexpr GPIOSettings out_of_window(
			uint16_t low_threshold, uint16_t high_threshold, bool high_polarity = false)
		{
			return GPIOSettings{GPIOFunction::OUT_OF_WINDOW, high_polarity, low_threshold, high_threshold};
		}

		/**
		 * Return the current GPIO interrupt trigger source.
		 */
		GPIOFunction function() const
		{
			return function_;
		}
		/**
		 * Return the current polarity level of GPIO interrupts.
		 */
		bool high_polarity() const
		{
			return high_polarity_;
		}
		/**
		 * Return the current low threshold, in mm.
		 */
		uint16_t low_threshold() const
		{
			return low_threshold_;
		}
		/**
		 * Return the current high threshold, in mm.
		 */
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
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, const GPIOSettings& settings);
	/// @endcond

	/// @cond notdocumented
	class InterruptStatus
	{
	public:
		InterruptStatus() = default;
		//TODO maybe return more useful info (is each bit mapped to GPIOFunction?)
		explicit operator uint8_t() const
		{
			return status_ & 0x07;
		}

	private:
		uint8_t status_ = 0;
	};
	/// @endcond

	/**
	 * Hold reference SPADs (Single Photon Avalanche Diode).
	 * VL53L0X device has 48 SPADs, only a part being enabled.
	 * SPAD enable status is stored as a bit in an array of 6 bytes.
	 * 
	 * @sa VL53L0X::get_reference_SPADs()
	 * @sa VL53L0X::set_reference_SPADs()
	 * @sa SPADInfo
	 */
	class SPADReference
	{
	public:
		/// @cond notdocumented
		static constexpr uint8_t NUM_PADS_BYTES = 6;
		SPADReference() = default;
		explicit SPADReference(const uint8_t spad_refs[NUM_PADS_BYTES])
		{
			memcpy(spad_refs_, spad_refs, NUM_PADS_BYTES);
		}
		/// @endcond

		/**
		 * Get a pointer to the 6-byte array stored in this SPADReference instance.
		 * The returned array cannot be modified.
		 */
		const uint8_t* spad_refs() const
		{
			return spad_refs_;
		}

		/**
		 * Get a pointer to the 6-byte array stored in this SPADReference instance.
		 * The returned array can be modified.
		 */
		uint8_t* spad_refs()
		{
			return spad_refs_;
		}

	private:
		uint8_t spad_refs_[NUM_PADS_BYTES];
	};

	/**
	 * Type of pulse period configured for VL53L0X device VCSEL (Vertical Cavity
	 * Surface Emitting Laser).
	 * VCSEL pulse period can be configured for PRE-RANGE and FINAL-RANGE steps
	 * in ranging steps sequence.
	 * Changing pulse periods has an impact on range distance.
	 * Pulse period is expressed in PCLK, whatever that really means.
	 * @sa VL53L0X::get_vcsel_pulse_period()
	 * @sa VL53L0X::set_vcsel_pulse_period()
	 */
	enum class VcselPeriodType : uint8_t
	{
		PRE_RANGE = uint8_t(vl53l0x::Register::PRE_RANGE_CONFIG_VCSEL_PERIOD),
		FINAL_RANGE = uint8_t(vl53l0x::Register::FINAL_RANGE_CONFIG_VCSEL_PERIOD)
	};

	/**
	 * Hold VL53L0X sequence steps to use for ranging.
	 * This class is implemented using the "builder" pattern, demonstrated in 
	 * following snippet:
	 * @code
	 * // steps1 contains only pre-range and final-range steps
	 * SequenceSteps steps1 = SequenceSteps::create().pre_range().final_range();
	 * // steps2 contains all steps except TCC
	 * SequenceSteps steps2 = SequenceSteps::all().no_tcc();
	 * @endcode
	 * All builder methods are `constexpr` hence are typically evaluated at 
	 * compile-time.
	 * Each step has an impact on minimal measurement time for ranging and on 
	 * accuracy.
	 * @note Advised sequence to get good results shall include PRE-RANGE,
	 * FINAL-RANGE and DSS.
	 * 
	 * @sa VL53L0X::get_sequence_steps()
	 * @sa VL53L0X::set_sequence_steps()
	 */
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
		/**
		 * Create an empty step sequence for further adding individual steps.
		 */
		static constexpr SequenceSteps create()
		{
			return SequenceSteps{};
		}
		/**
		 * Create a full step sequence with all steps, with the possibility to 
		 * remove steps you do not want:
		 * - TCC
		 * - DSS
		 * - MSRC
		 * - PRE-RANGE
		 * - FINAL-RANGE
		 */
		static constexpr SequenceSteps all()
		{
			return SequenceSteps{TCC | DSS | MSRC | PRE_RANGE | FINAL_RANGE};
		}

		/// @cond notdocumented
		constexpr SequenceSteps() = default;
		/// @endcond

		/**
		 * Create a new SequenceSteps by adding TCC to steps of this instance.
		 */
		constexpr SequenceSteps tcc() const
		{
			return SequenceSteps{uint8_t(steps_ | TCC)};
		}

		/**
		 * Create a new SequenceSteps by adding DSS to steps of this instance.
		 */
		constexpr SequenceSteps dss() const
		{
			return SequenceSteps{uint8_t(steps_ | DSS)};
		}

		/**
		 * Create a new SequenceSteps by adding MSRC to steps of this instance.
		 */
		constexpr SequenceSteps msrc() const
		{
			return SequenceSteps{uint8_t(steps_ | MSRC)};
		}

		/**
		 * Create a new SequenceSteps by adding PRE-RANGE to steps of this instance.
		 */
		constexpr SequenceSteps pre_range() const
		{
			return SequenceSteps{uint8_t(steps_ | PRE_RANGE)};
		}

		/**
		 * Create a new SequenceSteps by adding FINAL-RANGE to steps of this instance.
		 */
		constexpr SequenceSteps final_range() const
		{
			return SequenceSteps{uint8_t(steps_ | FINAL_RANGE)};
		}

		/**
		 * Create a new SequenceSteps by removing TCC from steps of this instance.
		 */
		constexpr SequenceSteps no_tcc() const
		{
			return SequenceSteps{uint8_t(steps_ & bits::COMPL(TCC))};
		}

		/**
		 * Create a new SequenceSteps by removing DSS from steps of this instance.
		 */
		constexpr SequenceSteps no_dss() const
		{
			return SequenceSteps{uint8_t(steps_ & bits::COMPL(DSS))};
		}

		/**
		 * Create a new SequenceSteps by removing MSRC from steps of this instance.
		 */
		constexpr SequenceSteps no_msrc() const
		{
			return SequenceSteps{uint8_t(steps_ & bits::COMPL(MSRC))};
		}

		/**
		 * Create a new SequenceSteps by removing PRE-RANGE from steps of this instance.
		 */
		constexpr SequenceSteps no_pre_range() const
		{
			return SequenceSteps{uint8_t(steps_ & bits::COMPL(PRE_RANGE))};
		}

		/**
		 * Create a new SequenceSteps by removing FINAL-RANGE from steps of this instance.
		 */
		constexpr SequenceSteps no_final_range() const
		{
			return SequenceSteps{uint8_t(steps_ & bits::COMPL(FINAL_RANGE))};
		}

		/**
		 * Get steps of this instance as a byte.
		 */
		uint8_t value() const
		{
			return steps_;
		}

		/**
		 * Indicate if this instance has TCC (Target Center Check) step.
		 */
		bool is_tcc() const
		{
			return steps_ & TCC;
		}

		/**
		 * Indicate if this instance has DSS (Dynamic SPAD Selection) step.
		 */
		bool is_dss() const
		{
			return steps_ & DSS;
		}

		/**
		 * Indicate if this instance has MSRC (Minimum Signal Rate Check) step.
		 */
		bool is_msrc() const
		{
			return steps_ & MSRC;
		}

		/**
		 * Indicate if this instance has PRE-RANGE step.
		 */
		bool is_pre_range() const
		{
			return steps_ & PRE_RANGE;
		}

		/**
		 * Indicate if this instance has FINAL-RANGE step.
		 */
		bool is_final_range() const
		{
			return steps_ & FINAL_RANGE;
		}

	private:
		explicit constexpr SequenceSteps(uint8_t steps) : steps_{uint8_t(steps | FORCED_BITS)} {}

		uint8_t steps_ = FORCED_BITS;

		template<typename MANAGER> friend class VL53L0X;
	};
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, SequenceSteps steps);
	/// @endcond

	/**
	 * Hold VL53L0X sequence steps timeouts and other related settings used for 
	 * ranging.
	 * 
	 * @sa VL53L0X::get_sequence_steps_timeout()
	 */
	class SequenceStepsTimeout
	{
	public:
		/// @cond notdocumented
		SequenceStepsTimeout() = default;
		/// @endcond

		/**
		 * VCSEL PCLK value for pre-range step.
		 */
		uint8_t pre_range_vcsel_period_pclks() const
		{
			return pre_range_vcsel_period_pclks_;
		}

		/**
		 * VCSEL PCLK value for final-range step.
		 */
		uint8_t final_range_vcsel_period_pclks() const
		{
			return final_range_vcsel_period_pclks_;
		}

		/**
		 * MCLK for any of MSRC, DSS or TCC steps.
		 */
		uint16_t msrc_dss_tcc_mclks() const
		{
			return msrc_dss_tcc_mclks_ + 1;
		}

		/**
		 * MCLK for PRE-RANGE step.
		 */
		uint16_t pre_range_mclks() const
		{
			return TimeoutUtilities::decode_timeout(pre_range_mclks_);
		}

		/**
		 * MCLK for FINAL-RANGE step.
		 * @param is_pre_range indicate if calculation is performed on a sequence 
		 * including PRE-RANGE step; this impacts calculation.
		 * @sa SequenceSteps
		 */
		uint16_t final_range_mclks(bool is_pre_range) const
		{
			uint16_t temp_final_range_mclks = TimeoutUtilities::decode_timeout(final_range_mclks_);
			return (is_pre_range ? (temp_final_range_mclks - pre_range_mclks()) : temp_final_range_mclks);
		}

		// Following values are calculated from others

		/**
		 * Calculate the timing in us of any of MSRC, DSS or TCC steps.
		 */
		uint32_t msrc_dss_tcc_us() const
		{
			return TimeoutUtilities::calculate_timeout_us(msrc_dss_tcc_mclks(), pre_range_vcsel_period_pclks());
		}

		/**
		 * Calculate the timing in us of PRE-RANGE step.
		 */
		uint32_t pre_range_us() const
		{
			return TimeoutUtilities::calculate_timeout_us(pre_range_mclks(), pre_range_vcsel_period_pclks());
		}

		/**
		 * Calculate the timing in us of FINAL-RANGE step.
		 * @param is_pre_range indicate if calculation is performed on a sequence 
		 * including PRE-RANGE step; this impacts calculation.
		 * @sa SequenceSteps
		 */
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
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, const SequenceStepsTimeout& timeouts);
	/// @endcond

	/**
	 * Hold SPAD information from VL53L0X device.
	 * This information is used to calculate refrence SPADs.
	 * @warning you should never need to access instances of this class.
	 * @sa VL53L0X::get_SPAD_info()
	 * @sa VL53L0X::get_reference_SPADs()
	 * @sa VL53L0X::set_reference_SPADs()
	 */
	class SPADInfo
	{
	private:
		static constexpr uint8_t APERTURE = bits::BV8(7);
		static constexpr uint8_t COUNT = bits::CBV8(7);

	public:
		/// @cond notdocumented
		SPADInfo() = default;
		explicit SPADInfo(uint8_t info) : info_{info} {}
		/// @endcond

		/**
		 * Indicate which is the first SPAD to enable, if `false` this is SPAD `0`,
		 * else it is SPAD `12`.
		 */
		bool is_aperture() const
		{
			return info_ & APERTURE;
		}

		/**
		 * Indicate the number of SPADs to enable.
		 */
		uint8_t count() const
		{
			return info_ & COUNT;
		}

	private:
		uint8_t info_ = 0;
	};
	/// @cond notdocumented
	streams::ostream& operator<<(streams::ostream& out, SPADInfo spad);
	/// @endcond

	/**
	 * Possible profiles of ranging for VL53L0X top-level API `VL53L0X::begin()`.
	 * Each profile defines specific VL53L0X settings.
	 * @sa VL53L0X::begin()
	 */
	enum class Profile : uint8_t
	{
		/** Standard profile: 33ms ranging time, common accuracy, 1.2m range. */
		STANDARD = 0x00,
		/** Long range profile: 33ms ranging time, common accuracy, 2.0m range. */
		LONG_RANGE = 0x01,
		/** Accurate standard profile: 200ms ranging time, high accuracy, 1.2m range. */
		STANDARD_ACCURATE = 0x02,
		/** Accurate long range profile: 200ms ranging time, high accuracy, 2.0m range. */
		LONG_RANGE_ACCURATE = 0x03,
		/** Standard fast profile: 20ms ranging time, low accuracy, 1.2m range. */
		STANDARD_FAST = 0x04,
		/** Standard long range profile: 20ms ranging time, low accuracy, 2.0m range. */
		LONG_RANGE_FAST = 0x05,
	};
}

#endif /* VL53L0X_TYPES_H */
/// @endcond
