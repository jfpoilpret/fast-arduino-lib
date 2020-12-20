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
//TODO - define stream insertors for all enum types defined here
// OPEN POINTS:
// - calibration mode or only hard-coded calibration?

namespace devices::vl53l0x
{
	/// @cond notdocumented
	namespace internals = vl53l0x_internals;
	namespace regs = internals::registers;
	namespace actions = internals::action;
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
		SequenceStepsTimeout() = default;
		SequenceStepsTimeout(uint8_t pre_range_vcsel_period_pclks, uint8_t final_range_vcsel_period_pclks,
			uint8_t msrc_dss_tcc_mclks, uint16_t pre_range_mclks, uint16_t final_range_mclks)
			:	pre_range_vcsel_period_pclks_{pre_range_vcsel_period_pclks},
				final_range_vcsel_period_pclks_{final_range_vcsel_period_pclks},
				msrc_dss_tcc_mclks_{msrc_dss_tcc_mclks},
				pre_range_mclks_{pre_range_mclks},
				final_range_mclks_{final_range_mclks} {}

		uint16_t pre_range_vcsel_period_pclks() const
		{
			return (pre_range_vcsel_period_pclks_ + 1) << 1;
		}
		uint16_t final_range_vcsel_period_pclks() const
		{
			return (final_range_vcsel_period_pclks_ + 1) << 1;
		}
		uint16_t msrc_dss_tcc_mclks() const
		{
			return msrc_dss_tcc_mclks_ + 1;
		}
		uint16_t pre_range_mclks() const
		{
			return pre_range_mclks_;
		}
		uint16_t final_range_mclks() const
		{
			return final_range_mclks_;
		}

		//TODO following values are calcvulated from others
		// uint32_t msrc_dss_tcc_us,    pre_range_us,    final_range_us;
	private:
		uint8_t pre_range_vcsel_period_pclks_ = 0;
		uint8_t final_range_vcsel_period_pclks_ = 0;

