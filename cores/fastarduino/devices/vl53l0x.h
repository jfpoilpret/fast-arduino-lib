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
	enum class DeviceError : uint8_t
	{
		NONE                        = 0,
		VCSELCONTINUITYTESTFAILURE  = 1,
		VCSELWATCHDOGTESTFAILURE    = 2,
		NOVHVVALUEFOUND             = 3,
		MSRCNOTARGET                = 4,
		SNRCHECK                    = 5,
		RANGEPHASECHECK             = 6,
		SIGMATHRESHOLDCHECK         = 7,
		TCC                         = 8,
		PHASECONSISTENCY            = 9,
		MINCLIP                     = 10,
		RANGECOMPLETE               = 11,
		ALGOUNDERFLOW               = 12,
		ALGOOVERFLOW                = 13,
		RANGEIGNORETHRESHOLD        = 14,
		UNKNOWN                     = 15
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
		template<uint8_t REGISTER, typename T = uint8_t> class ReadByteRegisterFuture;
		template<uint8_t REGISTER, typename T = uint8_t> class WriteByteRegisterFuture;
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
		using GetModelFuture = ReadByteRegisterFuture<REG_IDENTIFICATION_MODEL_ID>;
		int get_model(PROXY<GetModelFuture> future)
		{
			return async_read(future);
		}

		using GetRevisionFuture = ReadByteRegisterFuture<REG_IDENTIFICATION_REVISION_ID>;
		int get_revision(PROXY<GetRevisionFuture> future)
		{
			return async_read(future);
		}

		using GetPowerModeFuture = ReadByteRegisterFuture<REG_POWER_MANAGEMENT, PowerMode>;
		int get_power_mode(PROXY<GetPowerModeFuture> future)
		{
			return async_read(future);
		}

		using GetRangeStatusFuture = ReadByteRegisterFuture<REG_RESULT_RANGE_STATUS, DeviceStatus>;
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return async_read(future);
		}

		using GetSequenceStepsFuture = ReadByteRegisterFuture<REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int get_sequence_steps(PROXY<GetSequenceStepsFuture> future)
		{
			return async_read(future);
		}

		using SetSequenceStepsFuture = WriteByteRegisterFuture<REG_SYSTEM_SEQUENCE_CONFIG, SequenceSteps>;
		int set_sequence_steps(PROXY<SetSequenceStepsFuture> future)
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

	private:
		static constexpr const uint8_t DEFAULT_DEVICE_ADDRESS = 0x52;

		// Future to read a byte register
		//TODO also use register type (uint8, uint16 or uint32) as template arg and specialize for endianness correction
		template<uint8_t REGISTER, typename T>
		class ReadByteRegisterFuture: public FUTURE<T, uint8_t>
		{
			static_assert(sizeof(T) == 1, "T must be exactly one byte long");
			using PARENT = FUTURE<T, uint8_t>;
		public:
			explicit ReadByteRegisterFuture() : PARENT{REGISTER} {}
			ReadByteRegisterFuture(ReadByteRegisterFuture<REGISTER, T>&&) = default;
			ReadByteRegisterFuture& operator=(ReadByteRegisterFuture<REGISTER, T>&&) = default;
		};

		template<typename T> class WriteContent
		{
		public:
			WriteContent(uint8_t reg, const T& value) : register_{reg}, value_{value} {}

		private:
			const uint8_t register_;
			const T value_; 
		};

		template<uint8_t REGISTER, typename T>
		class WriteByteRegisterFuture: public FUTURE<void, WriteContent<T>>
		{
			static_assert(sizeof(T) == 1, "T must be exactly one byte long");
			using PARENT = FUTURE<void, WriteContent<T>>;
		public:
			explicit WriteByteRegisterFuture(const T& value) : PARENT{WriteContent{REGISTER, value}} {}
			WriteByteRegisterFuture(WriteByteRegisterFuture<REGISTER, T>&&) = default;
			WriteByteRegisterFuture& operator=(WriteByteRegisterFuture<REGISTER, T>&&) = default;
		};

		//TODO utility functions

		//TODO stop variable (find better name?)
	};
}

#endif /* VL53L0X_H */
/// @endcond
