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
 * Note that most API here has been adapted and improved from official 
 * STMicroelectronics C-library API; this was necessary as the device datasheet
 * does not describe the internals (registers) of the chip, the only way to
 * understand how it works was thus to analyze the API source code.
 * 
 * @sa https://www.st.com/content/st_com/en/products/embedded-software/proximity-sensors-software/stsw-img005.html
 */

#ifndef VL53L0X_H
#define VL53L0X_H

#include "../array.h"
#include "../flash.h"
#include "../i2c.h"
#include "../future.h"
#include "../realtime_timer.h"
#include "../time.h"
#include "../utilities.h"
#include "../i2c_handler.h"
#include "../i2c_device.h"
#include "../i2c_device_utilities.h"
#include "vl53l0x_internals.h"
#include "vl53l0x_futures.h"
#include "vl53l0x_registers.h"
#include "vl53l0x_types.h"

namespace devices
{
	/**
	 * Defines API for VL53L0X Time-of-Flight ranging sensor chip usage.
	 */
	namespace vl53l0x
	{
	}
}

//TODO - implement low-level API step by step
//       - set_address
//
//TODO - implement higher level API (including ranging profiles)
//       - begin()
//       - standby()
//       - ranging()
//
//TODO - check what of the remaing API shall be implemented or not
// OPEN POINTS:
// - calibration mode or only hard-coded calibration?

namespace devices::vl53l0x
{
	/// @cond notdocumented
	namespace internals = vl53l0x_internals;
	/// @endcond

	/**
	 * I2C device driver for the VL53L0X ToF ranging chip.
	 * This chip supports both standard and fast I2C modes.
	 * 
	 * @tparam MANAGER one of FastArduino available I2C Manager
	 */
	template<typename MANAGER>
	class VL53L0X : public i2c::I2CDevice<MANAGER>
	{
	private:
		using FUTURES = vl53l0x_futures::Futures<MANAGER>;

		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;

	public:
		/**
		 * Create a new device driver for a VL53L0X chip.
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 */
		explicit VL53L0X(MANAGER& manager) : PARENT{manager, DEFAULT_DEVICE_ADDRESS, i2c::I2C_FAST, false} {}

		// High-level API
		bool begin(Profile profile)
		{
			if (!init_data_first()) return false;
			uint8_t prof = uint8_t(profile);
			if (!init_static_second(GPIOSettings::sample_ready(), SequenceSteps::create().pre_range().final_range()))
				return false;
			if (!perform_ref_calibration()) return false;
			if (prof & 0x01)
			{
				// long range
				if (!set_vcsel_pulse_period<VcselPeriodType::PRE_RANGE>(18)) return false;
				if (!set_vcsel_pulse_period<VcselPeriodType::FINAL_RANGE>(14)) return false;
				if (!set_signal_rate_limit(0.1)) return false;
			}
			if (prof & 0x02)
			{
				// accurate
				if (!set_measurement_timing_budget(200000)) return false;
			}
			else if (prof & 0x04)
			{
				// fast
				if (!set_measurement_timing_budget(20000)) return false;
			}
			return true;
		}
		
		//TODO Review all API to include arguments check whenever needed!

		// Asynchronous API
		//==================
		// Low-level API: use only if you know what you are doing!
		template<Register REGISTER, typename T = uint8_t>
		int get_register(PROXY<typename FUTURES::TReadRegisterFuture<REGISTER, T>> future)
		{
			return this->async_read(future);
		}
		template<Register REGISTER, typename T = uint8_t>
		int set_register(PROXY<typename FUTURES::TWriteRegisterFuture<REGISTER, T>> future)
		{
			return this->async_write(future);
		}

		using GetRangeStatusFuture = 
			typename FUTURES::TReadRegisterFuture<Register::RESULT_RANGE_STATUS, DeviceStatus>;
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return this->async_read(future);
		}

