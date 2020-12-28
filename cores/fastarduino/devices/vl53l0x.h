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
		using GetModelFuture = typename FUTURES::TReadRegisterFuture<regs::REG_IDENTIFICATION_MODEL_ID>;
		int get_model(PROXY<GetModelFuture> future)
		{
			return this->async_read(future);
		}

		using GetRevisionFuture = typename FUTURES::TReadRegisterFuture<regs::REG_IDENTIFICATION_REVISION_ID>;
		int get_revision(PROXY<GetRevisionFuture> future)
		{
			return this->async_read(future);
		}

		using GetPowerModeFuture = typename FUTURES::TReadRegisterFuture<regs::REG_POWER_MANAGEMENT, PowerMode>;
		int get_power_mode(PROXY<GetPowerModeFuture> future)
		{
			return this->async_read(future);
		}

		using GetRangeStatusFuture = 
			typename FUTURES::TReadRegisterFuture<regs::REG_RESULT_RANGE_STATUS, DeviceStatus>;
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return this->async_read(future);
		}

		using GetSequenceStepsFuture = 
			typename FUTURES::TReadRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int get_sequence_steps(PROXY<GetSequenceStepsFuture> future)
		{
			return this->async_read(future);
		}

		using SetSequenceStepsFuture = 
			typename FUTURES::TWriteRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int set_sequence_steps(PROXY<SetSequenceStepsFuture> future)
		{
			return this->async_write(future);
		}

		template<VcselPeriodType TYPE> 
		using GetVcselPulsePeriodFuture = typename FUTURES::GetVcselPulsePeriodFuture<TYPE>;
		template<VcselPeriodType TYPE>
		int get_vcsel_pulse_period(PROXY<GetVcselPulsePeriodFuture<TYPE>> future)
		{
			return this->async_read(future);
		}

		template<VcselPeriodType TYPE> 
		using SetVcselPulsePeriodFuture = typename FUTURES::SetVcselPulsePeriodFuture<TYPE>;
		template<VcselPeriodType TYPE>
		int set_vcsel_pulse_period(PROXY<SetVcselPulsePeriodFuture<TYPE>> future)
		{
			return this->async_write(future);
		}

		using GetSignalRateLimitFuture = typename FUTURES::GetSignalRateLimitFuture;
		int get_signal_rate_limit(PROXY<GetSignalRateLimitFuture> future)
		{
			return this->async_read(future);
		}

		using SetSignalRateLimitFuture = typename FUTURES::SetSignalRateLimitFuture;
		int set_signal_rate_limit(PROXY<SetSignalRateLimitFuture> future)
		{
			return this->async_write(future);
		}

		using GetSPADInfoFuture = typename FUTURES::GetSPADInfoFuture;
		int get_SPAD_info(GetSPADInfoFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		using GetSequenceStepsTimeoutFuture = typename FUTURES::GetSequenceStepsTimeoutFuture;
		int get_sequence_steps_timeout(GetSequenceStepsTimeoutFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		using GetMeasurementTimingBudgetFuture = typename FUTURES::GetMeasurementTimingBudgetFuture;
		int get_measurement_timing_budget(GetMeasurementTimingBudgetFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		using SetMeasurementTimingBudgetFuture = typename FUTURES::SetMeasurementTimingBudgetFuture;
		int set_measurement_timing_budget(SetMeasurementTimingBudgetFuture& future)
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

		using InitDataFuture = typename FUTURES::InitDataFuture;
		int init_data_first(InitDataFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		using LoadTuningSettingsFuture = typename FUTURES::LoadTuningSettingsFuture;
		int load_tuning_settings(LoadTuningSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		using InitStaticFuture = typename FUTURES::InitStaticFuture;
		int init_static_second(InitStaticFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

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
			return this->template sync_read<GetModelFuture>(model);
		}
		bool get_revision(uint8_t& revision)
		{
			return this->template sync_read<GetRevisionFuture>(revision);
		}
		bool get_power_mode(PowerMode& power_mode)
		{
			return this->template sync_read<GetPowerModeFuture>(power_mode);
		}
		bool get_range_status(DeviceStatus& range_status)
		{
			return this->template sync_read<GetRangeStatusFuture>(range_status);
		}
		bool get_sequence_steps(SequenceSteps& sequence_steps)
		{
			return this->template sync_read<GetSequenceStepsFuture>(sequence_steps);
		}
		bool set_sequence_steps(SequenceSteps sequence_steps)
		{
			return this->template sync_write<SetSequenceStepsFuture>(sequence_steps);
		}
		template<VcselPeriodType TYPE>
		bool get_vcsel_pulse_period(uint8_t& period)
		{
			return this->template sync_read<GetVcselPulsePeriodFuture<TYPE>>(period);
		}
		//TODO Much more complex process needed here: to be thought about further!
		template<VcselPeriodType TYPE>
		bool set_vcsel_pulse_period(uint8_t period)
		{
			//FIXME check period!
			return this->template sync_write<SetVcselPulsePeriodFuture<TYPE>>(period);
		}

		bool get_signal_rate_limit(float& signal_rate)
		{
			return this->template sync_read<GetSignalRateLimitFuture, float>(signal_rate);
		}
		bool set_signal_rate_limit(float signal_rate)
		{
			return this->template sync_write<SetSignalRateLimitFuture, float>(signal_rate);
		}

		bool get_SPAD_info(SPADInfo& info)
		{
			GetSPADInfoFuture future{};
			if (get_SPAD_info(future) != 0) return false;
			return future.get(info);
		}

		bool get_sequence_steps_timeout(SequenceStepsTimeout& timeouts)
		{
			GetSequenceStepsTimeoutFuture future{};
			if (get_sequence_steps_timeout(future) != 0) return false;
			return future.get(timeouts);
		}

		bool get_measurement_timing_budget(uint32_t& budget_us)
		{
			GetMeasurementTimingBudgetFuture future{};
			if (get_measurement_timing_budget(future) != 0) return false;
			return future.get(budget_us);
		}

		bool set_measurement_timing_budget(uint32_t budget_us)
		{
			SetMeasurementTimingBudgetFuture future{budget_us};
			if (set_measurement_timing_budget(future) == 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		bool get_GPIO_settings(GPIOSettings& settings)
		{
			GetGPIOSettingsFuture future{};
			if (get_GPIO_settings(future) != 0) return false;
			return future.get(settings);
		}

		bool set_GPIO_settings(const GPIOSettings& settings)
		{
			SetGPIOSettingsFuture future{};
			if (set_GPIO_settings(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		bool get_interrupt_status(InterruptStatus& status)
		{
			return this->template sync_read<GetInterruptStatusFuture>(status);
		}

		bool clear_interrupt()
		{
			return this->template sync_write<ClearInterruptFuture>();
		}

		bool init_data_first()
		{
			InitDataFuture future{};
			if (init_data_first(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		bool load_tuning_settings()
		{
			LoadTuningSettingsFuture future{};
			if (load_tuning_settings(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		bool init_static_second()
		{
			InitStaticFuture future{};
			if (init_static_second(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
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
