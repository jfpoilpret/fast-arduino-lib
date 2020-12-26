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
	namespace regs = internals::registers;
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
		using THIS = VL53L0X<MANAGER>;
		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;
		using ABSTRACT_FUTURE = typename PARENT::ABSTRACT_FUTURE;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

		using FUTURE_OUTPUT_LISTENER = typename PARENT::FUTURE_OUTPUT_LISTENER;
		using FUTURE_STATUS_LISTENER = typename PARENT::FUTURE_STATUS_LISTENER;

		template<typename T> 
		using ReadRegisterFuture = i2c::ReadRegisterFuture<MANAGER, T, true>;
		template<typename T>
		using WriteRegisterFuture = i2c::WriteRegisterFuture<MANAGER, T, true>;

		template<uint8_t REGISTER, typename T = uint8_t>
		using TReadRegisterFuture = i2c::TReadRegisterFuture<MANAGER, REGISTER, T, true>;
		template<uint8_t REGISTER, typename T = uint8_t>
		using TWriteRegisterFuture = i2c::TWriteRegisterFuture<MANAGER, REGISTER, T, true>;

		using AbstractI2CFuturesGroup = i2c::AbstractI2CFuturesGroup<MANAGER>;
		using I2CFuturesGroup = i2c::I2CFuturesGroup<MANAGER>;
		using I2CSameFutureGroup = i2c::I2CSameFutureGroup<MANAGER>;
		using ComplexI2CFuturesGroup = i2c::ComplexI2CFuturesGroup<MANAGER>;

		template<uint8_t SIZE> using FUTURE_WRITE = i2c::FUTURE_WRITE<MANAGER, SIZE>;
		template<uint8_t SIZE> using FUTURE_READ = i2c::FUTURE_READ<MANAGER, SIZE>;
		using FUTURE_READ1 = i2c::FUTURE_READ1<MANAGER>;

		// Forward declarations needed by compiler
		class AbstractGetVcselPulsePeriodFuture;
		class DeviceStrobeWaitFuture;

		static constexpr const uint8_t NUM_REF_SPADS = 48;
		static constexpr const uint8_t SPADS_PER_BYTE = 8;
		static constexpr const uint8_t NUM_REF_SPADS_BYTES = NUM_REF_SPADS / SPADS_PER_BYTE;

	public:
		/**
		 * Create a new device driver for a VL53L0X chip.
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 */
		explicit VL53L0X(MANAGER& manager) : PARENT{manager, DEFAULT_DEVICE_ADDRESS, i2c::I2C_FAST, false} {}

		// Asynchronous API
		//==================
		using GetModelFuture = TReadRegisterFuture<regs::REG_IDENTIFICATION_MODEL_ID>;
		int get_model(PROXY<GetModelFuture> future)
		{
			return this->async_read(future);
		}

		using GetRevisionFuture = TReadRegisterFuture<regs::REG_IDENTIFICATION_REVISION_ID>;
		int get_revision(PROXY<GetRevisionFuture> future)
		{
			return this->async_read(future);
		}

		using GetPowerModeFuture = TReadRegisterFuture<regs::REG_POWER_MANAGEMENT, PowerMode>;
		int get_power_mode(PROXY<GetPowerModeFuture> future)
		{
			return this->async_read(future);
		}

		using GetRangeStatusFuture = TReadRegisterFuture<regs::REG_RESULT_RANGE_STATUS, DeviceStatus>;
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return this->async_read(future);
		}

		using GetSequenceStepsFuture = TReadRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int get_sequence_steps(PROXY<GetSequenceStepsFuture> future)
		{
			return this->async_read(future);
		}

		using SetSequenceStepsFuture = TWriteRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int set_sequence_steps(PROXY<SetSequenceStepsFuture> future)
		{
			return this->async_write(future);
		}

		template<VcselPeriodType TYPE>
		class GetVcselPulsePeriodFuture : public AbstractGetVcselPulsePeriodFuture
		{
			using PARENT = AbstractGetVcselPulsePeriodFuture;

		public:
			explicit GetVcselPulsePeriodFuture(FUTURE_STATUS_LISTENER* status_listener = nullptr)
			:	PARENT{uint8_t(TYPE), status_listener} {}
			GetVcselPulsePeriodFuture(GetVcselPulsePeriodFuture&&) = default;
			GetVcselPulsePeriodFuture& operator=(GetVcselPulsePeriodFuture&&) = default;
		};
		template<VcselPeriodType TYPE>
		int get_vcsel_pulse_period(PROXY<GetVcselPulsePeriodFuture<TYPE>> future)
		{
			return this->async_read(future);
		}

		//TODO rework (Group of futures?)
		template<VcselPeriodType TYPE>
		class SetVcselPulsePeriodFuture : public TWriteRegisterFuture<uint8_t(TYPE)>
		{
			using PARENT = TWriteRegisterFuture<uint8_t(TYPE)>;
			static uint8_t encode_period(uint8_t period)
			{
				return (period >> 1) - 1;
			}

		public:
			explicit SetVcselPulsePeriodFuture(uint8_t period_pclks) : PARENT{encode_period(period_pclks)} {}
			SetVcselPulsePeriodFuture(SetVcselPulsePeriodFuture<TYPE>&&) = default;
			SetVcselPulsePeriodFuture& operator=(SetVcselPulsePeriodFuture<TYPE>&&) = default;
			//TODO check compliance of period_clicks
		};
		template<VcselPeriodType TYPE>
		int set_vcsel_pulse_period(PROXY<SetVcselPulsePeriodFuture<TYPE>> future)
		{
			return this->async_write(future);
		}

		class GetSignalRateLimitFuture : 
			public TReadRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>
		{
			using PARENT = TReadRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;

		public:
			explicit GetSignalRateLimitFuture() : PARENT{} {}
			GetSignalRateLimitFuture(GetSignalRateLimitFuture&&) = default;
			GetSignalRateLimitFuture& operator=(GetSignalRateLimitFuture&&) = default;

			bool get(float& result)
			{
				uint16_t temp = 0;
				if (!PARENT::get(temp)) return false;
				result = FixPoint9_7::convert(temp);
				return true;
			}
		};
		int get_signal_rate_limit(PROXY<GetSignalRateLimitFuture> future)
		{
			return this->async_read(future);
		}

		class SetSignalRateLimitFuture : 
			public TWriteRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>
		{
			using PARENT = TWriteRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;

		public:
			explicit SetSignalRateLimitFuture(float signal_rate) : PARENT{FixPoint9_7::convert(signal_rate)} {}
			SetSignalRateLimitFuture(SetSignalRateLimitFuture&&) = default;
			SetSignalRateLimitFuture& operator=(SetSignalRateLimitFuture&&) = default;
		};
		int set_signal_rate_limit(PROXY<SetSignalRateLimitFuture> future)
		{
			return this->async_write(future);
		}

		class GetSPADInfoFuture : public ComplexI2CFuturesGroup
		{
			using PARENT = ComplexI2CFuturesGroup;
			using ProcessAction = typename PARENT::ProcessAction;
			
		public:
			GetSPADInfoFuture() : PARENT{uint16_t(internals::spad_info::BUFFER)}
			{
				PARENT::init({&write1_, &write2_, &read_, &strobe_}, PARENT::NO_LIMIT);
			}
			//TODO Move ctor/asgn?

			bool get(SPADInfo& info)
			{
				if (PARENT::await() != future::FutureStatus::READY)
					return false;
				info = SPADInfo{info_};
				return true;
			}

		protected:
			bool start(THIS& device)
			{
				PARENT::set_device(device);
				device_ = &device;
				return next_future();
			}

		private:
			bool process_marker(uint8_t marker)
			{
				namespace data = internals::spad_info;
				// Check marker and act accordingly
				switch (marker)
				{
					case data::MARKER_OVERWRITE_REG_DEVICE_STROBE:
					read_.get(forced_value_);
					forced_value_ |= data::REG_DEVICE_STROBE_FORCED_VALUE;
					change_value_ = true;
					break;

					case data::MARKER_READ_SPAD_INFO:
					read_.get(info_);
					break;

					default:
					// Error: unexpected marker
					return PARENT::check_error(errors::EILSEQ);
				}
				return true;
			}

			bool process_include(uint8_t include)
			{
				if (include != internals::INCLUDE_DEVICE_STROBE_WAIT)
					// Error: unexpected include
					return PARENT::check_error(errors::EILSEQ);
				return strobe_.start(*device_);
			}
			
			bool process_read(UNUSED uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only one kind of read here (1 byte)
				read_.reset_(reg);
				return PARENT::check_error(
					PARENT::launch_commands(read_, {PARENT::write(), PARENT::read(0, false, stop)}));
			}

			bool process_write(uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only two kinds of write here (1 or 2 bytes)
				if (count == 1)
				{
					uint8_t value = this->next_byte();
					if (change_value_)
						value = forced_value_;
					write1_.reset_({reg, value});
					return PARENT::check_error(
						PARENT::launch_commands(write1_, {PARENT::write(0, false, stop)}));
				}
				else
				{
					uint8_t val1 = this->next_byte();
					uint8_t val2 = this->next_byte();
					write2_.reset_({reg, val1, val2});
					return PARENT::check_error(
						PARENT::launch_commands(write2_, {PARENT::write(0, false, stop)}));
				}
			}

			// Launch next future from the list stored in flash)
			bool next_future()
			{
				change_value_ = false;
				ProcessAction action;
				while ((action = this->process_action()) == ProcessAction::MARKER)
				{
					if (!process_marker(this->next_byte()))
						return false;
				}
				if (action == ProcessAction::DONE)
					return false;
				if (action == ProcessAction::INCLUDE)
					return process_include(this->next_byte());
				if (action == ProcessAction::READ)
					return process_read(this->count(), this->is_stop());
				if (action == ProcessAction::WRITE)
					return process_write(this->count(), this->is_stop());
				// It is impossible to fall here as all ProcessAction values have been tested before
				return false;
			}

			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				// First check that current future was executed successfully
				if (status == future::FutureStatus::READY)
					next_future();
			}

			// The device that uses this future
			THIS* device_ = nullptr;

			// SPAD info once read from device
			uint8_t info_ = 0;

			bool change_value_ = false;
			uint8_t forced_value_ = 0x00;

			// Placeholders for dynamic futures
			FUTURE_WRITE<1> write1_{{0}, this};
			FUTURE_WRITE<2> write2_{{0, 0}, this};
			FUTURE_READ1 read_{0, this};
			DeviceStrobeWaitFuture strobe_{this};

			friend THIS;
		};
		int get_SPAD_info(GetSPADInfoFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		class GetSequenceStepsTimeoutFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			GetSequenceStepsTimeoutFuture() : PARENT{futures_, NUM_FUTURES}
			{
				PARENT::init(futures_);
			}

			bool get(SequenceStepsTimeout& timeouts)
			{
				if (this->await() != future::FutureStatus::READY)
					return false;
				uint8_t pre_range_vcsel_period_pclks = 0;
				readVcselPeriodPreRange_.get(pre_range_vcsel_period_pclks);
				uint8_t final_range_vcsel_period_pclks = 0;
				readVcselPeriodFinalRange_.get(final_range_vcsel_period_pclks);
				uint8_t msrc_dss_tcc_mclks = 0;
				readMsrcTimeout_.get(msrc_dss_tcc_mclks);
				uint16_t pre_range_mclks = 0;
				readPreRangeTimeout_.get(pre_range_mclks);
				uint16_t final_range_mclks = 0;
				readFinalRangeTimeout_.get(final_range_mclks);
				timeouts = SequenceStepsTimeout{
					pre_range_vcsel_period_pclks, final_range_vcsel_period_pclks,
					msrc_dss_tcc_mclks, pre_range_mclks, final_range_mclks};
				return true;
			}

		private:
			// Actual futures  embedded in this group
			TReadRegisterFuture<uint8_t(VcselPeriodType::PRE_RANGE)> readVcselPeriodPreRange_{this};
			TReadRegisterFuture<uint8_t(VcselPeriodType::FINAL_RANGE)> readVcselPeriodFinalRange_{this};
			TReadRegisterFuture<regs::REG_MSRC_CONFIG_TIMEOUT_MACROP> readMsrcTimeout_{this};
			TReadRegisterFuture<regs::REG_PRE_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t> readPreRangeTimeout_{this};
			TReadRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_TIMEOUT_MACROP_HI, uint16_t> readFinalRangeTimeout_{this};
			static constexpr uint8_t NUM_FUTURES = 5;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&readVcselPeriodPreRange_,
				&readVcselPeriodFinalRange_,
				&readMsrcTimeout_,
				&readPreRangeTimeout_,
				&readFinalRangeTimeout_
			};

			friend THIS;
		};
		int get_sequence_steps_timeout(GetSequenceStepsTimeoutFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		class GetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			GetGPIOSettingsFuture() : PARENT{futures_, NUM_FUTURES}
			{
				PARENT::init(futures_);
			}

			bool get(GPIOSettings& settings)
			{
				if (this->await() != future::FutureStatus::READY)
					return false;
				GPIOFunction function = GPIOFunction::DISABLED;
				read_config_.get(function);
				uint8_t active_high = 0;
				read_GPIO_active_high_.get(active_high);
				uint16_t low_threshold = 0;
				read_low_threshold_.get(low_threshold);
				uint16_t high_threshold = 0;
				read_high_threshold_.get(high_threshold);
				settings = GPIOSettings{function, bool(active_high & 0x10), low_threshold, high_threshold};
				return true;
			}

		private:
			TReadRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CONFIG_GPIO, GPIOFunction> read_config_{};
			TReadRegisterFuture<regs::REG_GPIO_HV_MUX_ACTIVE_HIGH> read_GPIO_active_high_{};
			TReadRegisterFuture<regs::REG_SYSTEM_THRESH_LOW, uint16_t> read_low_threshold_{};
			TReadRegisterFuture<regs::REG_SYSTEM_THRESH_HIGH, uint16_t> read_high_threshold_{};

			static constexpr uint8_t NUM_FUTURES = 4;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&read_config_,
				&read_GPIO_active_high_,
				&read_low_threshold_,
				&read_high_threshold_
			};

			friend THIS;
		};
		int get_GPIO_settings(GetGPIOSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		class SetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			SetGPIOSettingsFuture(const GPIOSettings& settings)
				:	PARENT{futures_, NUM_FUTURES},
					write_config_{settings.function()},
					write_GPIO_active_high_{settings.high_polarity() ? 0x10 : 0x00},
					write_low_threshold_{settings.low_threshold()},
					write_high_threshold_{settings.high_threshold()}
			{
				PARENT::init(futures_);
			}

		private:
			TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CONFIG_GPIO, GPIOFunction> write_config_;
			TWriteRegisterFuture<regs::REG_GPIO_HV_MUX_ACTIVE_HIGH> write_GPIO_active_high_;
			TWriteRegisterFuture<regs::REG_SYSTEM_THRESH_LOW, uint16_t> write_low_threshold_;
			TWriteRegisterFuture<regs::REG_SYSTEM_THRESH_HIGH, uint16_t> write_high_threshold_;

			static constexpr uint8_t NUM_FUTURES = 4;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&write_config_,
				&write_GPIO_active_high_,
				&write_low_threshold_,
				&write_high_threshold_
			};

			friend THIS;
		};
		int set_GPIO_settings(SetGPIOSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		using GetInterruptStatusFuture = TReadRegisterFuture<regs::REG_RESULT_INTERRUPT_STATUS, InterruptStatus>;
		int get_interrupt_status(PROXY<GetInterruptStatusFuture> future)
		{
			return this->async_read(future);
		}

		//FIXME need to specify some value to clear interrupt! Not that simple?
		class ClearInterruptFuture : public TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CLEAR>
		{
			using PARENT = TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CLEAR>;
		public:
			ClearInterruptFuture() : PARENT{0x01} {}
		};
		int clear_interrupt(PROXY<ClearInterruptFuture> future)
		{
			return this->async_write(future);
		}

		//TODO additional arguments for init? eg sequence steps, limits...
		class InitDataFuture : public ComplexI2CFuturesGroup
		{
			using PARENT = ComplexI2CFuturesGroup;
			using ProcessAction = typename PARENT::ProcessAction;
			
		public:
			InitDataFuture() : PARENT{uint16_t(internals::init_data::BUFFER)}
			{
				PARENT::init({&write1_, &write2_, &read_}, PARENT::NO_LIMIT);
			}
			//TODO Move ctor/asgn?

		protected:
			bool start(THIS& device)
			{
				PARENT::set_device(device);
				device_ = &device;
				return next_future();
			}

		private:
			void process_marker(uint8_t marker)
			{
				namespace data = internals::init_data;
				// Check marker and act accordingly
				switch (marker)
				{
					case data::MARKER_VHV_CONFIG:
					read_.get(forced_value_);
					forced_value_ |= data::VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_SET_2V8;
					change_value_ = true;
					break;

					case data::MARKER_STOP_VARIABLE:
					read_.get(device_->stop_variable_);
					break;

					case data::MARKER_MSRC_CONFIG_CONTROL:
					read_.get(forced_value_);
					forced_value_ |= data::MSRC_CONFIG_CONTROL_INIT;
					change_value_ = true;
					break;

					default:
					//TODO ERROR
					break;
				}
			}

			bool process_read(UNUSED uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only one kind of read here (1 byte)
				read_.reset_(reg);
				return PARENT::check_error(
					PARENT::launch_commands(read_, {PARENT::write(), PARENT::read(0, false, stop)}));
			}

			bool process_write(uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only two kinds of write here (1 or 2 bytes)
				if (count == 1)
				{
					uint8_t value = this->next_byte();
					if (change_value_)
						value = forced_value_;
					write1_.reset_({reg, value});
					return PARENT::check_error(
						PARENT::launch_commands(write1_, {PARENT::write(0, false, stop)}));
				}
				else
				{
					uint8_t val1 = this->next_byte();
					uint8_t val2 = this->next_byte();
					write2_.reset_({reg, val1, val2});
					return PARENT::check_error(
						PARENT::launch_commands(write2_, {PARENT::write(0, false, stop)}));
				}
			}

			// Launch next future from the list stored in flash)
			bool next_future()
			{
				change_value_ = false;
				ProcessAction action;
				while ((action = this->process_action()) == ProcessAction::MARKER)
				{
					process_marker(this->next_byte());
				}
				if (action == ProcessAction::DONE)
					return false;
				if (action == ProcessAction::READ)
					return process_read(this->count(), this->is_stop());
				if (action == ProcessAction::WRITE)
					return process_write(this->count(), this->is_stop());
				// If we fall here , this is an error (no ProcessAction::INCLUDE in this group)
				//TODO error
				return false;
			}

			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				// First check that current future was executed successfully
				if (status == future::FutureStatus::READY)
					next_future();
			}

			// The device that uses this future
			THIS* device_ = nullptr;

			bool change_value_ = false;
			uint8_t forced_value_ = 0x00;

			// Placeholders for dynamic futures
			FUTURE_WRITE<1> write1_{{0}, this};
			FUTURE_WRITE<2> write2_{{0, 0}, this};
			FUTURE_READ1 read_{0, this};

			friend THIS;
		};

		int init_data_first(InitDataFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		//TODO
		class LoadTuningSettingsFuture : public I2CSameFutureGroup
		{
			using PARENT = I2CSameFutureGroup;
		public:
			LoadTuningSettingsFuture(FUTURE_STATUS_LISTENER* status_listener = nullptr)
				: PARENT{	uint16_t(internals::load_tuning_settings::BUFFER), 
							internals::load_tuning_settings::BUFFER_SIZE, status_listener} {}

			friend THIS;
		};

		int load_tuning_settings(LoadTuningSettingsFuture& future)
		{
			return (future.start(*this) ? 0 : future.error());
		}

		class InitStaticFuture : public ComplexI2CFuturesGroup
		{
			using PARENT = ComplexI2CFuturesGroup;
			using ProcessAction = typename PARENT::ProcessAction;
			
		public:
			InitStaticFuture() : PARENT{uint16_t(internals::init_static::BUFFER)}
			{
				PARENT::init({
					&write1_, &write2_, &write6_, 
					&read1_, &read6_, 
					&getSpadInfo_, &setGPIOSettings_, &loadTuningSettings_
				}, PARENT::NO_LIMIT);
			}
			//TODO Move ctor/asgn?

		protected:
			bool start(THIS& device)
			{
				PARENT::set_device(device);
				return next_future();
			}

			THIS& device()
			{
				return (THIS&) PARENT::device();
			}

		private:
			bool process_include(uint8_t include)
			{
				switch (include)
				{
					case internals::INCLUDE_GET_SPAD_INFO:
					return getSpadInfo_.start(device());

					case internals::INCLUDE_LOAD_TUNING_SETTINGS:
					return loadTuningSettings_.start(device());

					case internals::INCLUDE_SET_GPIO_SETTINGS:
					return setGPIOSettings_.start(device());

					default:
					// Error: unexpected include
					return PARENT::check_error(errors::EILSEQ);
				}
			}
			
			bool process_marker(uint8_t marker)
			{
				namespace data = internals::init_static;
				// Check marker and act accordingly
				if (marker == data::MARKER_GET_REFERENCE_SPADS)
				{
					// Get SPAD info
					SPADInfo info{};
					getSpadInfo_.get(info);
					// Get NVM ref SPADs
					read6_.get(ref_spads_);
					// Calculate reference spads
					calculate_reference_SPADs(ref_spads_.data(), info);
				}
				else
				{
					// Error: unexpected marker
					return PARENT::check_error(errors::EILSEQ);
				}
			}

			bool process_read(UNUSED uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				// Only one kind of read here (1 byte)
				if (count == 1)
				{
					read1_.reset_(reg);
					return PARENT::check_error(
						PARENT::launch_commands(read1_, {PARENT::write(), PARENT::read(0, false, stop)}));
				}
				else if (count == 6)
				{
					read6_.reset_(reg);
					return PARENT::check_error(
						PARENT::launch_commands(read6_, {PARENT::write(), PARENT::read(0, false, stop)}));
				}
				else
				{
					// Error: unexpected read
					return PARENT::check_error(errors::EILSEQ);
				}
			}

			bool process_write(uint8_t count, bool stop)
			{
				const uint8_t reg = this->next_byte();
				if (count == 1)
				{
					uint8_t value = this->next_byte();
					write1_.reset_({reg, value});
					return PARENT::check_error(
						PARENT::launch_commands(write1_, {PARENT::write(0, false, stop)}));
				}
				else if (count == 2)
				{
					uint8_t val1 = this->next_byte();
					uint8_t val2 = this->next_byte();
					write2_.reset_({reg, val1, val2});
					return PARENT::check_error(
						PARENT::launch_commands(write2_, {PARENT::write(0, false, stop)}));
				}
				else if (count == 6)
				{
					// Skip 6 bytes
					for (uint8_t i = 0; i < 6; ++i)
						this->next_byte();
					//FIXME review reset_() call here (will not work as is)
					typename FUTURE_WRITE<6>::IN input{reg};
					input.template set<NUM_REF_SPADS_BYTES>(1, ref_spads_.data());
					write6_.reset_(input);
					return PARENT::check_error(
						PARENT::launch_commands(write6_, {PARENT::write(0, false, stop)}));
				}
				else
				{
					// Error: unexpected read
					return PARENT::check_error(errors::EILSEQ);
				}
			}

			// Launch next future from the list stored in flash)
			bool next_future()
			{
				ProcessAction action;
				while ((action = this->process_action()) == ProcessAction::MARKER)
				{
					if (!process_marker(this->next_byte()))
						return false;
				}
				if (action == ProcessAction::DONE)
					return false;
				if (action == ProcessAction::INCLUDE)
					return process_include(this->next_byte());
				if (action == ProcessAction::READ)
					return process_read(this->count(), this->is_stop());
				if (action == ProcessAction::WRITE)
					return process_write(this->count(), this->is_stop());
				// It is impossible to fall here as all ProcessAction values have been tested before
				return false;
			}

			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				// First check that current future was executed successfully
				if (status == future::FutureStatus::READY)
					next_future();
			}

			containers::array<uint8_t, NUM_REF_SPADS_BYTES> ref_spads_{};

			// Futures included in this one
			GetSPADInfoFuture getSpadInfo_{};
			LoadTuningSettingsFuture loadTuningSettings_{};
			SetGPIOSettingsFuture setGPIOSettings_{GPIOSettings::sample_ready()};
			// Placeholders for dynamic futures
			FUTURE_WRITE<1> write1_{};
			FUTURE_WRITE<2> write2_{};
			FUTURE_WRITE<6> write6_{};
			FUTURE_READ1 read1_{};
			FUTURE_READ<6> read6_{};

			friend THIS;
		};

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

		bool get_revision(uint8_t& revision)
		{
			return this->template sync_read<GetRevisionFuture>(revision);
		}
		bool get_model(uint8_t& model)
		{
			return this->template sync_read<GetModelFuture>(model);
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

	private:
		static constexpr const uint8_t DEFAULT_DEVICE_ADDRESS = 0x52;
		static constexpr const uint8_t FIRST_APERTURE_SPAD = 12;

		static void calculate_reference_SPADs(uint8_t ref_spads[NUM_REF_SPADS_BYTES], SPADInfo info)
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

		void prepare_commands(uint16_t address, const int8_t* sizes, i2c::I2CLightCommand* commands, uint8_t count)
		{
			flash::read_flash(address, sizes, count);
			for (uint8_t i = 0; i < count; ++i)
			{
				const int8_t size = sizes[i];
				if (size < 0)
					commands[i] = this->read(-size);
				else
					commands[i] = this->write(size);
			}
		}

		//TODO future for device strobe wait
		class DeviceStrobeWaitFuture : public AbstractI2CFuturesGroup
		{
			using PARENT = AbstractI2CFuturesGroup;
		public:
			DeviceStrobeWaitFuture(FUTURE_STATUS_LISTENER* listener = nullptr) : PARENT{listener}
			{
				PARENT::init({&write_, &read_}, PARENT::NO_LIMIT);
			}

			bool start(THIS& device)
			{
				this->set_device(device);
				return write_strobe(0x00, StrobeStep::STEP_INIT_STROBE);
			}

		private:
			enum class StrobeStep : uint8_t
			{
				STEP_INIT_STROBE = 0,
				STEP_READ_STROBE = 1,
				STEP_EXIT_STROBE = 2
			};

			bool write_strobe(uint8_t value, StrobeStep next_step)
			{
				write_.reset_(value);
				step_ = next_step;
				return PARENT::check_error(PARENT::launch_commands(write_, {PARENT::write(0, false, true)}));
			}

			void read_strobe()
			{
				read_.reset_();
				PARENT::check_error(PARENT::launch_commands(read_, {PARENT::write(), PARENT::read()}));
			}

			bool check_strobe()
			{
				uint8_t strobe = 0;
				read_.get(strobe);
				return (strobe != 0);
			}

			void on_status_change(const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				PARENT::on_status_change(future, status);
				if (status != future::FutureStatus::READY) return;
				switch (step_)
				{
					case StrobeStep::STEP_READ_STROBE:
					if (check_strobe())
					{
						// Strobe is OK, go to last step
						write_strobe(0x01, StrobeStep::STEP_EXIT_STROBE);
						return;
					}
					if (++loop_ >= MAX_LOOP)
					{
						// Strobe is not OK after too many loops, abandon
						PARENT::check_error(errors::ETIME);
						return;
					}
					// Read strobe again
					// Intentional fallthrough
					
					case StrobeStep::STEP_INIT_STROBE:
					// Initial write strobe is finished, start loop reading probe
					step_ = StrobeStep::STEP_READ_STROBE;
					read_strobe();
					break;
					
					case StrobeStep::STEP_EXIT_STROBE:
					PARENT::set_future_finish_();
					break;
				}
			}

			static constexpr const uint16_t MAX_LOOP = 2000;

			StrobeStep step_ = StrobeStep::STEP_INIT_STROBE;
			uint16_t loop_ = 0;
			TWriteRegisterFuture<regs::REG_DEVICE_STROBE> write_{0x00, this};
			TReadRegisterFuture<regs::REG_DEVICE_STROBE> read_{this};
		};

		//TODO do we still need that intermediate class?
		class AbstractGetVcselPulsePeriodFuture : public ReadRegisterFuture<uint8_t>
		{
			using PARENT = ReadRegisterFuture<uint8_t>;

		public:
			explicit AbstractGetVcselPulsePeriodFuture(uint8_t reg, FUTURE_STATUS_LISTENER* listener = nullptr)
				: PARENT{reg, listener} {}
			bool get(uint8_t& result)
			{
				if (!PARENT::get(result)) return false;
				result = (result + 1) << 1;
				return true;
			}
			AbstractGetVcselPulsePeriodFuture(AbstractGetVcselPulsePeriodFuture&&) = default;
			AbstractGetVcselPulsePeriodFuture& operator=(AbstractGetVcselPulsePeriodFuture&&) = default;
		};

		// Stop variable used across device invocations
		uint8_t stop_variable_ = 0;
	};
}

#endif /* VL53L0X_H */
/// @endcond
