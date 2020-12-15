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
#include "vl53l0x_internals.h"

namespace devices
{
	/**
	 * Defines API for VL53L0X Time-of-Flight ranging sensor chip usage.
	 */
	namespace vl53l0x
	{
	}
}

//TODO - use PROGMEM (flash memory) for long sequences of bytes to be sent?
//TODO - implement low-level API step by step
//       - init_data
//       - static_init
//       - calibration?
//       - single ranging
//       - status
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
//
// OPEN POINTS:
// - calibration mode or only hard-coded calibration?

namespace devices::vl53l0x
{
	/// @cond notdocumented
	namespace internals = vl53l0x_internals;
	namespace regs = internals::registers;
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

	// enum class DeviceMode : uint8_t
	// {
	// 	SINGLE_RANGING				= 0,
	// 	CONTINUOUS_RANGING			= 1,
	// 	SINGLE_HISTOGRAM			= 2,
	// 	CONTINUOUS_TIMED_RANGING	= 3,
	// 	SINGLE_ALS					= 10,
	// 	GPIO_DRIVE					= 20,
	// 	GPIO_OSC					= 21
	// };

	enum class PowerMode : uint8_t
	{
		STANDBY = 0,
		IDLE = 1
	};

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

	// enum class CheckEnable : uint8_t
	// {
	// 	SIGMA_FINAL_RANGE          = 0,
	// 	SIGNAL_RATE_FINAL_RANGE    = 1,
	// 	SIGNAL_REF_CLIP            = 2,
	// 	RANGE_IGNORE_THRESHOLD     = 3,
	// 	SIGNAL_RATE_MSRC           = 4,
	// 	SIGNAL_RATE_PRE_RANGE      = 5
	// };

	//TODO the following helpers should be put somewhere else maybe (utilities.h?) or be hidden somehow
	template<typename T> T change_endianness(const T& value)
	{
		return value;
	}
	template<> uint16_t change_endianness(const uint16_t& value)
	{
		uint16_t temp = value;
		utils::swap_bytes(temp);
		return temp;
	}
	template<> int16_t change_endianness(const int16_t& value)
	{
		int16_t temp = value;
		utils::swap_bytes(temp);
		return temp;
	}
	template<> uint32_t change_endianness(const uint32_t& value)
	{
		uint32_t temp = value;
		utils::swap_bytes(temp);
		return temp;
	}
	template<> int32_t change_endianness(const int32_t& value)
	{
		int32_t temp = value;
		utils::swap_bytes(temp);
		return temp;
	}

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

		using FUTURE_OUTPUT_LISTENER = future::FutureOutputListener<ABSTRACT_FUTURE>;
		using FUTURE_STATUS_LISTENER = future::FutureStatusListener<ABSTRACT_FUTURE>;

		// Forward declarations needed by compiler
		template<uint8_t REGISTER, typename T = uint8_t> class ReadRegisterFuture;
		template<uint8_t REGISTER, typename T = uint8_t> class WriteRegisterFuture;

