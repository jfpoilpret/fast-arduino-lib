//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * Note that most API here has been adapted and improved from official 
 * STMicroelectronics C-library API; this was necessary as the device datasheet
 * does not describe the internals (registers) of the chip, the only way to
 * understand how it works was thus to analyze the API source code.
 * Also, the pololu library for this chip (simpler rewrite of STM lib) was a
 * good inspiration to understanding how this chip works.
 * 
 * @sa https://www.st.com/content/st_com/en/products/embedded-software/proximity-sensors-software/stsw-img005.html
 * @sa https://github.com/pololu/vl53l0x-arduino
 */

#ifndef VL53L0X_H
#define VL53L0X_H

#include "../array.h"
#include "../flash.h"
#include "../i2c.h"
#include "../functors.h"
#include "../future.h"
#include "../realtime_timer.h"
#include "../time.h"
#include "../utilities.h"
#include "../i2c_handler.h"
#include "../i2c_device.h"
#include "../i2c_device_utilities.h"
#include "vl53l0x_internals.h"
#include "vl53l0x_registers.h"
#include "vl53l0x_types.h"

namespace devices
{
	/**
	 * Defines API for VL53L0X Time-of-Flight ranging sensor chip usage.
	 * @note the API is partial, some functions of the original STM library have 
	 * not been ported at all, partly because their documentation did not allow
	 * understanding what their purpose could be. This concerns calculations
	 * performed by the API itself (not the device), or special calibration
	 * procedures. Additional API may be added in the future if needed.
	 */
	namespace vl53l0x
	{
	}
}

namespace devices::vl53l0x
{
	/// @cond notdocumented
	namespace internals = vl53l0x_internals;
	/// @endcond

	/**
	 * I2C device driver for the VL53L0X ToF ranging chip.
	 * This chip supports both standard and fast I2C modes.
	 * 
	 * Several levels of API are provided:
	 * - low-level API : reserved to developers who know how the VL53L0X device works
	 * and know what they do
	 * - mid-level API : for common use where developers need fine level of detail
	 * on how the VL53L0X device shall work
	 * - high-level API : for simplest use of the VL53L0X device
	 * 
	 * Most API comes in synchronous mode only, although it can of course work with
	 * an asynchronous I2C Manager. This is due to the highly complex protocol of
	 * VL53L0X device where I2C transactions can be long and would have required
	 * highly complex Future classes to be defined with little added value.
	 * Only API that were deemed useful in non-blocking mode were also made
	 * asynchronous.
	 * Synchronous API are blocking until completion and should never be called
	 * from an ISR!
	 * Asynchronous API can be called from anywhere, but you must await their
	 * completion through a Future.
	 * 
	 * @note The VL53L0X device is extremely complex and not well documented; its 
	 * only complete reference is the bloated C code provided by STM which was not
	 * possible to directly use in FastArduino library. Hence it took a lot of
	 * experiments to make this device work. Not all original STM API is provided
	 * here.
	 * 
	 * @tparam MANAGER one of FastArduino available I2C Manager
	 */
	template<typename MANAGER>
	class VL53L0X : public i2c::I2CDevice<MANAGER>
	{
	private:
		using PARENT = i2c::I2CDevice<MANAGER>;
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;

		template<Register REGISTER, typename T = uint8_t>
		using TReadRegisterFuture = 
			i2c::TReadRegisterFuture<MANAGER, uint8_t(REGISTER), T, functor::ChangeEndianness<T>>;
		template<Register REGISTER, typename T = uint8_t>
		using TWriteRegisterFuture = 
			i2c::TWriteRegisterFuture<MANAGER, uint8_t(REGISTER), T, functor::ChangeEndianness<T>>;

		using I2CFuturesGroup = i2c::I2CFuturesGroup<MANAGER>;
		using I2CSameFutureGroup = i2c::I2CSameFutureGroup<MANAGER>;

	public:
		/**
		 * Create a new device driver for a VL53L0X chip.
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 */
		explicit VL53L0X(MANAGER& manager) : PARENT{manager, DEFAULT_DEVICE_ADDRESS, i2c::I2C_FAST, false} {}

		/**
		 * Change the I2C address of this VL53L0X device.
		 * For this to work, other I2C devices with the same address shall be
		 * shut down during this method call.
		 * @warning Blocking API!
		 * @note High-level API
		 * 
		 * @param device_address the new I2C address for this device; only 7 LSB
		 * are relevant.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 */
		bool set_address(uint8_t device_address)
		{
			static constexpr uint8_t ADDRESS_MASK = 0x7F;
			using SetAddressFuture = TWriteRegisterFuture<Register::I2C_SLAVE_DEVICE_ADDRESS>;
			device_address &= ADDRESS_MASK;
			if (!this->template sync_write<SetAddressFuture>(device_address)) return false;
			this->set_device(uint8_t(device_address << 1));
			return true;
		}

		/**
		 * Fully initialize this VL53L0X device and configures it with provided @p profile.
		 * Once this method has been called successfully, you can start perform ranging
		 * (continuous or not).
		 * You may also want to set some GPIO interrupts before ranging.
		 * @warning Blocking API!
		 * @note High-level API
		 * If you need more custom settings to initalize the device you should turn to
		 * mid-level API.
		 * 
		 * @param profile the pre-defined Profile to use on this sensor
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa start_continuous_ranging()
		 * @sa await_single_range()
		 * 
		 * @sa set_GPIO_settings()
		 * 
		 * @sa init_data_first()
		 * @sa init_static_second()
		 */
		bool begin(Profile profile)
		{
			static constexpr uint8_t LONG_RANGE_MASK = 0x01;
			static constexpr uint8_t ACCURATE_MASK = 0x02;
			static constexpr uint8_t FAST_MASK = 0x04;
			static constexpr uint32_t ACCURATE_TIMING_BUDGET = 200'000UL;
			static constexpr uint32_t FAST_TIMING_BUDGET = 20'000UL;
			static constexpr uint8_t LONG_RANGE_VCSEL_PERIOD_PRE = 18;
			static constexpr uint8_t LONG_RANGE_VCSEL_PERIOD_FINAL = 14;
			
			if (!init_data_first()) return false;
			uint8_t prof = uint8_t(profile);
			if (!init_static_second(GPIOSettings::sample_ready(), 
				SequenceSteps::create().pre_range().final_range().dss()))
				return false;
			if (!perform_ref_calibration()) return false;
			if (prof & LONG_RANGE_MASK)
			{
				// long range
				if (!set_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(LONG_RANGE_VCSEL_PERIOD_PRE)) return false;
				if (!set_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(LONG_RANGE_VCSEL_PERIOD_FINAL)) return false;
				if (!set_signal_rate_limit(0.1)) return false;
			}
			if (prof & ACCURATE_MASK)
			{
				// accurate
				if (!set_measurement_timing_budget(ACCURATE_TIMING_BUDGET)) return false;
			}
			else if (prof & FAST_MASK)
			{
				// fast
				if (!set_measurement_timing_budget(FAST_TIMING_BUDGET)) return false;
			}
			return true;
		}

