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

//TODO review which API can still be async/sync and those that are only sync
//TODO - implement low-level API step by step
//       - static_init
//       - single ranging
//       - status
//       - calibration?
// 
//       - continuous ranging
//       - set_address
//       - interrupt handling
//
//TODO - implement high level API
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
	namespace regs = vl53l0x_registers;
	namespace actions = i2c::actions;
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

		// Asynchronous API
		//==================
		//TODO sync-only?
		using GetRangeStatusFuture = 
			typename FUTURES::TReadRegisterFuture<regs::REG_RESULT_RANGE_STATUS, DeviceStatus>;
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return this->async_read(future);
		}

		//TODO sync-only
		using GetSequenceStepsTimeoutFuture = typename FUTURES::GetSequenceStepsTimeoutFuture;
		int get_sequence_steps_timeout(GetSequenceStepsTimeoutFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		//TODO sync-only
		using GetMeasurementTimingBudgetFuture = typename FUTURES::GetMeasurementTimingBudgetFuture;
		int get_measurement_timing_budget(GetMeasurementTimingBudgetFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
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
			typename FUTURES::TReadRegisterFuture<regs::REG_RESULT_INTERRUPT_STATUS, InterruptStatus>;
		int get_interrupt_status(PROXY<GetInterruptStatusFuture> future)
		{
			return this->async_read(future);
		}

		using ClearInterruptFuture = typename FUTURES::ClearInterruptFuture;
		int clear_interrupt(PROXY<ClearInterruptFuture> future)
		{
			return this->async_write(future);
		}

		//TODO sync-only
		//TODO private only
		using LoadTuningSettingsFuture = typename FUTURES::LoadTuningSettingsFuture;
		int load_tuning_settings(LoadTuningSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		//TODO Reading sensor range


		// Synchronous API
		//=================
		bool set_address(uint8_t device_address)
		{
			//TODO
			// set_device(device_address);
		}

		//TODO define all needed API here

		bool get_model(uint8_t& model)
		{
			using GetModelFuture = typename FUTURES::TReadRegisterFuture<regs::REG_IDENTIFICATION_MODEL_ID>;
			return this->template sync_read<GetModelFuture>(model);
		}
		bool get_revision(uint8_t& revision)
		{
			using GetRevisionFuture = typename FUTURES::TReadRegisterFuture<regs::REG_IDENTIFICATION_REVISION_ID>;
			return this->template sync_read<GetRevisionFuture>(revision);
		}
		bool get_power_mode(PowerMode& power_mode)
		{
			using GetPowerModeFuture = typename FUTURES::TReadRegisterFuture<regs::REG_POWER_MANAGEMENT, PowerMode>;
			return this->template sync_read<GetPowerModeFuture>(power_mode);
		}
		bool get_range_status(DeviceStatus& range_status)
		{
			return this->template sync_read<GetRangeStatusFuture>(range_status);
		}
		bool get_sequence_steps(SequenceSteps& sequence_steps)
		{
			using GetSequenceStepsFuture = 
				typename FUTURES::TReadRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
			return this->template sync_read<GetSequenceStepsFuture>(sequence_steps);
		}
		bool set_sequence_steps(SequenceSteps sequence_steps)
		{
			using SetSequenceStepsFuture = 
				typename FUTURES::TWriteRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
			return this->template sync_write<SetSequenceStepsFuture>(sequence_steps);
		}
		template<VcselPeriodType TYPE>
		bool get_vcsel_pulse_period(uint8_t& period)
		{
			using GetVcselPulsePeriodFuture = typename FUTURES::GetVcselPulsePeriodFuture<TYPE>;
			return this->template sync_read<GetVcselPulsePeriodFuture>(period);
		}
		//TODO Much more complex process needed here: to be thought about further!
		template<VcselPeriodType TYPE>
		bool set_vcsel_pulse_period(uint8_t period)
		{
			using SetVcselPulsePeriodFuture = typename FUTURES::SetVcselPulsePeriodFuture<TYPE>;
			//FIXME check period!
			return this->template sync_write<SetVcselPulsePeriodFuture>(period);
		}

		bool get_signal_rate_limit(float& signal_rate)
		{
			using GetSignalRateLimitFuture = typename FUTURES::GetSignalRateLimitFuture;
			return this->template sync_read<GetSignalRateLimitFuture, float>(signal_rate);
		}
		bool set_signal_rate_limit(float signal_rate)
		{
			using SetSignalRateLimitFuture = typename FUTURES::SetSignalRateLimitFuture;
			return this->template sync_write<SetSignalRateLimitFuture, float>(signal_rate);
		}

		bool get_reference_SPADs(SPADReference& spad_ref)
		{
			using GetReferenceSPADsFuture = 
				typename FUTURES::TReadRegisterFuture<regs::REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0, SPADReference>;
			return this->template sync_read<GetReferenceSPADsFuture, SPADReference>(spad_ref);
		}

		using SetReferenceSPADsFuture = 
			typename FUTURES::TWriteRegisterFuture<regs::REG_GLOBAL_CONFIG_SPAD_ENABLES_REF_0, SPADReference>;

		bool set_reference_SPADs(const SPADReference& spad_ref)
		{
			if (!await_same_future_group(
				internals::set_reference_spads::BUFFER, internals::set_reference_spads::BUFFER_SIZE))
				return false;
			return this->template sync_write<SetReferenceSPADsFuture, SPADReference>(spad_ref);
		}

		//TODO Makes this API private (or protected in AbstractDevice?)
		bool await_same_future_group(const uint8_t* buffer, uint8_t size)
		{
			typename FUTURES::I2CSameFutureGroup future{uint16_t(buffer), size};
			if (!future.start(*this)) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		//TODO Makes this API private (or protected in AbstractDevice?)
		bool await_device_strobe()
		{
			using READ_STROBE = typename FUTURES::TReadRegisterFuture<regs::REG_DEVICE_STROBE>;
			using WRITE_STROBE = typename FUTURES::TWriteRegisterFuture<regs::REG_DEVICE_STROBE>;
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

		bool get_SPAD_info(SPADInfo& info)
		{
			using READ_SPAD = typename FUTURES::TReadRegisterFuture<regs::REG_SPAD_INFO, SPADInfo>;
			using READ_STROBE = typename FUTURES::TReadRegisterFuture<regs::REG_DEVICE_STROBE>;
			using WRITE_STROBE = typename FUTURES::TWriteRegisterFuture<regs::REG_DEVICE_STROBE>;
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

		//TODO rewrite with sync_read
		bool get_sequence_steps_timeout(SequenceStepsTimeout& timeouts)
		{
			GetSequenceStepsTimeoutFuture future{};
			if (get_sequence_steps_timeout(future) != 0) return false;
			return future.get(timeouts);
		}

		//TODO rewrite with sync_read
		bool get_measurement_timing_budget(uint32_t& budget_us)
		{
			GetMeasurementTimingBudgetFuture future{};
			if (get_measurement_timing_budget(future) != 0) return false;
			return future.get(budget_us);
		}

		bool set_measurement_timing_budget(uint32_t budget_us)
		{
			using WRITE_BUDGET = typename
				FUTURES::TWriteRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t>;
			SequenceSteps steps;
			if (!get_sequence_steps(steps)) return false;
			SequenceStepsTimeout timeouts;
			if (!get_sequence_steps_timeout(timeouts)) return false;
			// Calculate budget
			uint16_t budget = devices::vl53l0x_futures::TimingBudgetUtilities::calculate_final_range_timeout_mclks(
				steps, timeouts, budget_us);
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

		bool force_io_2_8V()
		{
			using READ_VHV_CONFIG = typename FUTURES::TReadRegisterFuture<regs::REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV>;
			using WRITE_VHV_CONFIG = typename FUTURES::TWriteRegisterFuture<regs::REG_VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV>;
			uint8_t config = 0;
			if (!this->template sync_read<READ_VHV_CONFIG>(config)) return false;
			config |= 0x01;
			return this->template sync_write<WRITE_VHV_CONFIG>(config);
		}

		bool set_I2C_mode()
		{
			using WRITE_I2C_MODE = typename FUTURES::TWriteRegisterFuture<0x88>;
			return this->template sync_write<WRITE_I2C_MODE>(uint8_t(0x00));
		}

		bool read_stop_variable()
		{
			// Write prefix
			if (!await_same_future_group(
				internals::stop_variable::PRE_BUFFER, internals::stop_variable::PRE_BUFFER_SIZE))
				return false;

			// Read stop variable
			using READ_STOP_VAR = typename FUTURES::TReadRegisterFuture<0x91>;
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
			using WRITE_STOP_VAR = typename FUTURES::TWriteRegisterFuture<0x91>;
			if (!this->template sync_write<WRITE_STOP_VAR>(stop_variable_)) return false;

			// Write suffix
			return await_same_future_group(
				internals::stop_variable::POST_BUFFER, internals::stop_variable::POST_BUFFER_SIZE);
		}

		bool disable_signal_rate_limit_checks()
		{
			using READ_MSRC_CONFIG = typename FUTURES::TReadRegisterFuture<regs::REG_MSRC_CONFIG_CONTROL>;
			using WRITE_MSRC_CONFIG = typename FUTURES::TWriteRegisterFuture<regs::REG_MSRC_CONFIG_CONTROL>;
			uint8_t config = 0;
			if (!this->template sync_read<READ_MSRC_CONFIG>(config)) return false;
			config |= 0x12;
			return this->template sync_write<WRITE_MSRC_CONFIG>(config);
		}

		bool start_continuous_ranging(uint16_t period_ms = 0)
		{
			using READ_OSC_CAL = typename FUTURES::TReadRegisterFuture<regs::REG_OSC_CALIBRATE_VAL, uint16_t>;
			using WRITE_PERIOD = typename FUTURES::TWriteRegisterFuture<regs::REG_SYSTEM_INTERMEASUREMENT_PERIOD, uint32_t>;
			using WRITE_SYSRANGE = typename FUTURES::TWriteRegisterFuture<regs::REG_SYSRANGE_START>;
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

		//TODO API to read range async directly, to read range with preliminary wait interrupt (sync + async)
		// This API shall be used only after InterruptStatus != 0, Interrupt Status should be clear immediately after it
		using GetDirectRangeFuture = typename FUTURES::TReadRegisterFuture<regs::REG_RESULT_RANGE_STATUS + 10, uint16_t>;
		int get_direct_range(PROXY<GetDirectRangeFuture> future)
		{
			return this->async_read(future);
		}
		bool get_direct_range(uint16_t& range_mm)
		{
			return this->template sync_read<GetDirectRangeFuture>(range_mm);
		}

		bool stop_continuous_ranging()
		{
			return await_same_future_group(
				internals::stop_continuous_ranging::BUFFER, internals::stop_continuous_ranging::BUFFER_SIZE);
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

		//TODO rework sync only, private and private Future
		bool load_tuning_settings()
		{
			LoadTuningSettingsFuture future{};
			if (load_tuning_settings(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
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
			FUTURES::calculate_reference_SPADs(ref_spads.spad_refs(), info);
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

		bool perform_ref_calibration(uint8_t& debug1, uint8_t& debug2)
		{
			using WRITE_STEPS = typename FUTURES::TWriteRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG>;
			// 1. Read current sequence steps
			SequenceSteps steps;
			debug1 = 1;
			if (!get_sequence_steps(steps)) return false;
			// 2. Set steps for VHV calibration
			debug1 = 2;
			if (!this->template sync_write<WRITE_STEPS>(uint8_t(0x01))) return false;
			// 3. Perform single VHV calibration
			debug1 = 3;
			if (!perform_single_ref_calibration(SingleRefCalibrationTarget::VHV_CALIBRATION, debug2)) return false;
			// 4. Set steps for Phase calibration
			debug1 = 4;
			if (!this->template sync_write<WRITE_STEPS>(uint8_t(0x02))) return false;
			// 5. Perform single Phase calibration
			debug1 = 5;
			if (!perform_single_ref_calibration(SingleRefCalibrationTarget::PHASE_CALIBRATION, debug2)) return false;
			// 6. Restore sequence steps (NOTE: 0x00 is used as marker by the future to actually restore saved sequence)
			debug1 = 6;
			return set_sequence_steps(steps);
		}

	protected:
		static constexpr const uint16_t MAX_LOOP = 2000;

		bool perform_single_ref_calibration(SingleRefCalibrationTarget target, uint8_t& debug)
		{
			using WRITE_SYS_RANGE = typename FUTURES::TWriteRegisterFuture<regs::REG_SYSRANGE_START>;
			// 1. Write to register SYS RANGE
			debug = 1;
			if (!this->template sync_write<WRITE_SYS_RANGE>(uint8_t(target))) return false;
			// 2. Read interrupt status until interrupt occurs
			uint16_t loops = MAX_LOOP;
			while (loops--)
			{
				InterruptStatus status;
				debug = 2;
				if (!get_interrupt_status(status)) return false;
				if (status != 0)
				{
					// 3. Clear interrupt
					debug = 3;
					if (!clear_interrupt(0x01)) return false;
					// 4. Write to register SYS RANGE
					debug = 4;
					return this->template sync_write<WRITE_SYS_RANGE>(uint8_t(0x00));
				}
			}
			debug = 255;
			return false;
		}

	private:
		static constexpr const uint8_t DEFAULT_DEVICE_ADDRESS = 0x52;

		// Stop variable used across device invocations
		uint8_t stop_variable_ = 0;

		friend vl53l0x_futures::Futures<MANAGER>;
	};
}

#endif /* VL53L0X_H */
/// @endcond