		// Utility functions used every time a register gets read or written
		//TODO refactor into I2CDevice? or subclass I2CDevice with register-based functions?
		template<typename F> int async_read(PROXY<F> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}
		template<typename F, typename T = uint8_t> bool sync_read(T& result)
		{
			F future{};
			if (async_read<F>(future) != 0) return false;
			return future.get(result);
		}
		template<typename F> int async_write(PROXY<F> future)
		{
			return this->launch_commands(future, {this->write()});
		}
		template<typename F, typename T = uint8_t> bool sync_write(const T& value)
		{
			F future{value};
			if (async_write<F>(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

	public:
		/**
		 * Create a new device driver for a VL53L0X chip.
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 */
		//TODO check if we can use false as last arg instead
		explicit VL53L0X(MANAGER& manager) : PARENT{manager, DEFAULT_DEVICE_ADDRESS, i2c::I2C_FAST, true} {}

		// Asynchronous API
		//==================
		using GetModelFuture = ReadRegisterFuture<regs::REG_IDENTIFICATION_MODEL_ID>;
		int get_model(PROXY<GetModelFuture> future)
		{
			return async_read(future);
		}

		using GetRevisionFuture = ReadRegisterFuture<regs::REG_IDENTIFICATION_REVISION_ID>;
		int get_revision(PROXY<GetRevisionFuture> future)
		{
			return async_read(future);
		}

		using GetPowerModeFuture = ReadRegisterFuture<regs::REG_POWER_MANAGEMENT, PowerMode>;
		int get_power_mode(PROXY<GetPowerModeFuture> future)
		{
			return async_read(future);
		}

		using GetRangeStatusFuture = ReadRegisterFuture<regs::REG_RESULT_RANGE_STATUS, DeviceStatus>;
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return async_read(future);
		}

		using GetSequenceStepsFuture = ReadRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int get_sequence_steps(PROXY<GetSequenceStepsFuture> future)
		{
			return async_read(future);
		}

		using SetSequenceStepsFuture = WriteRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int set_sequence_steps(PROXY<SetSequenceStepsFuture> future)
		{
			return async_write(future);
		}

		template<VcselPeriodType TYPE>
		class GetVcselPulsePeriodFuture : public ReadRegisterFuture<uint8_t(TYPE)>
		{
			using PARENT = ReadRegisterFuture<uint8_t(TYPE)>;

		public:
			explicit GetVcselPulsePeriodFuture() : PARENT{} {}
			bool get(uint8_t& result)
			{
				if (!PARENT::get(result)) return false;
				result = (result + 1) << 1;
				return true;
			}
			GetVcselPulsePeriodFuture(GetVcselPulsePeriodFuture<TYPE>&&) = default;
			GetVcselPulsePeriodFuture& operator=(GetVcselPulsePeriodFuture<TYPE>&&) = default;
		};
		template<VcselPeriodType TYPE>
		int get_vcsel_pulse_period(PROXY<GetVcselPulsePeriodFuture<TYPE>> future)
		{
			return async_read(future);
		}

		template<VcselPeriodType TYPE>
		class SetVcselPulsePeriodFuture : public WriteRegisterFuture<uint8_t(TYPE)>
		{
			using PARENT = WriteRegisterFuture<uint8_t(TYPE)>;
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
			return async_write(future);
		}

		class GetSignalRateLimitFuture : 
			public ReadRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>
		{
			using PARENT = ReadRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;

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
			return async_read(future);
		}

		class SetSignalRateLimitFuture : 
			public WriteRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>
		{
			using PARENT = WriteRegisterFuture<regs::REG_FINAL_RANGE_CONFIG_MIN_COUNT_RATE_RTN_LIMIT, uint16_t>;

		public:
			explicit SetSignalRateLimitFuture(float signal_rate) : PARENT{FixPoint9_7::convert(signal_rate)} {}
			SetSignalRateLimitFuture(SetSignalRateLimitFuture&&) = default;
			SetSignalRateLimitFuture& operator=(SetSignalRateLimitFuture&&) = default;
		};
		int set_signal_rate_limit(PROXY<SetSignalRateLimitFuture> future)
		{
			return async_write(future);
		}

		//TODO additional arguments for init? eg sequence steps, limits...
		class InitDataFuture : public FUTURE_STATUS_LISTENER
		{
			// Types of futures handled by this group
			template<uint8_t SIZE> using FUTURE_WRITE = FUTURE<void, containers::array<uint8_t, SIZE + 1>>;
			using FUTURE_READ = FUTURE<uint8_t, uint8_t>;
			
		public:
			InitDataFuture()
			{
				// Read list of futures settings from Flash
				flash::read_flash(
					uint16_t(internals::INIT_DATA_BUFFER_R_W), futures_, internals::INIT_DATA_BUFFER_RW_SIZE);
			}

			future::FutureStatus status() const
			{
				return status_;
			}

			future::FutureStatus await() const
			{
				while (true)
				{
					future::FutureStatus status = this->status();
					if (status != future::FutureStatus::NOT_READY)
						return status;
					time::yield();
				}
			}

			int error() const
			{
				future::FutureStatus status = await();
				switch (status)
				{
					case future::FutureStatus::READY:
					return 0;

					case future::FutureStatus::ERROR:
					return error_;

					default:
					// This should never happen
					return errors::EINVAL;
				}
			}

		private:
			void set_device(THIS* device)
			{
				device_ = device;
			}

			// Launch next future from the list stored in flash)
			bool next_future(bool change_value = false)
			{
				if (index_ >= internals::INIT_DATA_BUFFER_RW_SIZE)
				{
					if (status_ == future::FutureStatus::NOT_READY)
						status_ = future::FutureStatus::READY;
					return false;
				}
				const int8_t next = futures_[index_++];
				const uint8_t reg = next_byte();
				bool result;
				switch (next)
				{
					case -1:
					read_.reset_(reg);
					result = check_launch(device_->launch_commands(read_, {device_->write(), device_->read()}));
					break;
					
					case 1:
					{
						uint8_t value = next_byte();
						if (change_value)
							value = forced_value_;
						write1_.reset_({reg, value});
						result = check_launch(device_->launch_commands(write1_, {device_->write()}));
					}
					break;

					case 2:
					{
						uint8_t val1 = next_byte();
						uint8_t val2 = next_byte();
						write2_.reset_({reg, val1, val2});
						result = check_launch(device_->launch_commands(write2_, {device_->write()}));
					}
					break;

					default:
					result = false;
				}
				return result;
			}

			// Get the next byte, from the flash, to write to the device
			uint8_t next_byte()
			{
				uint8_t data = 0;
				return flash::read_flash(address_++, data);
			}

			// Check launch_commands() return and update own status if needed
			bool check_launch(int launch)
			{
				if (launch == 0) return true;
				error_ = launch;
				status_ = future::FutureStatus::ERROR;
				return false;
			}

			void on_status_change(UNUSED const ABSTRACT_FUTURE& future, future::FutureStatus status) final
			{
				//TODO
				// First check that current future was executed successfully
				if (status != future::FutureStatus::READY)
				{
					error_ = future.error();
					status_ = status;
					return;
				}
				// Check if next future needs change
				bool force = false;
				switch (index_)
				{
					case internals::INIT_DATA_FUTURE_VHV_CONFIG:
					read_.get(forced_value_);
					forced_value_ |= internals::VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_SET_2V8;
					force = true;
					break;

					case internals::INIT_DATA_FUTURE_STOP_VARIABLE:
					read_.get(device_->stop_variable_);
					break;

					case internals::INIT_DATA_FUTURE_MSRC_CONFIG_CONTROL:
					read_.get(forced_value_);
					forced_value_ |= internals::MSRC_CONFIG_CONTROL_INIT;
					force = true;
					break;

					default:
					break;
				}

				next_future(force);
			}

			// The device that uses this future
			THIS* device_ = nullptr;

			// Information about futures to create and launch
			uint16_t address_ = uint16_t(internals::INIT_DATA_BUFFER);
			int8_t futures_[internals::INIT_DATA_BUFFER_RW_SIZE];
			uint8_t index_ = 0;

			// Value to use on next future if need to be forced
			uint8_t forced_value_ = 0;

			// Placeholders for dynamic futures
			FUTURE_WRITE<1> write1_{{0}, this};
			FUTURE_WRITE<2> write2_{{0, 0}, this};
			FUTURE_READ read_{0, this};
			// Global status for whole future group
			future::FutureStatus status_ = future::FutureStatus::NOT_READY;
			int error_ = 0;

			friend THIS;
		};

		int init_data_first(InitDataFuture& future)
		{
			future.set_device(this);
			// Launch init (2V8 setting)
			if (!future.next_future())
				return future.error();
		}

		// //TODO group of futures for static init
		// class InitStaticGroup : public future::FuturesGroup, public FUTURE_STATUS_LISTENER
		// {
		// public:
		// 	//TODO

		// private:
		// 	//TODO on_change
		// };

		int init_static_second()
		{
			//TODO
			// 1. Get SPAD map
			// 2. Set reference SPADs
			// 3. Load tuning settings
			// 4. Set interrupt settings
			//...
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
			return sync_read<GetRevisionFuture>(revision);
		}
		bool get_model(uint8_t& model)
		{
			return sync_read<GetModelFuture>(model);
		}
		bool get_power_mode(PowerMode& power_mode)
		{
			return sync_read<GetPowerModeFuture>(power_mode);
		}
		bool get_range_status(DeviceStatus& range_status)
		{
			return sync_read<GetRangeStatusFuture>(range_status);
		}
		bool get_sequence_steps(SequenceSteps& sequence_steps)
		{
			return sync_read<GetSequenceStepsFuture>(sequence_steps);
		}
		bool set_sequence_steps(SequenceSteps sequence_steps)
		{
			return sync_write<SetSequenceStepsFuture>(sequence_steps);
		}
		template<VcselPeriodType TYPE>
		bool get_vcsel_pulse_period(uint8_t& period)
		{
			return sync_read<GetVcselPulsePeriodFuture<TYPE>>(period);
		}
		//TODO Much more complex process needed here: to be thought about further!
		template<VcselPeriodType TYPE>
		bool set_vcsel_pulse_period(uint8_t period)
		{
			//FIXME check period!
			return sync_write<SetVcselPulsePeriodFuture<TYPE>>(period);
		}

		bool get_signal_rate_limit(float& signal_rate)
		{
			return sync_read<GetSignalRateLimitFuture, float>(signal_rate);
		}
		bool set_signal_rate_limit(float signal_rate)
		{
			return sync_write<SetSignalRateLimitFuture, float>(signal_rate);
		}

		bool init_data_first()
		{
			InitDataFuture future{};
			if (init_data_first(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

	private:
		static constexpr const uint8_t DEFAULT_DEVICE_ADDRESS = 0x52;

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

		//TODO Add transformer functor to template?
		// Future to read a register
		template<uint8_t REGISTER, typename T>
		class ReadRegisterFuture: public FUTURE<T, uint8_t>
		{
			using PARENT = FUTURE<T, uint8_t>;
		public:
			explicit ReadRegisterFuture() : PARENT{REGISTER} {}
			bool get(T& result)
			{
				if (!PARENT::get(result)) return false;
				result = change_endianness(result);
				return true;
			}
			ReadRegisterFuture(ReadRegisterFuture<REGISTER, T>&&) = default;
			ReadRegisterFuture& operator=(ReadRegisterFuture<REGISTER, T>&&) = default;
		};

		//TODO Add transformer functor to template?
		//TODO Add checker functor to template?
		template<typename T> class WriteContent
		{
		public:
			WriteContent(uint8_t reg, const T& value) : register_{reg}, value_{change_endianness(value)} {}

		private:
			const uint8_t register_;
			const T value_; 
		};

		template<uint8_t REGISTER, typename T>
		class WriteRegisterFuture: public FUTURE<void, WriteContent<T>>
		{
			using PARENT = FUTURE<void, WriteContent<T>>;
		public:
			explicit WriteRegisterFuture(const T& value) : PARENT{WriteContent{REGISTER, value}} {}
			WriteRegisterFuture(WriteRegisterFuture<REGISTER, T>&&) = default;
			WriteRegisterFuture& operator=(WriteRegisterFuture<REGISTER, T>&&) = default;
		};

		//TODO utility functions

		// Stop variable used across device invocations
		uint8_t stop_variable_ = 0;
	};
}

#endif /* VL53L0X_H */
/// @endcond
