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

namespace devices
{
	/**
	 * Defines API for VL53L0X Time-of-Flight ranging sensor chip usage.
	 */
	namespace vl53l0x
	{
	}
}

//TODO - infer on grouping multiple futures
//TODO - infer on having a future value based on result from another future...
//			- use some kind of PlaceHolder updated by Future 1 and used by Future 2?
//			- use I2C events?
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
	// static utilities to support fiexed point 9/7 bits used by VL53L0X chip
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
			return value / (1 << DECIMAL_BITS);
		}

	private:
		static constexpr INTEGRAL_BITS = 9;
		static constexpr DECIMAL_BITS = 7;
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
		// static constexpr const uint8_t REG_PRE_RANGE_CONFIG_VCSEL_PERIOD = 0x50;
		PRE_RANGE = 0x50,
		// static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD = 0x70;
		FINAL_RANGE = 0x70
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
	 * This chip only supports both standard and fast I2C modes.
	 * 
	 * @tparam MANAGER one of FastArduino available I2C Manager
	 */
	template<typename MANAGER>
	class VL53L0X : public i2c::I2CDevice<MANAGER>
	{
	private:
		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

		// Forward declarations needed by compiler
		template<uint8_t REGISTER, typename T = uint8_t> class ReadRegisterFuture;
		template<uint8_t REGISTER, typename T = uint8_t> class WriteRegisterFuture;
		// Utility functions used every time a register gets read or written
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

		//TODO registers
		static constexpr const uint8_t REG_IDENTIFICATION_MODEL_ID = 0xC0;
		static constexpr const uint8_t REG_IDENTIFICATION_REVISION_ID = 0xC2;
		static constexpr const uint8_t REG_POWER_MANAGEMENT = 0x80;
		static constexpr const uint8_t REG_RESULT_RANGE_STATUS = 0x14;
		static constexpr const uint8_t REG_SYSTEM_SEQUENCE_CONFIG = 0x01;
		static constexpr const uint8_t REG_PRE_RANGE_CONFIG_VCSEL_PERIOD = 0x50;
		static constexpr const uint8_t REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD = 0x70;

	public:
		//TODO useful enums go here (Status, Ranging mode...)

		/**
		 * Create a new device driver for a VL53L0X chip.
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 */
		explicit VL53L0X(MANAGER& manager) : PARENT{manager, DEFAULT_DEVICE_ADDRESS, i2c::I2C_FAST, true} {}

		// Asynchronous API
		//==================
		using GetModelFuture = ReadRegisterFuture<REG_IDENTIFICATION_MODEL_ID>;
		int get_model(PROXY<GetModelFuture> future)
		{
			return async_read(future);
		}

		using GetRevisionFuture = ReadRegisterFuture<REG_IDENTIFICATION_REVISION_ID>;
		int get_revision(PROXY<GetRevisionFuture> future)
		{
			return async_read(future);
		}

		using GetPowerModeFuture = ReadRegisterFuture<REG_POWER_MANAGEMENT, PowerMode>;
		int get_power_mode(PROXY<GetPowerModeFuture> future)
		{
			return async_read(future);
		}

		using GetRangeStatusFuture = ReadRegisterFuture<REG_RESULT_RANGE_STATUS, DeviceStatus>;
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return async_read(future);
		}

		using GetSequenceStepsFuture = ReadRegisterFuture<REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int get_sequence_steps(PROXY<GetSequenceStepsFuture> future)
		{
			return async_read(future);
		}

		using SetSequenceStepsFuture = WriteRegisterFuture<REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
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

		// Synchronous API
		//=================
		bool set_address(uint8_t device_address)
		{
			//TODO
			// set_device(device_address);
		}

		bool init_first_once() {}
		bool init_second() {}
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

	private:
		static constexpr const uint8_t DEFAULT_DEVICE_ADDRESS = 0x52;

		//TODO Add transformer functor to template?
		// Future to read a register
		template<uint8_t REGISTER, typename T>
		class ReadRegisterFuture: public FUTURE<T, uint8_t>
		{
			static_assert(sizeof(T) == 1, "T must be exactly one byte long");
			using PARENT = FUTURE<T, uint8_t>;
		public:
			explicit ReadRegisterFuture() : PARENT{REGISTER} {}
			bool get(T& result)
			{
				if (!PARENT::get(result)) return false;
				change_endianness(result);
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
			static_assert(sizeof(T) == 1, "T must be exactly one byte long");
			using PARENT = FUTURE<void, WriteContent<T>>;
		public:
			explicit WriteRegisterFuture(const T& value) : PARENT{WriteContent{REGISTER, value}} {}
			WriteRegisterFuture(WriteRegisterFuture<REGISTER, T>&&) = default;
			WriteRegisterFuture& operator=(WriteRegisterFuture<REGISTER, T>&&) = default;
		};

		//TODO utility functions

		//TODO stop variable (find better name?)
	};
}

#endif /* VL53L0X_H */
/// @endcond