		/**
		 * Perform a single range action on VL53L0X device, and wait for the 
		 * measurement result.
		 * This shall be used when no continuous ranging is in effect.
		 * @warning Blocking API!
		 * @note High-level API
		 * @note Mid-level API
		 * 
		 * @tparam TIMER the Timer used for @p rtt; this template argument
		 * will be automatically deduced from @p rtt.
		 * @param rtt the real-time timer to use to count elapsed time
		 * @param range_mm a reference to a variable that will receive the 
		 * measurement result (in mm)
		 * @param timeout_ms the maximum amount of time to wait for a result;
		 * default is 100ms, but it should be higher than the measurement timing 
		 * budget.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed or timed out
		 * 
		 * @sa set_measurement_timing_budget()
		 * @sa await_single_range(uint16_t&, uint16_t)
		 */
		template<board::Timer TIMER>
		bool await_single_range(timer::RTT<TIMER>& rtt, uint16_t& range_mm, uint16_t timeout_ms = DEFAULT_TIMEOUT_MS)
		{
			time::RTTTime end = rtt.time() + time::RTTTime{timeout_ms, 0};
			using READ_SYSRANGE = TReadRegisterFuture<Register::SYSRANGE_START>;
			using WRITE_SYSRANGE = TWriteRegisterFuture<Register::SYSRANGE_START>;
			if (!use_stop_variable()) return false;
			if (!this->template sync_write<WRITE_SYSRANGE>(uint8_t(0x01))) return false;
			// Read SYSRANGE until != 0x01
			while (rtt.time() < end)
			{
				uint8_t sys_range = 0;
				if (!this->template sync_read<READ_SYSRANGE>(sys_range)) return false;
				if ((sys_range & 0x01) == 0)
				{
					// Replace timeout_ms with remaining time only
					time::RTTTime now = rtt.time();
					timeout_ms = (end > now) ? (end - now).millis() : 0;
					if (timeout_ms == 0) timeout_ms = 1U;
					return await_continuous_range(rtt, range_mm, timeout_ms);
				}
			}
			return false;
		}

		/**
		 * Start continuous ranging on this VL53L0X device.
		 * This method shall not be called before the device has been properly 
		 * initialized, either with high-level API `begin()` or mid-level API
		 * methods `init_data_first()`, `init_static_second()` and 
		 * `perform_ref_calibration()`.
		 * Once this method has been called, continuous ranging starts on the device,
		 * at the given period.
		 * You can check when a sample is ready to read by examining interrupt 
		 * status or reange status, then you can read the range and clear the 
		 * interrupt.
		 * Or you may prefer just await for new range which is easier to code
		 * but will block your program.
		 * @warning Blocking API!
		 * @note High-level API
		 * @note Mid-level API
		 * 
		 * @param period_ms the period, in ms, between 2 consecutive ranging 
		 * measures; if `0` (the default), then consecutive measures will follow
		 * each other with no delay ("back-to-back" mode).
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa stop_continuous_range()
		 * @sa await_continuous_range()
		 * @sa get_interrupt_status()
		 * @sa get_range_status()
		 * @sa get_direct_range()
		 */
		bool start_continuous_ranging(uint16_t period_ms = 0)
		{
			using READ_OSC_CAL = TReadRegisterFuture<Register::OSC_CALIBRATE_VAL, uint16_t>;
			using WRITE_PERIOD = TWriteRegisterFuture<Register::SYSTEM_INTERMEASUREMENT_PERIOD, uint32_t>;
			using WRITE_SYSRANGE = TWriteRegisterFuture<Register::SYSRANGE_START>;
			if (!use_stop_variable()) return false;
			uint8_t sys_range_start = 0x02;
			if (period_ms)
			{
				uint16_t osc_calibrate = 0;
				if (!this->template sync_read<READ_OSC_CAL>(osc_calibrate)) return false;
				uint32_t actual_period = period_ms;
				if (osc_calibrate) actual_period *= osc_calibrate;
				if (!this->template sync_write<WRITE_PERIOD>(actual_period)) return false;
				sys_range_start = 0x04;
			}
			return this->template sync_write<WRITE_SYSRANGE>(sys_range_start);
		}

		/**
		 * Wait for the next continuous ranging measure on VL53L0X device to be
		 * ready and return the result.
		 * This shall be used only when continuous ranging is in effect.
		 * @warning Blocking API!
		 * @note High-level API
		 * @note Mid-level API
		 * 
		 * @tparam TIMER the Timer used for @p rtt; this template argument
		 * will be automatically deduced from @p rtt.
		 * @param rtt the real-time timer to use to count elapsed time
		 * @param range_mm a reference to a variable that will receive the 
		 * measurement result (in mm)
		 * @param timeout_ms the maximum amount of time to wait for a result;
		 * default is 100ms, but it should be higher than the measurement timing 
		 * budget.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed or timed out
		 * 
		 * @sa start_continuous_ranging()
		 * @sa set_measurement_timing_budget()
		 * @sa await_continuous_range(uint16_t&, uint16_t)
		 */
		template<board::Timer TIMER> bool await_continuous_range(
			timer::RTT<TIMER>& rtt, uint16_t& range_mm, uint16_t timeout_ms = DEFAULT_TIMEOUT_MS)
		{
			if (!await_interrupt(rtt, timeout_ms)) return false;
			if (!get_direct_range(range_mm)) return false;
			return clear_interrupt();
		}

		/**
		 * Stop continuous ranging on this VL53L0X device.
		 * @warning Blocking API!
		 * @note High-level API
		 * @note Mid-level API
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa start_continuous_range()
		 */
		bool stop_continuous_ranging()
		{
			return await_same_future_group(
				internals::stop_continuous_ranging::BUFFER, internals::stop_continuous_ranging::BUFFER_SIZE);
		}

		/**
		 * Reset VL53L0X device. After this method is called, the device is in
		 * the same state as after power-up, hence device must be reinitialized 
		 * from scratch.
		 * @warning Blocking API!
		 * @note High-level API
		 * @note Mid-level API
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa begin()
		 * @sa init_data_first()
		 * @sa init_static_second()
		 * @sa perform_ref_calibration()
		 */
		bool reset_device()
		{
			static constexpr uint16_t RESET_DELAY_US = 100U;
			using WRITE_RESET = TWriteRegisterFuture<Register::SOFT_RESET_GO2_SOFT_RESET_N>;
			// Set reset bit
			if (!this->template sync_write<WRITE_RESET>(uint8_t(0x00))) return false;
			// Wait for some time
			uint8_t model = 0xFF;
			do
			{
				get_model(model);
			}
			while (model != 0);
			time::delay_us(RESET_DELAY_US);

			// Release reset
			if (!this->template sync_write<WRITE_RESET>(uint8_t(0x01))) return false;
			// Wait until correct boot-up
			model = 0x00;
			do
			{
				get_model(model);
			}
			while (model == 0);
			time::delay_us(RESET_DELAY_US);
			return true;
		}

		/**
		 * Perform 1st stage initialization of this VL53L0X device.
		 * VL53L0X initialization is made of 3 steps before it can perform any 
		 * ranging:
		 * - data init: must be performed once only after device power-up
		 * - static init: must be performed at least once, after data init, but can
		 * be called several times to changes ranging steps sequence
		 * - reference calibration: must be performed once
		 * One method exists for each stage.
		 * 
		 * After initalization, you may call methods changing all kinds of settings.
		 * Finally, you may then perform ranging, continuously or not.
		 * 
		 * @warning Blocking API!
		 * @note Mid-level API
		 * If you want simpler initialization of the device, you should turn to high-level API.
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa init_static_second()
		 * @sa perform_ref_calibration()
		 * @sa begin()
		 */
		bool init_data_first()
		{
			// 1. Force 2.8V for I/O (instead of default 1.8V)
			if (!force_io_2_8V()) return false;
			// 2. Set I2C standard mode
			if (!set_I2C_mode()) return false;
			// 3. Read stop variable here
			if (!read_stop_variable()) return false;
			// 4. Disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE limit checks
			if (!disable_signal_rate_limit_check()) return false;
			// 5. Set signal rate limit to 0.25 MCPS (million counts per second) in FP9.7 format
			if (!set_signal_rate_limit(0.25)) return false;
			// 6. Enable all sequence steps by default
			return set_sequence_steps((SequenceSteps) 0xFF);
		}