		uint8_t msrc_dss_tcc_mclks_ = 0;
		uint16_t pre_range_mclks_ = 0;
		uint16_t final_range_mclks_ = 0;
	};

	//TODO the following helpers should be put somewhere else maybe (utilities.h?) or be hidden somehow
	template<typename T> T change_endianness(const T& value)
	{
		static_assert(sizeof(T) == 1, "T must be 1 byte length.");
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
		template<typename T = uint8_t> class AbstractReadRegisterFuture;
		template<typename T = uint8_t> class AbstractWriteRegisterFuture;
		template<uint8_t REGISTER, typename T = uint8_t> class ReadRegisterFuture;
		template<uint8_t REGISTER, typename T = uint8_t> class WriteRegisterFuture;
		class AbstractGetVcselPulsePeriodFuture;

		// Utility functions used every time a register gets read or written
		//TODO refactor into I2CDevice? or subclass I2CDevice with register-based functions?
		template<typename F> int async_read(PROXY<F> future)
		{
			return this->launch_commands(future, {this->write(), this->read(0, false, true)});
		}
		template<typename F, typename T = uint8_t> bool sync_read(T& result)
		{
			F future{};
			if (async_read<F>(future) != 0) return false;
			return future.get(result);
		}
		template<typename F> int async_write(PROXY<F> future)
		{
			return this->launch_commands(future, {this->write(0, false, true)});
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
		explicit VL53L0X(MANAGER& manager) : PARENT{manager, DEFAULT_DEVICE_ADDRESS, i2c::I2C_FAST, false} {}

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
		class GetVcselPulsePeriodFuture : public AbstractGetVcselPulsePeriodFuture
		{
			using PARENT = AbstractGetVcselPulsePeriodFuture;

		public:
			explicit GetVcselPulsePeriodFuture() : PARENT{uint8_t(TYPE)} {}
			bool get(uint8_t& result)
			{
				if (!PARENT::get(result)) return false;
				result = (result + 1) << 1;
				return true;
			}
			GetVcselPulsePeriodFuture(GetVcselPulsePeriodFuture&&) = default;
			GetVcselPulsePeriodFuture& operator=(GetVcselPulsePeriodFuture&&) = default;
		};
		template<VcselPeriodType TYPE>
		int get_vcsel_pulse_period(PROXY<GetVcselPulsePeriodFuture<TYPE>> future)
		{
			return async_read(future);
		}

		//TODO rework (Group of futures?)
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

		//TODO we could have the same result with some specialized FuturesGroup, no?
		class GetSequenceStepsTimeoutFuture : public FUTURE_STATUS_LISTENER
		{
		public:
			GetSequenceStepsTimeoutFuture() = default;

			bool get(SequenceStepsTimeout& timeouts)
			{
				if (await() != future::FutureStatus::READY)
					return false;
				timeouts = SequenceStepsTimeout{pre_range_vcsel_period_pclks_, final_range_vcsel_period_pclks_,
					msrc_dss_tcc_mclks_, pre_range_mclks_, final_range_mclks_};
				return true;
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
			bool next_future()
			{
				namespace data = internals::sequence_step_timeouts;

				int8_t action = next_byte();
				if (action == actions::MARKER)
				{
					const int8_t marker = next_byte();
					// Check marker and act accordingly
					switch (marker)
					{
						case data::MARKER_VCSEL_PERIOD_PRE_RANGE:
						readVcselPeriod_.get(pre_range_vcsel_period_pclks_);
						break;

						case data::MARKER_VCSEL_PERIOD_FINAL_RANGE:
						readVcselPeriod_.get(final_range_vcsel_period_pclks_);
						break;

						case data::MARKER_MSRC_CONFIG_TIMEOUT:
						readByte_.get(msrc_dss_tcc_mclks_);
						break;
						
						case data::MARKER_PRE_RANGE_TIMEOUT:
						readWord_.get(pre_range_mclks_);
						break;
						
						case data::MARKER_FINAL_RANGE_TIMEOUT:
						readWord_.get(final_range_mclks_);
						break;
						
						default:
						// error_ = errors::EILSEQ;
						error_ = -1;
						status_ = future::FutureStatus::ERROR;
						return false;
					}
					// Proceed with next action immediately
					action = next_byte();
				}

				if (action == actions::END)
				{
					// Future is finished
					if (status_ == future::FutureStatus::NOT_READY)
						status_ = future::FutureStatus::READY;
					return false;
				}

				if (action == actions::INCLUDE)
				{
					const int8_t include = next_byte();
					// Check included future
					switch (include)
					{
						case data::INCLUDE_VCSEL_PERIOD_PRE_RANGE:
						readVcselPeriod_.reset_(uint8_t(VcselPeriodType::PRE_RANGE));
						break;

						case data::INCLUDE_VCSEL_PERIOD_FINAL_RANGE:
						readVcselPeriod_.reset_(uint8_t(VcselPeriodType::FINAL_RANGE));
						break;

						default:
						// error_ = errors::EILSEQ;
						error_ = -3;
						status_ = future::FutureStatus::ERROR;
						return false;
					}
					return check_launch(
						device_->launch_commands(readVcselPeriod_, {device_->write(), device_->read()}));
				}

				// Must be a read future
				if (actions::is_read(action))
				{
					const uint8_t reg = next_byte();
					const uint8_t count = actions::count(action);
					const bool stop = actions::is_stop(action);
					switch (count)
					{
						case 1:
						readByte_.reset_(reg);
						return check_launch(
							device_->launch_commands(readByte_, {device_->write(), device_->read(0, false, stop)}));
						break;

						case 2:
						readWord_.reset_(reg);
						return check_launch(
							device_->launch_commands(readWord_, {device_->write(), device_->read(0, false, stop)}));
						break;

						default:
						error_ = -4;
						status_ = future::FutureStatus::ERROR;
						return false;
						// break;
					}
				}
				// error_ = errors::EILSEQ;
				error_ = -2;
				status_ = future::FutureStatus::ERROR;
				return false;
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
				// First check that current future was executed successfully
				if (status != future::FutureStatus::READY)
				{
					error_ = future.error();
					status_ = status;
					return;
				}
				next_future();
			}

			// The device that uses this future
			THIS* device_ = nullptr;

			// Information about futures to create and launch
			uint16_t address_ = uint16_t(internals::sequence_step_timeouts::BUFFER);

			// Placeholders for dynamic futures
			AbstractGetVcselPulsePeriodFuture readVcselPeriod_{0, this};
			AbstractReadRegisterFuture<uint8_t> readByte_{0, this};
			AbstractReadRegisterFuture<uint16_t> readWord_{0, this};

			// Values read from each future
			uint8_t pre_range_vcsel_period_pclks_;
			uint8_t final_range_vcsel_period_pclks_;
			uint8_t msrc_dss_tcc_mclks_;
			uint16_t pre_range_mclks_;
			uint16_t final_range_mclks_;

			// Global status for whole future group
			volatile future::FutureStatus status_ = future::FutureStatus::NOT_READY;
			volatile int error_ = 0;

			friend THIS;
		};

		int get_sequence_steps_timeout(GetSequenceStepsTimeoutFuture& future)
		{
			future.set_device(this);
			// Launch init
			if (!future.next_future())
				return future.error();
		}

		//TODO additional arguments for init? eg sequence steps, limits...
		class InitDataFuture : public FUTURE_STATUS_LISTENER
		{
			// Types of futures handled by this group
			template<uint8_t SIZE> using FUTURE_WRITE = FUTURE<void, containers::array<uint8_t, SIZE + 1>>;
			using FUTURE_READ = FUTURE<uint8_t, uint8_t>;
			
		public:
			InitDataFuture() = default;
			//TODO Move ctor/asgn?

			//TODO factor out all this common code! Where? In future.h?
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
			bool next_future()
			{
				namespace data = internals::init_data;

				bool change_value = false;
				uint8_t forced_value = 0x00;
				int8_t action = next_byte();
				if (action == actions::MARKER)
				{
					const int8_t marker = next_byte();
					// Check marker and act accordingly
					switch (marker)
					{
						case data::MARKER_VHV_CONFIG:
						read_.get(forced_value);
						forced_value |= data::VHV_CONFIG_PAD_SCL_SDA_EXTSUP_HV_SET_2V8;
						change_value = true;
						break;

						case data::MARKER_STOP_VARIABLE:
						read_.get(device_->stop_variable_);
						break;

						case data::MARKER_MSRC_CONFIG_CONTROL:
						read_.get(forced_value);
						forced_value |= data::MSRC_CONFIG_CONTROL_INIT;
						change_value = true;
						break;

						default:
						break;
					}
					// Proceed with next action immediately
					action = next_byte();
				}

				if (action == actions::END)
				{
					// Future is finished
					if (status_ == future::FutureStatus::NOT_READY)
						status_ = future::FutureStatus::READY;
					return false;
				}
				// Either read or write future
				const uint8_t reg = next_byte();
				const uint8_t count = actions::count(action);
				const bool stop = actions::is_stop(action);
				if (actions::is_read(action))
				{
					// Only one kind of read here (1 byte)
					read_.reset_(reg);
					return check_launch(
						device_->launch_commands(read_, {device_->write(), device_->read(0, false, stop)}));
				}
				else if (actions::is_write(action))
				{
					// Only two kinds of write here (1 or 2 bytes)
					if (count == 1)
					{
						uint8_t value = next_byte();
						if (change_value)
							value = forced_value;
						write1_.reset_({reg, value});
						return check_launch(
							device_->launch_commands(write1_, {device_->write(0, false, stop)}));
					}
					else
					{
						uint8_t val1 = next_byte();
						uint8_t val2 = next_byte();
						write2_.reset_({reg, val1, val2});
						return check_launch(
							device_->launch_commands(write2_, {device_->write(0, false, stop)}));
					}
				}
				return false;
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
				// First check that current future was executed successfully
				if (status != future::FutureStatus::READY)
				{
					error_ = future.error();
					status_ = status;
					return;
				}
				next_future();
			}

			// The device that uses this future
			THIS* device_ = nullptr;

			// Information about futures to create and launch
			uint16_t address_ = uint16_t(internals::init_data::BUFFER);

			// Placeholders for dynamic futures
			FUTURE_WRITE<1> write1_{{0}, this};
			FUTURE_WRITE<2> write2_{{0, 0}, this};
			FUTURE_READ read_{0, this};
			// Global status for whole future group
			volatile future::FutureStatus status_ = future::FutureStatus::NOT_READY;
			volatile int error_ = 0;

			friend THIS;
		};

		int init_data_first(InitDataFuture& future)
		{
			future.set_device(this);
			// Launch init
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

		bool get_sequence_steps_timeout(SequenceStepsTimeout& timeouts)
		{
			GetSequenceStepsTimeoutFuture future{};
			if (get_sequence_steps_timeout(future) != 0) return false;
			return future.get(timeouts);
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

		//TODO rename AbstractReadRegisterFuture (abstract is incorrect)
		//TODO Add transformer functor to template?
		// Future to read a register
		template<typename T>
		class AbstractReadRegisterFuture: public FUTURE<T, uint8_t>
		{
			using PARENT = FUTURE<T, uint8_t>;
		public:
			explicit AbstractReadRegisterFuture(uint8_t reg, FUTURE_STATUS_LISTENER* listener = nullptr)
				: PARENT{reg, listener} {}
			bool get(T& result)
			{
				if (!PARENT::get(result)) return false;
				result = change_endianness(result);
				return true;
			}
			AbstractReadRegisterFuture(AbstractReadRegisterFuture&&) = default;
			AbstractReadRegisterFuture& operator=(AbstractReadRegisterFuture&&) = default;
		};

		template<uint8_t REGISTER, typename T>
		class ReadRegisterFuture: public AbstractReadRegisterFuture<T>
		{
			using PARENT = AbstractReadRegisterFuture<T>;
		public:
			explicit ReadRegisterFuture() : PARENT{REGISTER} {}
			ReadRegisterFuture(ReadRegisterFuture&&) = default;
			ReadRegisterFuture& operator=(ReadRegisterFuture&&) = default;
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

		//TODO rename AbstractWriteRegisterFuture (abstract is incorrect)
		template<typename T>
		class AbstractWriteRegisterFuture: public FUTURE<void, WriteContent<T>>
		{
			using PARENT = FUTURE<void, WriteContent<T>>;
		public:
			explicit AbstractWriteRegisterFuture(
				uint8_t reg, const T& value, FUTURE_STATUS_LISTENER* listener = nullptr)
				:	PARENT{WriteContent{reg, value}, listener} {}
			AbstractWriteRegisterFuture(AbstractWriteRegisterFuture&&) = default;
			AbstractWriteRegisterFuture& operator=(AbstractWriteRegisterFuture&&) = default;
		};

		template<uint8_t REGISTER, typename T>
		class WriteRegisterFuture: public AbstractWriteRegisterFuture<T>
		{
			using PARENT = AbstractWriteRegisterFuture<T>;
		public:
			explicit WriteRegisterFuture(const T& value) : PARENT{REGISTER, value} {}
			WriteRegisterFuture(WriteRegisterFuture&&) = default;
			WriteRegisterFuture& operator=(WriteRegisterFuture&&) = default;
		};

		class AbstractGetVcselPulsePeriodFuture : public AbstractReadRegisterFuture<uint8_t>
		{
			using PARENT = AbstractReadRegisterFuture<uint8_t>;

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