		using GetGPIOSettingsFuture = typename FUTURES::GetGPIOSettingsFuture;
		int get_GPIO_settings(GetGPIOSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		using SetGPIOSettingsFuture = typename FUTURES::SetGPIOSettingsFuture;
		int set_GPIO_settings(SetGPIOSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		using GetInterruptStatusFuture = 
			typename FUTURES::TReadRegisterFuture<Register::RESULT_INTERRUPT_STATUS, InterruptStatus>;
		int get_interrupt_status(PROXY<GetInterruptStatusFuture> future)
		{
			return this->async_read(future);
		}

		using ClearInterruptFuture = typename FUTURES::ClearInterruptFuture;
		int clear_interrupt(PROXY<ClearInterruptFuture> future)
		{
			return this->async_write(future);
		}

		// This API shall be used only after InterruptStatus != 0, Interrupt Status should be clear immediately after it
		using GetDirectRangeFuture = typename FUTURES::TReadRegisterFuture<Register::RESULT_RANGE_MILLIMETER, uint16_t>;
		int get_direct_range(PROXY<GetDirectRangeFuture> future)
		{
			return this->async_read(future);
		}

		// Synchronous API
		//=================
		// Low-level API: use only if you know what you are doing!
		template<Register REGISTER, typename T = uint8_t>
		bool get_register(T& value)
		{
			using GetRegisterFuture = typename FUTURES::TReadRegisterFuture<REGISTER, T>;
			return this->template sync_read<GetRegisterFuture>(value);
		}
		template<Register REGISTER, typename T = uint8_t>
		bool set_register(T value)
		{
			using SetRegisterFuture = typename FUTURES::TWriteRegisterFuture<REGISTER, T>;
			return this->template sync_write<SetRegisterFuture>(value);
		}

		bool set_address(uint8_t device_address)
		{
			using SetAddressFuture = typename FUTURES::TWriteRegisterFuture<Register::I2C_SLAVE_DEVICE_ADDRESS>;
			device_address &= 0x7F;
			if (!this->template sync_write<SetAddressFuture>(device_address)) return false;
			this->set_device(device_address << 1);
			return true;
		}

		bool get_model(uint8_t& model)
		{
			using GetModelFuture = typename FUTURES::TReadRegisterFuture<Register::IDENTIFICATION_MODEL_ID>;
			return this->template sync_read<GetModelFuture>(model);
		}
		bool get_revision(uint8_t& revision)
		{
			using GetRevisionFuture = typename FUTURES::TReadRegisterFuture<Register::IDENTIFICATION_REVISION_ID>;
			return this->template sync_read<GetRevisionFuture>(revision);
		}
		bool get_power_mode(PowerMode& power_mode)
		{
			using GetPowerModeFuture = typename FUTURES::TReadRegisterFuture<Register::POWER_MANAGEMENT, PowerMode>;
			return this->template sync_read<GetPowerModeFuture>(power_mode);
		}
		bool get_range_status(DeviceStatus& range_status)
		{
			return this->template sync_read<GetRangeStatusFuture>(range_status);
		}
		bool get_sequence_steps(SequenceSteps& sequence_steps)
		{
			using GetSequenceStepsFuture = 
				typename FUTURES::TReadRegisterFuture<Register::SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
			return this->template sync_read<GetSequenceStepsFuture>(sequence_steps);
		}
		bool set_sequence_steps(SequenceSteps sequence_steps)
		{
			using SetSequenceStepsFuture = 
				typename FUTURES::TWriteRegisterFuture<Register::SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
			return this->template sync_write<SetSequenceStepsFuture>(sequence_steps);
		}

		template<VcselPeriodType TYPE>
		bool get_vcsel_pulse_period(uint8_t& period)
		{
			using GetVcselPulsePeriodFuture = typename FUTURES::TReadRegisterFuture<Register((uint8_t) TYPE)>;
			if (!this->template sync_read<GetVcselPulsePeriodFuture>(period)) return false;
			period = decode_vcsel_period(period);
			return true;
		}

		template<VcselPeriodType TYPE>
		bool set_vcsel_pulse_period(uint8_t period)
		{
			return set_vcsel_pulse_period(TYPE, period);
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
				if (!set_core_vcsel_pulse_period_pre_range(period, vcsel_period, timeouts)) return false;
			}
			else
			{
				if (!set_core_vcsel_pulse_period_final_range(period, vcsel_period, steps.is_pre_range(), timeouts))
					return false;
			}
			// 4. Set measurement timing budget as before
			if (!set_measurement_timing_budget(timing_budget)) return false;
			// 5. Perform phase calibration
			using WRITE_STEPS = typename FUTURES::TWriteRegisterFuture<Register::SYSTEM_SEQUENCE_CONFIG>;
			if (!this->template sync_write<WRITE_STEPS>(uint8_t(0x02))) return false;
			perform_single_ref_calibration(SingleRefCalibrationTarget::PHASE_CALIBRATION);
			if (!set_sequence_steps(steps)) return false;
			return true;
		}

		bool get_signal_rate_limit(float& signal_rate)
		{
			using GetSignalRateLimitFuture = 
				typename FUTURES::TReadRegisterFuture<Register::FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;
			uint16_t temp = 0;
			if (!this->template sync_read<GetSignalRateLimitFuture>(temp)) return false;
			signal_rate = FixPoint9_7::convert(temp);
			return true;
		}

		bool set_signal_rate_limit(float signal_rate)
		{
			using SetSignalRateLimitFuture = 
				typename FUTURES::TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;
			return this->template sync_write<SetSignalRateLimitFuture>(FixPoint9_7::convert(signal_rate));
		}

		bool get_reference_SPADs(SPADReference& spad_ref)
		{
			using GetReferenceSPADsFuture = 
				typename FUTURES::TReadRegisterFuture<Register::GLOBAL_CONFIG_SPAD_ENABLES_REF_0, SPADReference>;
			return this->template sync_read<GetReferenceSPADsFuture, SPADReference>(spad_ref);
		}

		bool set_reference_SPADs(const SPADReference& spad_ref)
		{
			using SetReferenceSPADsFuture = 
				typename FUTURES::TWriteRegisterFuture<Register::GLOBAL_CONFIG_SPAD_ENABLES_REF_0, SPADReference>;
			if (!await_same_future_group(
				internals::set_reference_spads::BUFFER, internals::set_reference_spads::BUFFER_SIZE))
				return false;
			return this->template sync_write<SetReferenceSPADsFuture, SPADReference>(spad_ref);
		}

		bool get_SPAD_info(SPADInfo& info)
		{
			using READ_SPAD = typename FUTURES::TReadRegisterFuture<Register::SPAD_INFO, SPADInfo>;
			using READ_STROBE = typename FUTURES::TReadRegisterFuture<Register::DEVICE_STROBE>;
			using WRITE_STROBE = typename FUTURES::TWriteRegisterFuture<Register::DEVICE_STROBE>;
			// 1. Write initial registers
			if (!await_same_future_group(internals::spad_info::BUFFER1, internals::spad_info::BUFFER1_SIZE))
				return false;
			// 2. Force strobe (read/write)
			uint8_t strobe = 0;
			if (!this->template sync_read<READ_STROBE>(strobe)) return false;
			strobe |= 0x04;
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
			strobe &= ~0x04;
			if (!this->template sync_write<WRITE_STROBE>(strobe)) return false;
			// 8. Write last pass registers
			return await_same_future_group(internals::spad_info::BUFFER4, internals::spad_info::BUFFER4_SIZE);
		}

		bool get_sequence_steps_timeout(SequenceStepsTimeout& timeouts)
		{
			using READ_MSRC_TIMEOUT = typename FUTURES::TReadRegisterFuture<Register::MSRC_CONFIG_TIMEOUT_MACROP>;
			using READ_PRERANGE_TIMEOUT =
				typename FUTURES::TReadRegisterFuture<Register::PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			using READ_FINALRANGE_TIMEOUT =
				typename FUTURES::TReadRegisterFuture<Register::FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			
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

		bool get_measurement_timing_budget(uint32_t& budget_us)
		{
			// Get steps and timeouts
			SequenceSteps steps{};
			if (!get_sequence_steps(steps)) return false;
			SequenceStepsTimeout timeouts{};
			if (!get_sequence_steps_timeout(timeouts)) return false;
			// Calculate timing budget
			budget_us = calculate_measurement_timing_budget_us(true, steps, timeouts);
			return true;
		}

		bool set_measurement_timing_budget(uint32_t budget_us)
		{
			using WRITE_BUDGET = typename
				FUTURES::TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			SequenceSteps steps;
			if (!get_sequence_steps(steps)) return false;
			SequenceStepsTimeout timeouts;
			if (!get_sequence_steps_timeout(timeouts)) return false;
			// Calculate budget
			uint16_t budget = calculate_final_range_timeout_mclks(steps, timeouts, budget_us);
			if (budget == 0) return false;
			return this->template sync_write<WRITE_BUDGET>(budget);
		}

		bool get_GPIO_settings(GPIOSettings& settings)
		{
			GetGPIOSettingsFuture future{};
			if (get_GPIO_settings(future) != 0) return false;
			return future.get(settings);
		}

		bool set_GPIO_settings(const GPIOSettings& settings)
		{
			SetGPIOSettingsFuture future{settings};
			if (set_GPIO_settings(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		static constexpr uint16_t DEFAULT_TIMEOUT_MS = 100;
		template<board::Timer TIMER>
		bool await_interrupt(timer::RTT<TIMER>& rtt, uint16_t timeout_ms = DEFAULT_TIMEOUT_MS)
		{
			time::RTTTime end = rtt.time() + time::RTTTime{timeout_ms, 0};
			while (rtt.time() < end)
			{
				InterruptStatus status;
				if (!this->template sync_read<GetInterruptStatusFuture>(status)) return false;
				if (status != 0)
					return true;
			}
			return false;
		}

		bool await_interrupt(uint16_t loops = MAX_LOOP)
		{
			// Read interrupt until !=0
			while (loops--)
			{
				InterruptStatus status;
				if (!this->template sync_read<GetInterruptStatusFuture>(status)) return false;
				if (status != 0)
					return true;
			}
			return false;
		}

		bool get_interrupt_status(InterruptStatus& status)
		{
			return this->template sync_read<GetInterruptStatusFuture>(status);
		}

		bool clear_interrupt(uint8_t clear_mask = 0x01)
		{
			return this->template sync_write<ClearInterruptFuture>(clear_mask);
		}

		bool start_continuous_ranging(uint16_t period_ms = 0)
		{
			using READ_OSC_CAL = typename FUTURES::TReadRegisterFuture<Register::OSC_CALIBRATE_VAL, uint16_t>;
			using WRITE_PERIOD = typename FUTURES::TWriteRegisterFuture<Register::SYSTEM_INTERMEASUREMENT_PERIOD, uint32_t>;
			using WRITE_SYSRANGE = typename FUTURES::TWriteRegisterFuture<Register::SYSRANGE_START>;
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

		template<board::Timer TIMER> bool await_continuous_range(
			timer::RTT<TIMER>& rtt, uint16_t& range_mm, uint16_t timeout_ms = DEFAULT_TIMEOUT_MS)
		{
			if (!await_interrupt(rtt, timeout_ms)) return false;
			if (!get_direct_range(range_mm)) return false;
			return clear_interrupt();
		}

		bool await_continuous_range(uint16_t& range_mm, uint16_t loops = MAX_LOOP)
		{
			if (!await_interrupt(loops)) return false;
			if (!get_direct_range(range_mm)) return false;
			return clear_interrupt();
		}

		// This API shall be used only after InterruptStatus != 0, Interrupt Status should be clear immediately after it
		bool get_direct_range(uint16_t& range_mm)
		{
			return this->template sync_read<GetDirectRangeFuture>(range_mm);
		}

		bool stop_continuous_ranging()
		{
			return await_same_future_group(
				internals::stop_continuous_ranging::BUFFER, internals::stop_continuous_ranging::BUFFER_SIZE);
		}

		template<board::Timer TIMER>
		bool await_single_range(timer::RTT<TIMER>& rtt, uint16_t& range_mm, uint16_t timeout_ms = DEFAULT_TIMEOUT_MS)
		{
			time::RTTTime end = rtt.time() + time::RTTTime{timeout_ms, 0};
			using READ_SYSRANGE = typename FUTURES::TReadRegisterFuture<Register::SYSRANGE_START>;
			using WRITE_SYSRANGE = typename FUTURES::TWriteRegisterFuture<Register::SYSRANGE_START>;
			if (!use_stop_variable()) return false;
			if (!this->template sync_write<WRITE_SYSRANGE>(uint8_t(0x01))) return false;
			// Read SYSRANGE until != 0x01
			while (rtt.time() < end)
			{
				uint8_t sys_range = 0;
				if (!this->template sync_read<READ_SYSRANGE>(sys_range)) return false;
				if (!(sys_range & 0x01))
					return await_continuous_range(rtt, range_mm, timeout_ms);
			}
			return false;
		}

		bool await_single_range(uint16_t& range_mm, uint16_t loops = MAX_LOOP)
		{
			using READ_SYSRANGE = typename FUTURES::TReadRegisterFuture<Register::SYSRANGE_START>;
			using WRITE_SYSRANGE = typename FUTURES::TWriteRegisterFuture<Register::SYSRANGE_START>;
			if (!use_stop_variable()) return false;
			if (!this->template sync_write<WRITE_SYSRANGE>(uint8_t(0x01))) return false;
			// Read SYSRANGE until != 0x01
			while (loops--)
			{
				uint8_t sys_range = 0;
				if (!this->template sync_read<READ_SYSRANGE>(sys_range)) return false;
				if (!(sys_range & 0x01))
					return await_continuous_range(range_mm, loops);
			}
			return false;
		}

		bool init_data_first()
		{
			// 1. Force 2.8V for I/O (instead of default 1.8V)
			if (!force_io_2_8V()) return false;
			// 2. Set I2C standard mode
			if (!set_I2C_mode()) return false;
			// 3. Read stop variable here
			if (!read_stop_variable()) return false;
			// 4. Disable SIGNAL_RATE_MSRC and SIGNAL_RATE_PRE_RANGE limit checks
			if (!disable_signal_rate_limit_checks()) return false;
			// 5. Set signal rate limit to 0.25 MCPS (million counts per second) in FP9.7 format
			if (!set_signal_rate_limit(0.25)) return false;
			// 6. Enable all sequence steps by default
			return set_sequence_steps((SequenceSteps) 0xFF);
		}

		bool init_static_second(const GPIOSettings& settings, SequenceSteps steps)
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

		bool perform_ref_calibration()
		{
			using WRITE_STEPS = typename FUTURES::TWriteRegisterFuture<Register::SYSTEM_SEQUENCE_CONFIG>;
			// 1. Read current sequence steps
			SequenceSteps steps;
			if (!get_sequence_steps(steps)) return false;
			// 2. Set steps for VHV calibration
			if (!this->template sync_write<WRITE_STEPS>(uint8_t(0x01))) return false;
			// 3. Perform single VHV calibration
			if (!perform_single_ref_calibration(SingleRefCalibrationTarget::VHV_CALIBRATION)) return false;
			// 4. Set steps for Phase calibration
			if (!this->template sync_write<WRITE_STEPS>(uint8_t(0x02))) return false;
			// 5. Perform single Phase calibration
			if (!perform_single_ref_calibration(SingleRefCalibrationTarget::PHASE_CALIBRATION)) return false;
			// 6. Restore sequence steps (NOTE: 0x00 is used as marker by the future to actually restore saved sequence)
			return set_sequence_steps(steps);
		}

		bool reset_device()
		{
			using WRITE_RESET = typename FUTURES::TWriteRegisterFuture<Register::SOFT_RESET_GO2_SOFT_RESET_N>;
			// Set reset bit
			if (!this->template sync_write<WRITE_RESET>(uint8_t(0x00))) return false;
			// Wait for some time
			uint8_t model = 0xFF;
			do
			{
				get_model(model);
			}
			while (model != 0);
			time::delay_us(100);

			// Release reset
			if (!this->template sync_write<WRITE_RESET>(uint8_t(0x01))) return false;
			// Wait until correct boot-up
			model = 0x00;
			do
			{
				get_model(model);
			}
			while (model == 0);
			time::delay_us(100);
			return true;
		}

	private:
		bool force_io_2_8V()
		{
			using READ_VHV_CONFIG = typename FUTURES::TReadRegisterFuture<Register::VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV>;
			using WRITE_VHV_CONFIG = typename FUTURES::TWriteRegisterFuture<Register::VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV>;
			uint8_t config = 0;
			if (!this->template sync_read<READ_VHV_CONFIG>(config)) return false;
			config |= 0x01;
			return this->template sync_write<WRITE_VHV_CONFIG>(config);
		}

		bool set_I2C_mode()
		{
			using WRITE_I2C_MODE = typename FUTURES::TWriteRegisterFuture<Register::SYSTEM_CONFIG_I2C_MODE>;
			return this->template sync_write<WRITE_I2C_MODE>(uint8_t(0x00));
		}

		bool read_stop_variable()
		{
			// Write prefix
			if (!await_same_future_group(
				internals::stop_variable::PRE_BUFFER, internals::stop_variable::PRE_BUFFER_SIZE))
				return false;

			// Read stop variable
			using READ_STOP_VAR = typename FUTURES::TReadRegisterFuture<Register::SYSTEM_STOP_VARIABLE>;
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
			using WRITE_STOP_VAR = typename FUTURES::TWriteRegisterFuture<Register::SYSTEM_STOP_VARIABLE>;
			if (!this->template sync_write<WRITE_STOP_VAR>(stop_variable_)) return false;

			// Write suffix
			return await_same_future_group(
				internals::stop_variable::POST_BUFFER, internals::stop_variable::POST_BUFFER_SIZE);
		}

		bool disable_signal_rate_limit_checks()
		{
			using READ_MSRC_CONFIG = typename FUTURES::TReadRegisterFuture<Register::MSRC_CONFIG_CONTROL>;
			using WRITE_MSRC_CONFIG = typename FUTURES::TWriteRegisterFuture<Register::MSRC_CONFIG_CONTROL>;
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

		bool perform_single_ref_calibration(SingleRefCalibrationTarget target)
		{
			using WRITE_SYS_RANGE = typename FUTURES::TWriteRegisterFuture<Register::SYSRANGE_START>;
			// 1. Write to register SYS RANGE
			if (!this->template sync_write<WRITE_SYS_RANGE>(uint8_t(target))) return false;
			// 2. Read interrupt status until interrupt occurs
			uint16_t loops = MAX_LOOP;
			while (loops--)
			{
				InterruptStatus status;
				if (!get_interrupt_status(status)) return false;
				if (status != 0)
				{
					// 3. Clear interrupt
					if (!clear_interrupt(0x01)) return false;
					// 4. Write to register SYS RANGE
					return this->template sync_write<WRITE_SYS_RANGE>(uint8_t(0x00));
				}
			}
			return false;
		}

		//TODO Move this API to AbstractDevice?
		bool await_same_future_group(const uint8_t* buffer, uint8_t size)
		{
			typename FUTURES::I2CSameFutureGroup future{uint16_t(buffer), size};
			if (!future.start(*this)) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		bool await_device_strobe()
		{
			using READ_STROBE = typename FUTURES::TReadRegisterFuture<Register::DEVICE_STROBE>;
			using WRITE_STROBE = typename FUTURES::TWriteRegisterFuture<Register::DEVICE_STROBE>;
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

		bool set_core_vcsel_pulse_period_pre_range(
			uint8_t period, uint8_t vcsel_period, const SequenceStepsTimeout& timeouts)
		{
			// 3.1. [PRE_RANGE]
			using WRITE_PHASE_HIGH = 
				typename FUTURES::TWriteRegisterFuture<Register::PRE_RANGE_CONFIG_VALID_PHASE_HIGH>;
			using WRITE_PHASE_LOW = 
				typename FUTURES::TWriteRegisterFuture<Register::PRE_RANGE_CONFIG_VALID_PHASE_LOW>;
			using WRITE_VCSEL = 
				typename FUTURES::TWriteRegisterFuture<Register::PRE_RANGE_CONFIG_VCSEL_PERIOD>;
			using WRITE_RANGE_TIMEOUT = 
				typename FUTURES::TWriteRegisterFuture<Register::PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			using WRITE_MSRC_TIMEOUT = 
				typename FUTURES::TWriteRegisterFuture<Register::MSRC_CONFIG_TIMEOUT_MACROP>;

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
			msrc_mclks = (msrc_mclks > 256) ? 255 : (msrc_mclks - 1);
			return this->template sync_write<WRITE_MSRC_TIMEOUT>(uint8_t(msrc_mclks));
		}

		bool set_core_vcsel_pulse_period_final_range(
			uint8_t period, uint8_t vcsel_period, bool has_pre_range, const SequenceStepsTimeout& timeouts)
		{
			// 3.2. [FINAL_RANGE]
			using WRITE_PHASE_HIGH = 
				typename FUTURES::TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_VALID_PHASE_HIGH>;
			using WRITE_PHASE_LOW = 
				typename FUTURES::TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_VALID_PHASE_LOW>;
			using WRITE_VCSEL_WIDTH = 
				typename FUTURES::TWriteRegisterFuture<Register::GLOBAL_CONFIG_VCSEL_WIDTH>;
			using WRITE_PHASECAL_TIMEOUT = 
				typename FUTURES::TWriteRegisterFuture<Register::ALGO_PHASECAL_CONFIG_TIMEOUT>;
			using WRITE_PHASECAL_LIMIT = 
				typename FUTURES::TWriteRegisterFuture<Register::ALGO_PHASECAL_LIM>;
			using WRITE_RANGE_TIMEOUT = 
				typename FUTURES::TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			using WRITE_VCSEL = 
				typename FUTURES::TWriteRegisterFuture<Register::FINAL_RANGE_CONFIG_VCSEL_PERIOD>;
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
			return (period >> 1) - 1;
		}
		static constexpr uint8_t decode_vcsel_period(uint8_t value)
		{
			return (value + 1) << 1;
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

		static constexpr const uint32_t MIN_TIMING_BUDGET    = 20000UL;
		static constexpr const uint16_t START_OVERHEAD_SET   = 1320U;
		static constexpr const uint16_t START_OVERHEAD_GET   = 1910U;
		static constexpr const uint16_t END_OVERHEAD         = 960U;
		static constexpr const uint16_t MSRC_OVERHEAD        = 660U;
		static constexpr const uint16_t TCC_OVERHEAD         = 590U;
		static constexpr const uint16_t DSS_OVERHEAD         = 690U;
		static constexpr const uint16_t PRE_RANGE_OVERHEAD   = 660U;
		static constexpr const uint16_t FINAL_RANGE_OVERHEAD = 550U;

		static uint32_t calculate_measurement_timing_budget_us(
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

		static uint16_t calculate_final_range_timeout_mclks(
			const vl53l0x::SequenceSteps steps, const vl53l0x::SequenceStepsTimeout& timeouts, uint32_t budget_us)
		{
			// Requested budget must be be above minimum allowed
			if (budget_us < MIN_TIMING_BUDGET) return 0;
			// This calculation is useless if there is no final range step
			if (!steps.is_final_range()) return 0;

			// Calculate current used budget without final range
			uint32_t used_budget_us = 
				calculate_measurement_timing_budget_us(false, steps.no_final_range(), timeouts);

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

		friend vl53l0x_futures::Futures<MANAGER>;
	};
}

#endif /* VL53L0X_H */
/// @endcond