		/**
		 * Perform 2nd stage initialization of this VL53L0X device.
		 * This must be called once, or more, after `init_data_first()` has been 
		 * called.
		 * It allows you to select which ranging steps shall be used afterwards.
		 * 
		 * @warning Blocking API!
		 * @note Mid-level API
		 * If you want simpler initialization of the device, you should turn to high-level API.
		 * 
		 * @param settings intial GPIO settings to set on the device; these settings
		 * can be changed anytime later.
		 * @param steps the ranging steps sequence
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_GPIO_settings()
		 * @sa set_sequence_steps()
		 * @sa init_data_first()
		 * @sa perform_ref_calibration()
		 * @sa begin()
		 */
		bool init_static_second(const GPIOSettings& settings, 
			SequenceSteps steps = SequenceSteps::create().pre_range().final_range().dss())
		{
			// 1. Get SPAD info
			SPADInfo info;
			if (!get_SPAD_info(info)) return false;
			// 2. Get reference SPADs from NVM
			SPADReference ref_spads;
			if (!get_reference_SPADs(ref_spads)) return false;
			// 3. Calculate SPADs and set reference SPADs
			calculate_reference_SPADs(ref_spads.spad_refs(), info);
			if (!set_reference_SPADs(ref_spads)) return false;
			// 4. Load tuning settings
			if (!load_tuning_settings()) return false;
			// 5. Set GPIO settings
			if (!set_GPIO_settings(settings)) return false;
			// 6. Get current timing budget
			uint32_t budget_us = 0UL;
			if (!get_measurement_timing_budget(budget_us)) return false;
			// 7. Set sequence steps by default?
			if (!set_sequence_steps(steps)) return false;
			// 8. Recalculate timing budget and set it
			return set_measurement_timing_budget(budget_us);
		}

		/**
		 * Perform VL53L0X VHV and Phase calibration.
		 * This must be called once before any measurement, but after data and 
		 * static initalization.
		 * 
		 * @warning Blocking API!
		 * @note Mid-level API
		 * If you want simpler initialization of the device, you should turn to high-level API.
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa init_data_first()
		 * @sa init_static_second()
		 * @sa begin()
		 */
		bool perform_ref_calibration()
		{
			static constexpr uint8_t CODE_VHV_CALIBRATION = 0x01;
			static constexpr uint8_t CODE_PHASE_CALIBRATION = 0x02;
			using WRITE_STEPS = TWriteRegisterFuture<Register::SYSTEM_SEQUENCE_CONFIG>;
			// 1. Read current sequence steps
			SequenceSteps steps;
			if (!get_sequence_steps(steps)) return false;
			// 2. Set steps for VHV calibration
			if (!this->template sync_write<WRITE_STEPS>(CODE_VHV_CALIBRATION)) return false;
			// 3. Perform single VHV calibration
			if (!perform_single_ref_calibration(SingleRefCalibrationTarget::VHV_CALIBRATION)) return false;
			// 4. Set steps for Phase calibration
			if (!this->template sync_write<WRITE_STEPS>(CODE_PHASE_CALIBRATION)) return false;
			// 5. Perform single Phase calibration
			if (!perform_single_ref_calibration(SingleRefCalibrationTarget::PHASE_CALIBRATION)) return false;
			// 6. Restore sequence steps (NOTE: 0x00 is used as marker by the future to actually restore saved sequence)
			return set_sequence_steps(steps);
		}

		/**
		 * Future to get device range status.
		 * @sa get_range_status()
		 */
		using GetRangeStatusFuture = TReadRegisterFuture<Register::RESULT_RANGE_STATUS, DeviceStatus>;

		/**
		 * Get current `DeviceStatus` from this device.
		 * @warning Asynchronous API!
		 * @note Mid-level API
		 * 
		 * @param future a GetRangeStatusFuture passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * 
		 * @sa get_range_status(DeviceStatus&)
		 */
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return this->async_read(future);
		}

		/**
		 * Get current `DeviceStatus` from this device.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param range_status a reference to a variable that will receive the 
		 * device status
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_range_status()
		 */
		bool get_range_status(DeviceStatus& range_status)
		{
			return this->template sync_read<GetRangeStatusFuture>(range_status);
		}

		/**
		 * Future to get device current GPIO settings.
		 * @sa get_GPIO_settings()
		 */
		class GetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			/// @cond notdocumented
			GetGPIOSettingsFuture() : PARENT{futures_, NUM_FUTURES}
			{
				PARENT::init(futures_);
			}
			GetGPIOSettingsFuture(GetGPIOSettingsFuture&&) = default;
			GetGPIOSettingsFuture& operator=(GetGPIOSettingsFuture&&) = default;

			bool get(vl53l0x::GPIOSettings& settings)
			{
				static constexpr uint8_t GPIO_ACTIVE_LEVEL_MASK = 0x10;
				if (this->await() != future::FutureStatus::READY)
					return false;
				vl53l0x::GPIOFunction function = vl53l0x::GPIOFunction::DISABLED;
				read_config_.get(function);
				uint8_t active_high = 0;
				read_GPIO_active_high_.get(active_high);
				uint16_t low_threshold = 0;
				read_low_threshold_.get(low_threshold);
				uint16_t high_threshold = 0;
				read_high_threshold_.get(high_threshold);
				settings = vl53l0x::GPIOSettings{
					function, bool(active_high & GPIO_ACTIVE_LEVEL_MASK), low_threshold, high_threshold};
				return true;
			}
			/// @endcond

		private:
			TReadRegisterFuture<Register::SYSTEM_INTERRUPT_CONFIG_GPIO, vl53l0x::GPIOFunction> read_config_{};
			TReadRegisterFuture<Register::GPIO_HV_MUX_ACTIVE_HIGH> read_GPIO_active_high_{};
			TReadRegisterFuture<Register::SYSTEM_THRESH_LOW, uint16_t> read_low_threshold_{};
			TReadRegisterFuture<Register::SYSTEM_THRESH_HIGH, uint16_t> read_high_threshold_{};

			static constexpr uint8_t NUM_FUTURES = 4;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&read_config_,
				&read_GPIO_active_high_,
				&read_low_threshold_,
				&read_high_threshold_
			};

			friend VL53L0X<MANAGER>;
		};

		/**
		 * Get current `GPIOSettings` from this device.
		 * @warning Asynchronous API!
		 * @note Mid-level API
		 * 
		 * @param future a GetGPIOSettingsFuture passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * 
		 * @sa set_GPIO_settings()
		 * @sa get_GPIO_settings(GPIOSettings&)
		 */
		int get_GPIO_settings(GetGPIOSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		/**
		 * Get current `GPIOSettings` from this device.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param settings a reference to a variable that will receive the
		 * current GPIO settings for this device
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_GPIO_settings(const GPIOSettings&)
		 * @sa get_GPIO_settings()
		 */
		bool get_GPIO_settings(GPIOSettings& settings)
		{
			GetGPIOSettingsFuture future{};
			if (get_GPIO_settings(future) != 0) return false;
			return future.get(settings);
		}

		/**
		 * Future to set device GPIO settings.
		 * @sa set_GPIO_settings()
		 */
		class SetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			/// @cond notdocumented
			explicit SetGPIOSettingsFuture(const vl53l0x::GPIOSettings& settings)
				:	PARENT{futures_, NUM_FUTURES},
					write_config_{settings.function()},
					// The following hard-coded values look OK but this is not how it should be done!
					//TODO GPIO_HV_MUX_ACTIVE_HIGH should first be read and then bit 4 clear or set
					write_GPIO_active_high_{uint8_t(settings.high_polarity() ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW)},
					// Threshold values must be divided by 2, but nobody knows why
					write_low_threshold_{settings.low_threshold() / 2},
					write_high_threshold_{settings.high_threshold() / 2}
			{
				PARENT::init(futures_);
			}
			SetGPIOSettingsFuture(SetGPIOSettingsFuture&&) = default;
			SetGPIOSettingsFuture& operator=(SetGPIOSettingsFuture&&) = default;
			/// @endcond

		private:
			static constexpr uint8_t GPIO_LEVEL_HIGH = 0x11;
			static constexpr uint8_t GPIO_LEVEL_LOW = 0x01;
			TWriteRegisterFuture<Register::SYSTEM_INTERRUPT_CONFIG_GPIO, vl53l0x::GPIOFunction> write_config_;
			TWriteRegisterFuture<Register::GPIO_HV_MUX_ACTIVE_HIGH> write_GPIO_active_high_;
			TWriteRegisterFuture<Register::SYSTEM_THRESH_LOW, uint16_t> write_low_threshold_;
			TWriteRegisterFuture<Register::SYSTEM_THRESH_HIGH, uint16_t> write_high_threshold_;
			TWriteRegisterFuture<Register::SYSTEM_INTERRUPT_CLEAR> clear_interrupt_{0};

			static constexpr uint8_t NUM_FUTURES = 5;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&write_config_,
				&write_GPIO_active_high_,
				&write_low_threshold_,
				&write_high_threshold_,
				&clear_interrupt_
			};

			friend VL53L0X<MANAGER>;
		};

		/**
		 * Set new `GPIOSettings` for this device.
		 * @warning Asynchronous API!
		 * @note Mid-level API
		 * 
		 * @param future a SetGPIOSettingsFuture passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * 
		 * @sa get_GPIO_settings()
		 * @sa set_GPIO_settings(const GPIOSettings&)
		 */
		int set_GPIO_settings(SetGPIOSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		/**
		 * Set new `GPIOSettings` for this device.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param settings new GPIO settings to write to this device
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_GPIO_settings()
		 * @sa get_GPIO_settings(GPIOSettings&)
		 */
		bool set_GPIO_settings(const GPIOSettings& settings)
		{
			SetGPIOSettingsFuture future{settings};
			if (set_GPIO_settings(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Future to get device current interrupt status.
		 * @sa get_interrupt_status()
		 */
		using GetInterruptStatusFuture = TReadRegisterFuture<Register::RESULT_INTERRUPT_STATUS, InterruptStatus>;

		/**
		 * Get current `InterruptStatus` from this device.
		 * @warning Asynchronous API!
		 * @note Mid-level API
		 * 
		 * @param future a GetInterruptStatusFuture passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * 
		 * @sa clear_interrupt()
		 * @sa get_interrupt_status(InterruptStatus&)
		 */
		int get_interrupt_status(PROXY<GetInterruptStatusFuture> future)
		{
			return this->async_read(future);
		}

		/**
		 * Get current `InterruptStatus` from this device.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param status a reference to a variable that will receive the current
		 * interrupt status of this device
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa clear_interrupt(uint8_t)
		 * @sa get_interrupt_status()
		 */
		bool get_interrupt_status(InterruptStatus& status)
		{
			return this->template sync_read<GetInterruptStatusFuture>(status);
		}

		/**
		 * Future to clear device interrupt status.
		 * @sa clear_interrupt()
		 */
		using ClearInterruptFuture = TWriteRegisterFuture<Register::SYSTEM_INTERRUPT_CLEAR>;

		/**
		 * Clear interrupt status from this device.
		 * @warning Asynchronous API!
		 * @note Mid-level API
		 * 
		 * @param future a ClearInterruptFuture passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * 
		 * @sa clear_interrupt(uint8_t)
		 * @sa get_interrupt_status()
		 */
		int clear_interrupt(PROXY<ClearInterruptFuture> future)
		{
			return this->async_write(future);
		}

		/**
		 * Clear interrupt status from this device.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param clear_mask the mask of the interrupts to be cleared
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_interrupt_status()
		 * @sa clear_interrupt()
		 */
		bool clear_interrupt(uint8_t clear_mask = 0x01)
		{
			return this->template sync_write<ClearInterruptFuture>(clear_mask);
		}

		/**
		 * Wait for an interrupt condition on VL53L0X device.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @tparam TIMER the Timer used for @p rtt; this template argument
		 * will be automatically deduced from @p rtt.
		 * @param rtt the real-time timer to use to count elapsed time
		 * @param timeout_ms the maximum amount of time to wait for a result;
		 * default is 100ms, but it should be higher than the measurement timing 
		 * budget.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed or timed out
		 * 
		 * @sa await_continuous_range()
		 */
		template<board::Timer TIMER>
		bool await_interrupt(timer::RTT<TIMER>& rtt, uint16_t timeout_ms = DEFAULT_TIMEOUT_MS)
		{
			time::RTTTime end = rtt.time() + time::RTTTime{timeout_ms, 0};
			while (rtt.time() < end)
			{
				InterruptStatus status;
				if (!this->template sync_read<GetInterruptStatusFuture>(status)) return false;
				if (uint8_t(status) != 0)
					return true;
			}
			return false;
		}

		/**
		 * Future to get device current range measure.
		 * @sa get_direct_range()
		 */
		using GetDirectRangeFuture = TReadRegisterFuture<Register::RESULT_RANGE_MILLIMETER, uint16_t>;

		/**
		 * Get range measured by this device.
		 * This method does not wait for anything, it just gets the current value
		 * in the range register. This is useful only when you know a range is ready
		 * to read.
		 * In general, this method shall be used only after device interrupt
		 * status != 0, then interrupt status should be cleared immediately after.
		 * You would probably prefer to use methods that first await for range
		 * measure to be ready before returning its value.
		 * 
		 * @warning Asynchronous API!
		 * @note Mid-level API
		 * 
		 * @param future a GetDirectRangeFuture passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * 
		 * @sa await_single_range()
		 * @sa await_continuous_range()
		 * @sa get_direct_range(uint16_t&)
		 */
		int get_direct_range(PROXY<GetDirectRangeFuture> future)
		{
			return this->async_read(future);
		}

		/**
		 * Get range measured by this device.
		 * This method does not wait for anything, it just gets the current value
		 * in the range register. This is useful only when you know a range is ready
		 * to read.
		 * You would probably prefer to use methods that first await for range
		 * measure to be ready before returning its value.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param range_mm a reference to a variable that will receive the current 
		 * range (in mm) stored in the device
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa await_single_range()
		 * @sa await_continuous_range()
		 * @sa get_direct_range()
		 */
		bool get_direct_range(uint16_t& range_mm)
		{
			return this->template sync_read<GetDirectRangeFuture>(range_mm);
		}

		/**
		 * Set new "measurement timing budget" for this device.
		 * This is the amount of time (in us) that is used to perform a range.
		 * 
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param budget_us the new measurement timing budget to use for ranging;
		 * it must be bigger than 20000us; the actual minimum value also depends
		 * on other device settings, in particular the `SequenceSteps` used for 
		 * ranging. The bigger the budget, the higher the accuracy of measurements.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_measurement_timing_budget()
		 */
		bool set_measurement_timing_budget(uint32_t budget_us)
		{
			using WRITE_BUDGET = TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			SequenceSteps steps;
			if (!get_sequence_steps(steps)) return false;
			SequenceStepsTimeout timeouts;
			if (!get_sequence_steps_timeout(timeouts)) return false;
			// Calculate budget
			uint16_t budget = calculate_final_range_timeout(steps, timeouts, budget_us);
			if (budget == 0) return false;
			return this->template sync_write<WRITE_BUDGET>(budget);
		}

		/**
		 * Get current "measurement timing budget" for this device.
		 * This is the amount of time (in us) that is used to perform a range.
		 * This amount is calculated based on other settings of the device.
		 * 
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param budget_us a reference to a variable that will receive the
		 * current measurement timing budget for this device
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_measurement_timing_budget()
		 */
		bool get_measurement_timing_budget(uint32_t& budget_us)
		{
			// Get steps and timeouts
			SequenceSteps steps{};
			if (!get_sequence_steps(steps)) return false;
			SequenceStepsTimeout timeouts{};
			if (!get_sequence_steps_timeout(timeouts)) return false;
			// Calculate timing budget
			budget_us = calculate_measurement_budget_us(true, steps, timeouts);
			return true;
		}

		/**
		 * Set measurement steps to be executed in sequence by the device 
		 * during ranging.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param sequence_steps the sequence steps to use for ranging
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_sequence_steps()
		 * @sa SequenceSteps
		 */
		bool set_sequence_steps(SequenceSteps sequence_steps)
		{
			using SetSequenceStepsFuture = TWriteRegisterFuture<Register::SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
			return this->template sync_write<SetSequenceStepsFuture>(sequence_steps);
		}

		/**
		 * Get current measurement steps executed in sequence by the device 
		 * during ranging.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param sequence_steps a reference to a variable that will receive the 
		 * sequence steps
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_sequence_steps()
		 * @sa SequenceSteps
		 */
		bool get_sequence_steps(SequenceSteps& sequence_steps)
		{
			using GetSequenceStepsFuture = TReadRegisterFuture<Register::SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
			return this->template sync_read<GetSequenceStepsFuture>(sequence_steps);
		}

		/**
		 * Set new pulse period of the VCSEL for pre-range or final-range step.
		 * Pulse period is expressed in PCLK, whatever that really means.
		 * Changing pulse periods has an impact on range distance.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @tparam TYPE the type of pulse period we want to set (pre-range 
		 * or final-range)
		 * @param period the new pulse period to use for @p TYPE step; for pre-range,
		 * possible values are 12, 14, 16, 18; for final-range these are 8, 10, 
		 * 12, 14.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_vcsel_pulse_period()
		 * @sa VcselPeriodType
		 */
		template<VcselPeriodType TYPE>
		bool set_vcsel_pulse_period(uint8_t period)
		{
			return set_vcsel_pulse_period(TYPE, period);
		}

		/**
		 * Get current pulse period of the VCSEL for pre-range or final-range step.
		 * Pulse period is expressed in PCLK, whatever that really means.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @tparam TYPE the type of pulse period we want to read
		 * @param period a reference to a variable that will receive the 
		 * pulse period
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_vcsel_pulse_period()
		 * @sa VcselPeriodType
		 */
		template<VcselPeriodType TYPE>
		bool get_vcsel_pulse_period(uint8_t& period)
		{
			using GetVcselPulsePeriodFuture = TReadRegisterFuture<Register((uint8_t) TYPE)>;
			if (!this->template sync_read<GetVcselPulsePeriodFuture>(period)) return false;
			period = decode_vcsel_period(period);
			return true;
		}

		/**
		 * Set new signal rate limit for ranging.
		 * This is a ratio between 0.0 and 1.0; lower values will allow
		 * ranging in long distance or in a noisy environment; default is normally
		 * 0.25.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param signal_rate the new signal rate limit to use for ranging;
		 * must be between 0.0 and 1.0.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_signal_rate_limit()
		 */
		bool set_signal_rate_limit(float signal_rate)
		{
			if ((signal_rate <= 0.0) || (signal_rate > 1.0)) return false;
			using SetSignalRateLimitFuture = 
				TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;
			return this->template sync_write<SetSignalRateLimitFuture>(FixPoint9_7::convert(signal_rate));
		}

		/**
		 * Get current signal rate limit for ranging.
		 * This is a ratio between 0.0 and 1.0; lower values will allow
		 * ranging in long distance or in a noisy environment; default is normally
		 * 0.25.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param signal_rate a reference to a variable that will receive the 
		 * current signal rate limit
		 * 
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_signal_rate_limit()
		 */
		bool get_signal_rate_limit(float& signal_rate)
		{
			using GetSignalRateLimitFuture = 
				TReadRegisterFuture<Register::FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;
			uint16_t temp = 0;
			if (!this->template sync_read<GetSignalRateLimitFuture>(temp)) return false;
			signal_rate = FixPoint9_7::convert(temp);
			return true;
		}

		/**
		 * Get current power mode of this VL53L0X register.
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param power_mode a reference to a variable that will receive the revision
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 */
		bool get_power_mode(PowerMode& power_mode)
		{
			using GetPowerModeFuture = TReadRegisterFuture<Register::POWER_MANAGEMENT, PowerMode>;
			return this->template sync_read<GetPowerModeFuture>(power_mode);
		}

		/**
		 * Get model of this VL53L0X register. This is normally a constant (`0xEE`).
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param model a reference to a variable that will receive the model
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_revision()
		 */
		bool get_model(uint8_t& model)
		{
			using GetModelFuture = TReadRegisterFuture<Register::IDENTIFICATION_MODEL_ID>;
			return this->template sync_read<GetModelFuture>(model);
		}

		/**
		 * Get revision of this VL53L0X register. This is normally a constant (`0x10`).
		 * @warning Blocking API!
		 * @note Mid-level API
		 * 
		 * @param revision a reference to a variable that will receive the revision
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_model()
		 */
		bool get_revision(uint8_t& revision)
		{
			using GetRevisionFuture = TReadRegisterFuture<Register::IDENTIFICATION_REVISION_ID>;
			return this->template sync_read<GetRevisionFuture>(revision);
		}

		/**
		 * Wait for an interrupt condition on VL53L0X device.
		 * Wait is performed based on a maximum number of loops, not on actual
		 * time. 
		 * This is more difficult to use than `await_interrupt(timer::RTT<TIMER>&, uint16_t)`
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @param loops the maximum number of waiting loops before timeout;
		 * default is 2000, but it may not be suitable in every situation.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed or timed out
		 * 
		 * @sa await_interrupt(timer::RTT<TIMER>&, uint16_t)
		 */
		bool await_interrupt(uint16_t loops = MAX_LOOP)
		{
			// Read interrupt until !=0
			while (loops--)
			{
				InterruptStatus status;
				if (!this->template sync_read<GetInterruptStatusFuture>(status)) return false;
				if (uint8_t(status) != 0)
					return true;
			}
			return false;
		}

		/**
		 * Wait for the next continuous ranging measure on VL53L0X device to be
		 * ready and return the result.
		 * This shall be used only when continuous ranging is in effect.
		 * Wait is performed based on a maximum number of loops, not on actual
		 * time. 
		 * This is more difficult to use than `await_continuous_range(timer::RTT<TIMER>&, uint16_t&, uint16_t)`
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @param range_mm a reference to a variable that will receive the 
		 * measurement result (in mm)
		 * @param loops the maximum number of waiting loops before timeout;
		 * default is 2000, but it may not be suitable in every situation.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed or timed out
		 * 
		 * @sa start_continuous_ranging()
		 * @sa set_measurement_timing_budget()
		 * @sa await_continuous_range(timer::RTT<TIMER>&, uint16_t&, uint16_t)
		 */
		bool await_continuous_range(uint16_t& range_mm, uint16_t loops = MAX_LOOP)
		{
			if (!await_interrupt(loops)) return false;
			if (!get_direct_range(range_mm)) return false;
			return clear_interrupt();
		}

		/**
		 * Perform a single range action on VL53L0X device, and wait for the 
		 * measurement result.
		 * This shall be used when no continuous ranging is in effect.
		 * Wait is performed based on a maximum number of loops, not on actual
		 * time. 
		 * This is more difficult to use than `await_single_range(timer::RTT<TIMER>&, uint16_t&, uint16_t)`
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @param range_mm a reference to a variable that will receive the 
		 * measurement result (in mm)
		 * @param loops the maximum number of waiting loops before timeout;
		 * default is 2000, but it may not be suitable in every situation.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed or timed out
		 * 
		 * @sa await_single_range(timer::RTT<TIMER>&, uint16_t&, uint16_t)
		 */
		bool await_single_range(uint16_t& range_mm, uint16_t loops = MAX_LOOP)
		{
			using READ_SYSRANGE = TReadRegisterFuture<Register::SYSRANGE_START>;
			using WRITE_SYSRANGE = TWriteRegisterFuture<Register::SYSRANGE_START>;
			if (!use_stop_variable()) return false;
			if (!this->template sync_write<WRITE_SYSRANGE>(uint8_t(0x01))) return false;
			// Read SYSRANGE until != 0x01
			while (loops--)
			{
				uint8_t sys_range = 0;
				if (!this->template sync_read<READ_SYSRANGE>(sys_range)) return false;
				if ((sys_range & 0x01) == 0)
					return await_continuous_range(range_mm, loops);
			}
			return false;
		}

		/**
		 * Get the reference SPADs status (enabled or not).
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @param spad_ref a reference to a variable that will receive the 
		 * current reference SPADs status
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_reference_SPADs()
		 */
		bool get_reference_SPADs(SPADReference& spad_ref)
		{
			using GetReferenceSPADsFuture = 
				TReadRegisterFuture<Register::GLOBAL_CONFIG_SPAD_ENABLES_REF_0, SPADReference>;
			return this->template sync_read<GetReferenceSPADsFuture, SPADReference>(spad_ref);
		}

		/**
		 * Set the reference SPADs status (enabled or not).
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @param spad_ref the new reference SPADs status to set
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_reference_SPADs()
		 */
		bool set_reference_SPADs(const SPADReference& spad_ref)
		{
			using SetReferenceSPADsFuture = 
				TWriteRegisterFuture<Register::GLOBAL_CONFIG_SPAD_ENABLES_REF_0, SPADReference>;
			if (!await_same_future_group(
				internals::set_reference_spads::BUFFER, internals::set_reference_spads::BUFFER_SIZE))
				return false;
			return this->template sync_write<SetReferenceSPADsFuture, SPADReference>(spad_ref);
		}

		/**
		 * Get current SPAD information (number of SPAD aperture or not).
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @param info a reference to a variable that will receive the 
		 * current reference SPAD information
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 */
		bool get_SPAD_info(SPADInfo& info)
		{
			using READ_SPAD = TReadRegisterFuture<Register::SPAD_INFO, SPADInfo>;
			using READ_STROBE = TReadRegisterFuture<Register::DEVICE_STROBE>;
			using WRITE_STROBE = TWriteRegisterFuture<Register::DEVICE_STROBE>;
			// 1. Write initial registers
			if (!await_same_future_group(internals::spad_info::BUFFER1, internals::spad_info::BUFFER1_SIZE))
				return false;
			// 2. Force strobe (read/write)
			uint8_t strobe = 0;
			if (!this->template sync_read<READ_STROBE>(strobe)) return false;
			strobe |= 0x04U;
			if (!this->template sync_write<WRITE_STROBE>(strobe)) return false;
			// 3. Write 2nd pass registers
			if (!await_same_future_group(internals::spad_info::BUFFER2, internals::spad_info::BUFFER2_SIZE))
				return false;
			// 4. Wait for strobe
			if (!await_device_strobe()) return false;
			// 5. Read spad info
			if (!this->template sync_read<READ_SPAD>(info)) return false;
			// 6. Write 3rd pass registers
			if (!await_same_future_group(internals::spad_info::BUFFER3, internals::spad_info::BUFFER3_SIZE))
				return false;
			// 7. Force strobe
			strobe = 0;
			if (!this->template sync_read<READ_STROBE>(strobe)) return false;
			strobe &= ~0x04U;
			if (!this->template sync_write<WRITE_STROBE>(strobe)) return false;
			// 8. Write last pass registers
			return await_same_future_group(internals::spad_info::BUFFER4, internals::spad_info::BUFFER4_SIZE);
		}

		/**
		 * Get current timeouts associated to each ranging step.
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @param timeouts a reference to a variable that will receive the 
		 * current timeouts associated to ranging steps
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 */
		bool get_sequence_steps_timeout(SequenceStepsTimeout& timeouts)
		{
			using READ_MSRC_TIMEOUT = TReadRegisterFuture<Register::MSRC_CONFIG_TIMEOUT_MACROP>;
			using READ_PRERANGE_TIMEOUT = TReadRegisterFuture<Register::PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			using READ_FINALRANGE_TIMEOUT = 
				TReadRegisterFuture<Register::FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			
			uint8_t pre_range_vcsel_period_pclks = 0;
			if (!get_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(pre_range_vcsel_period_pclks)) return false;
			uint8_t final_range_vcsel_period_pclks = 0;
			if (!get_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(final_range_vcsel_period_pclks)) return false;
			uint8_t msrc_dss_tcc_mclks = 0;
			if (!this->template sync_read<READ_MSRC_TIMEOUT>(msrc_dss_tcc_mclks)) return false;
			uint16_t pre_range_mclks = 0;
			if (!this->template sync_read<READ_PRERANGE_TIMEOUT>(pre_range_mclks)) return false;
			uint16_t final_range_mclks = 0;
			if (!this->template sync_read<READ_FINALRANGE_TIMEOUT>(final_range_mclks)) return false;

			timeouts = vl53l0x::SequenceStepsTimeout{
				pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks,
				msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks};
			return true;
		}

		/**
		 * Directly get value of a VL53L0X register.
		 * @warning Asynchronous API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @tparam REGISTER the register to read from VL53l)X device
		 * @tparam T the type of value stored in @p REGISTER
		 * 
		 * @param future a Future passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * 
		 * @sa set_register()
		 * @sa get_register(T& value)
		 */
		template<Register REGISTER, typename T = uint8_t>
		int get_register(PROXY<TReadRegisterFuture<REGISTER, T>> future)
		{
			return this->async_read(future);
		}

		/**
		 * Directly set value of a VL53L0X register.
		 * @warning Asynchronous API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @tparam REGISTER the register to write to VL53l)X device
		 * @tparam T the type of value stored in @p REGISTER
		 * 
		 * @param future a Future passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * 
		 * @sa get_register()
		 * @sa set_register(T value)
		 */
		template<Register REGISTER, typename T = uint8_t>
		int set_register(PROXY<TWriteRegisterFuture<REGISTER, T>> future)
		{
			return this->async_write(future);
		}

		/**
		 * Directly get value of a VL53L0X register.
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @tparam REGISTER the register to read from VL53l)X device
		 * @tparam T the type of value stored in @p REGISTER
		 * 
		 * @param value a reference to a variable of type @p T, that will receive 
		 * the value in register @p REGISTER
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa set_register(T)
		 * @sa get_register()
		 */
		template<Register REGISTER, typename T = uint8_t>
		bool get_register(T& value)
		{
			using GetRegisterFuture = TReadRegisterFuture<REGISTER, T>;
			return this->template sync_read<GetRegisterFuture>(value);
		}

		/**
		 * Directly set value of a VL53L0X register.
		 * @warning Blocking API!
		 * @note Low-level API! Generally you shall not use this API unless you
		 * know what you are doing.
		 * 
		 * @tparam REGISTER the register to write to VL53l)X device
		 * @tparam T the type of value stored in @p REGISTER
		 * 
		 * @param value a value of type @p T that will be written to register @p REGISTER
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa get_register(T& value)
		 * @sa set_register()
		 */
		template<Register REGISTER, typename T = uint8_t>
		bool set_register(T value)
		{
			using SetRegisterFuture = TWriteRegisterFuture<REGISTER, T>;
			return this->template sync_write<SetRegisterFuture>(value);
		}

	private:
		bool force_io_2_8V()
		{
			using READ_VHV_CONFIG = TReadRegisterFuture<Register::VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV>;
			using WRITE_VHV_CONFIG = TWriteRegisterFuture<Register::VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV>;
			uint8_t config = 0;
			if (!this->template sync_read<READ_VHV_CONFIG>(config)) return false;
			config |= 0x01;
			return this->template sync_write<WRITE_VHV_CONFIG>(config);
		}

		bool set_I2C_mode()
		{
			using WRITE_I2C_MODE = TWriteRegisterFuture<Register::SYSTEM_CONFIG_I2C_MODE>;
			return this->template sync_write<WRITE_I2C_MODE>(uint8_t(0x00));
		}

		bool read_stop_variable()
		{
			// Write prefix
			if (!await_same_future_group(
				internals::stop_variable::PRE_BUFFER, internals::stop_variable::PRE_BUFFER_SIZE))
				return false;

			// Read stop variable
			using READ_STOP_VAR = TReadRegisterFuture<Register::SYSTEM_STOP_VARIABLE>;
			if (!this->template sync_read<READ_STOP_VAR>(stop_variable_)) return false;

			// Write suffix
			return await_same_future_group(
				internals::stop_variable::POST_BUFFER, internals::stop_variable::POST_BUFFER_SIZE);
		}

		bool use_stop_variable()
		{
			// Write prefix
			if (!await_same_future_group(
				internals::stop_variable::PRE_BUFFER, internals::stop_variable::PRE_BUFFER_SIZE))
				return false;

			// Read stop variable
			using WRITE_STOP_VAR = TWriteRegisterFuture<Register::SYSTEM_STOP_VARIABLE>;
			if (!this->template sync_write<WRITE_STOP_VAR>(stop_variable_)) return false;

			// Write suffix
			return await_same_future_group(
				internals::stop_variable::POST_BUFFER, internals::stop_variable::POST_BUFFER_SIZE);
		}

		bool disable_signal_rate_limit_check()
		{
			using READ_MSRC_CONFIG = TReadRegisterFuture<Register::MSRC_CONFIG_CONTROL>;
			using WRITE_MSRC_CONFIG = TWriteRegisterFuture<Register::MSRC_CONFIG_CONTROL>;
			uint8_t config = 0;
			if (!this->template sync_read<READ_MSRC_CONFIG>(config)) return false;
			config |= 0x12;
			return this->template sync_write<WRITE_MSRC_CONFIG>(config);
		}

		bool load_tuning_settings()
		{
			return await_same_future_group(
				internals::load_tuning_settings::BUFFER, internals::load_tuning_settings::BUFFER_SIZE);
		}

		static constexpr const uint16_t MAX_LOOP = 2000;
		static constexpr uint16_t DEFAULT_TIMEOUT_MS = 100;

		enum class SingleRefCalibrationTarget : uint8_t
		{
			PHASE_CALIBRATION = 0x01,
			VHV_CALIBRATION = 0x41
		};

		bool perform_single_ref_calibration(SingleRefCalibrationTarget target)
		{
			using WRITE_SYS_RANGE = TWriteRegisterFuture<Register::SYSRANGE_START>;
			// 1. Write to register SYS RANGE
			if (!this->template sync_write<WRITE_SYS_RANGE>(uint8_t(target))) return false;
			// 2. Read interrupt status until interrupt occurs
			uint16_t loops = MAX_LOOP;
			while (loops--)
			{
				InterruptStatus status;
				if (!get_interrupt_status(status)) return false;
				if (uint8_t(status) != 0)
				{
					// 3. Clear interrupt
					if (!clear_interrupt(0x01)) return false;
					// 4. Write to register SYS RANGE
					return this->template sync_write<WRITE_SYS_RANGE>(uint8_t(0x00));
				}
			}
			return false;
		}

		bool await_same_future_group(const uint8_t* buffer, uint8_t size)
		{
			return i2c::await_same_future_group<MANAGER>(*this, buffer, size);
		}

		bool await_device_strobe()
		{
			using READ_STROBE = TReadRegisterFuture<Register::DEVICE_STROBE>;
			using WRITE_STROBE = TWriteRegisterFuture<Register::DEVICE_STROBE>;
			// 1. Clear strobe
			if (!this->template sync_write<WRITE_STROBE>(uint8_t(0x00))) return false;
			// 2. Read strobe until !=0
			uint16_t loops = MAX_LOOP;
			while (loops--)
			{
				uint8_t strobe = 0;
				if (!this->template sync_read<READ_STROBE>(strobe)) return false;
				if (strobe != 0)
					// 3. Set strobe
					return this->template sync_write<WRITE_STROBE>(uint8_t(0x01));
			}
			return false;
		}

		bool set_vcsel_pulse_period(VcselPeriodType type, uint8_t period)
		{
			// 0. Check period
			if (!check_vcsel_period(type, period)) return false;
			// 0'. Encode period
			uint8_t vcsel_period = encode_vcsel_period(period);
			// 0". Read current measurement timing budget
			uint32_t timing_budget = 0UL;
			if (!get_measurement_timing_budget(timing_budget)) return false;
			// 1. Read sequence steps enables
			SequenceSteps steps;
			if (!get_sequence_steps(steps)) return false;
			// 2. Read sequence steps timeouts
			SequenceStepsTimeout timeouts;
			if (!get_sequence_steps_timeout(timeouts)) return false;
			if (type == VcselPeriodType::PRE_RANGE)
			{
				if (!set_vcsel_period_pre_range(period, vcsel_period, timeouts)) return false;
			}
			else
			{
				if (!set_vcsel_period_final_range(period, vcsel_period, steps.is_pre_range(), timeouts))
					return false;
			}
			// 4. Set measurement timing budget as before
			if (!set_measurement_timing_budget(timing_budget)) return false;
			// 5. Perform phase calibration
			using WRITE_STEPS = TWriteRegisterFuture<Register::SYSTEM_SEQUENCE_CONFIG>;
			if (!this->template sync_write<WRITE_STEPS>(uint8_t(0x02))) return false;
			perform_single_ref_calibration(SingleRefCalibrationTarget::PHASE_CALIBRATION);
			if (!set_sequence_steps(steps)) return false;
			return true;
		}

		bool set_vcsel_period_pre_range(
			uint8_t period, uint8_t vcsel_period, const SequenceStepsTimeout& timeouts)
		{
			// 3.1. [PRE_RANGE]
			using WRITE_PHASE_HIGH = TWriteRegisterFuture<Register::PRE_RANGE_CONFIG_VALID_PHASE_HIGH>;
			using WRITE_PHASE_LOW = TWriteRegisterFuture<Register::PRE_RANGE_CONFIG_VALID_PHASE_LOW>;
			using WRITE_VCSEL = TWriteRegisterFuture<Register::PRE_RANGE_CONFIG_VCSEL_PERIOD>;
			using WRITE_RANGE_TIMEOUT = TWriteRegisterFuture<Register::PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			using WRITE_MSRC_TIMEOUT = TWriteRegisterFuture<Register::MSRC_CONFIG_TIMEOUT_MACROP>;

			// 3.1.1. Write PRE_RANGE_CONFIG_VALID_PHASE_HIGH
			uint8_t phase_high = 0;
			switch (period)
			{
				case 12:
				phase_high = 0x18;
				break;

				case 14:
				phase_high = 0x30;
				break;
				
				case 16:
				phase_high = 0x40;
				break;
				
				case 18:
				phase_high = 0x50;
				break;
				
				default:
				break;
			}
			if (!this->template sync_write<WRITE_PHASE_HIGH>(phase_high)) return false;
			// 3.1.2. Write PRE_RANGE_CONFIG_VALID_PHASE_LOW
			if (!this->template sync_write<WRITE_PHASE_LOW>(uint8_t(0x08))) return false;
			// 3.1.3. Write PRE_RANGE_CONFIG_VCSEL_PERIOD
			if (!this->template sync_write<WRITE_VCSEL>(vcsel_period)) return false;
			// 3.1.4. Write PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI
			// Calculate new pre-range timeout
			uint16_t pre_range_mclks = vl53l0x::TimeoutUtilities::encode_timeout(
				vl53l0x::TimeoutUtilities::calculate_timeout_mclks(timeouts.pre_range_us(), period));
			if (!this->template sync_write<WRITE_RANGE_TIMEOUT>(pre_range_mclks)) return false;
			// 3.1.5. Write MSRC_CONFIG_TIMEOUT_MACROP
			// Calculate new MSRC timeout
			uint16_t msrc_mclks = vl53l0x::TimeoutUtilities::calculate_timeout_mclks(
				timeouts.msrc_dss_tcc_us(), period);
			msrc_mclks = (msrc_mclks > (UINT8_MAX + 1)) ? UINT8_MAX : (msrc_mclks - 1);
			return this->template sync_write<WRITE_MSRC_TIMEOUT>(uint8_t(msrc_mclks));
		}

		bool set_vcsel_period_final_range(
			uint8_t period, uint8_t vcsel_period, bool has_pre_range, const SequenceStepsTimeout& timeouts)
		{
			// 3.2. [FINAL_RANGE]
			using WRITE_PHASE_HIGH = TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_VALID_PHASE_HIGH>;
			using WRITE_PHASE_LOW = TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_VALID_PHASE_LOW>;
			using WRITE_VCSEL_WIDTH = TWriteRegisterFuture<Register::GLOBAL_CONFIG_VCSEL_WIDTH>;
			using WRITE_PHASECAL_TIMEOUT = TWriteRegisterFuture<Register::ALGO_PHASECAL_CONFIG_TIMEOUT>;
			using WRITE_PHASECAL_LIMIT = TWriteRegisterFuture<Register::ALGO_PHASECAL_LIM>;
			using WRITE_RANGE_TIMEOUT = TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			using WRITE_VCSEL = TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_VCSEL_PERIOD>;
			// Determine values to write based on provided period
			uint8_t phase_high = 0;
			uint8_t vcsel_width = 0;
			uint8_t phasecal_timeout = 0;
			uint8_t phasecal_limit = 0;
			switch (period)
			{
				case 8:
				phase_high = 0x10;
				vcsel_width = 0x02;
				phasecal_timeout = 0x0C;
				phasecal_limit = 0x30;
				break;
				
				case 10:
				phase_high = 0x28;
				vcsel_width = 0x03;
				phasecal_timeout = 0x09;
				phasecal_limit = 0x20;
				break;
				
				case 12:
				phase_high = 0x38;
				vcsel_width = 0x03;
				phasecal_timeout = 0x08;
				phasecal_limit = 0x20;
				break;
				
				case 14:
				phase_high = 0x48;
				vcsel_width = 0x03;
				phasecal_timeout = 0x07;
				phasecal_limit = 0x20;
				break;

				default:
				break;
			}
			// 3.2.1. Write FINAL_RANGE_CONFIG_VALID_PHASE_HIGH
			if (!this->template sync_write<WRITE_PHASE_HIGH>(phase_high)) return false;
			// 3.2.2. Write FINAL_RANGE_CONFIG_VALID_PHASE_LOW
			if (!this->template sync_write<WRITE_PHASE_LOW>(uint8_t(0x08))) return false;
			// 3.2.3. Write GLOBAL_CONFIG_VCSEL_WIDTH
			if (!this->template sync_write<WRITE_VCSEL_WIDTH>(vcsel_width)) return false;
			// 3.2.4. Write ALGO_PHASECAL_CONFIG_TIMEOUT
			if (!this->template sync_write<WRITE_PHASECAL_TIMEOUT>(phasecal_timeout)) return false;
			// 3.2.5. Write ALGO_PHASECAL_LIM
			if (!this->template sync_write<WRITE_PHASECAL_LIMIT>(phasecal_limit)) return false;
			// 3.2.6. Write FINAL_RANGE_CONFIG_VCSEL_PERIOD
			if (!this->template sync_write<WRITE_VCSEL>(vcsel_period)) return false;
			// 3.2.7. Write FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI
			// Calculate new final-range timeout
			uint16_t final_range_mclks = vl53l0x::TimeoutUtilities::encode_timeout(
				vl53l0x::TimeoutUtilities::calculate_timeout_mclks(
					timeouts.final_range_us(has_pre_range), period));
			return this->template sync_write<WRITE_RANGE_TIMEOUT>(final_range_mclks);
		}

		static constexpr bool check_vcsel_period(VcselPeriodType type, uint8_t period)
		{
			if (type == VcselPeriodType::PRE_RANGE)
				return check_vcsel_period_pre_range(period);
			else
				return check_vcsel_period_final_range(period);
		}

		static constexpr bool check_vcsel_period_pre_range(uint8_t period)
		{
			switch (period)
			{
				case 12:
				case 14:
				case 16:
				case 18:
				return true;

				default:
				return false;
			}
		}

		static constexpr bool check_vcsel_period_final_range(uint8_t period)
		{
			switch (period)
			{
				case 8:
				case 10:
				case 12:
				case 14:
				return true;

				default:
				return false;
			}
		}

		static constexpr uint8_t encode_vcsel_period(uint8_t period)
		{
			return (period >> 1U) - 1U;
		}
		static constexpr uint8_t decode_vcsel_period(uint8_t value)
		{
			return (value + 1U) << 1U;
		}

		static constexpr const uint8_t NUM_REF_SPADS = 48;
		static constexpr const uint8_t SPADS_PER_BYTE = 8;
		static constexpr const uint8_t NUM_REF_SPADS_BYTES = NUM_REF_SPADS / SPADS_PER_BYTE;
		
		static constexpr const uint8_t FIRST_APERTURE_SPAD = 12;
		static void calculate_reference_SPADs(uint8_t ref_spads[NUM_REF_SPADS_BYTES], vl53l0x::SPADInfo info)
		{
			const uint8_t count = info.count();
			const uint8_t first_spad = (info.is_aperture() ? FIRST_APERTURE_SPAD : 0);
			uint8_t enabled_spads = 0;
			uint8_t spad = 0;
			for (uint8_t i = 0; i < NUM_REF_SPADS_BYTES; ++i)
			{
				uint8_t& ref_spad = ref_spads[i];
				for (uint8_t j = 0; j < SPADS_PER_BYTE; ++j)
				{
					if ((spad < first_spad) || (enabled_spads == count))
						// Disable this SPAD as it should not be enabled
						ref_spad &= bits::CBV8(j);
					else if (ref_spad & bits::BV8(j))
						// Just count the current SPAD as enabled
						++enabled_spads;
					++spad;
				}
			}
		}

		static constexpr const uint32_t MIN_TIMING_BUDGET    = 20'000UL;
		static constexpr const uint16_t START_OVERHEAD_SET   = 1320U;
		static constexpr const uint16_t START_OVERHEAD_GET   = 1910U;
		static constexpr const uint16_t END_OVERHEAD         = 960U;
		static constexpr const uint16_t MSRC_OVERHEAD        = 660U;
		static constexpr const uint16_t TCC_OVERHEAD         = 590U;
		static constexpr const uint16_t DSS_OVERHEAD         = 690U;
		static constexpr const uint16_t PRE_RANGE_OVERHEAD   = 660U;
		static constexpr const uint16_t FINAL_RANGE_OVERHEAD = 550U;

		static uint32_t calculate_measurement_budget_us(
			bool get, const vl53l0x::SequenceSteps steps, const vl53l0x::SequenceStepsTimeout& timeouts)
		{
			// start and end overhead times always present
			uint32_t budget_us = (get ? START_OVERHEAD_GET : START_OVERHEAD_SET) + END_OVERHEAD;

			if (steps.is_tcc())
				budget_us += timeouts.msrc_dss_tcc_us() + TCC_OVERHEAD;

			if (steps.is_dss())
				budget_us += 2 * (timeouts.msrc_dss_tcc_us() + DSS_OVERHEAD);
			else if (steps.is_msrc())
				budget_us += timeouts.msrc_dss_tcc_us() + MSRC_OVERHEAD;

			if (steps.is_pre_range())
				budget_us += timeouts.pre_range_us() + PRE_RANGE_OVERHEAD;

			if (steps.is_final_range())
				budget_us += timeouts.final_range_us(steps.is_pre_range()) + FINAL_RANGE_OVERHEAD;

			return budget_us;
		}

		static uint16_t calculate_final_range_timeout(
			const vl53l0x::SequenceSteps steps, const vl53l0x::SequenceStepsTimeout& timeouts, uint32_t budget_us)
		{
			// Requested budget must be be above minimum allowed
			if (budget_us < MIN_TIMING_BUDGET) return 0;
			// This calculation is useless if there is no final range step
			if (!steps.is_final_range()) return 0;

			// Calculate current used budget without final range
			uint32_t used_budget_us = 
				calculate_measurement_budget_us(false, steps.no_final_range(), timeouts);

			// Now include final range and calculate difference
			used_budget_us += FINAL_RANGE_OVERHEAD;
			// Requested budget must be above calculated budget for all other steps
			if (used_budget_us > budget_us) return 0;

			// Calculate final range timeout in us
			const uint32_t final_range_timeout_us = budget_us - used_budget_us;

			// Deduce final range timeout in mclks
			uint32_t final_range_timeout_mclks = vl53l0x::TimeoutUtilities::calculate_timeout_mclks(
				final_range_timeout_us, timeouts.final_range_vcsel_period_pclks());
			if (steps.is_pre_range())
				final_range_timeout_mclks += timeouts.pre_range_mclks();
			
			return vl53l0x::TimeoutUtilities::encode_timeout(final_range_timeout_mclks);
		}

		static constexpr const uint8_t DEFAULT_DEVICE_ADDRESS = 0x52;

		// Stop variable used across device invocations
		uint8_t stop_variable_ = 0;
	};
}

#endif /* VL53L0X_H */
/// @endcond
