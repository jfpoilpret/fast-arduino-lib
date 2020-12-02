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
	//TODO use this type in API instead of uint8_t!
	enum class PowerMode : uint8_t
	{
		POWERDOWN = 0,
		WAIT_STATICINIT = 1,
		STANDBY = 2,
		IDLE = 3,
		RUNNING = 4,
		UNKNOWN = 98,
		ERROR = 99
	};

	//TODO use type for RangeStatus (2 parts: error status and data_ready)
	// see VL53L0X_DeviceError in vl53l0x_device.h
	// see VL53L0X_GetDeviceErrorStatus and VL53L0X_GetMeasurementDataReady in vl53l0x_api.h

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
		class ReadByteRegisterFuture;

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
		class GetModelFuture : public ReadByteRegisterFuture
		{
		public:
			/// @cond notdocumented
			explicit GetModelFuture() : ReadByteRegisterFuture{REG_IDENTIFICATION_MODEL_ID} {}
			GetModelFuture(GetModelFuture&&) = default;
			GetModelFuture& operator=(GetModelFuture&&) = default;
			/// @endcond
		};
		int get_model(PROXY<GetModelFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		class GetRevisionFuture : public ReadByteRegisterFuture
		{
		public:
			/// @cond notdocumented
			explicit GetRevisionFuture() : ReadByteRegisterFuture{REG_IDENTIFICATION_REVISION_ID} {}
			GetRevisionFuture(GetRevisionFuture&&) = default;
			GetRevisionFuture& operator=(GetRevisionFuture&&) = default;
			/// @endcond
		};
		int get_revision(PROXY<GetRevisionFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		class GetPowerModeFuture : public ReadByteRegisterFuture
		{
		public:
			/// @cond notdocumented
			explicit GetPowerModeFuture() : ReadByteRegisterFuture{REG_POWER_MANAGEMENT} {}
			GetPowerModeFuture(GetPowerModeFuture&&) = default;
			GetPowerModeFuture& operator=(GetPowerModeFuture&&) = default;
			/// @endcond
		};
		int get_power_mode(PROXY<GetPowerModeFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
		}

		class GetRangeStatusFuture : public ReadByteRegisterFuture
		{
		public:
			/// @cond notdocumented
			explicit GetRangeStatusFuture() : ReadByteRegisterFuture{REG_RESULT_RANGE_STATUS} {}
			GetRangeStatusFuture(GetRangeStatusFuture&&) = default;
			GetRangeStatusFuture& operator=(GetRangeStatusFuture&&) = default;
			/// @endcond
		};
		int get_range_status(PROXY<GetRangeStatusFuture> future)
		{
			return this->launch_commands(future, {this->write(), this->read()});
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
			GetRevisionFuture future{};
			if (get_revision(PARENT::make_proxy(future)) != 0) return false;
			return future.get(revision);
		}
		bool get_model(uint8_t& model)
		{
			GetModelFuture future{};
			if (get_model(PARENT::make_proxy(future)) != 0) return false;
			return future.get(model);
		}
		bool get_power_mode(uint8_t& power_mode)
		{
			GetPowerModeFuture future{};
			if (get_power_mode(PARENT::make_proxy(future)) != 0) return false;
			return future.get(power_mode);
		}
		bool get_range_status(uint8_t& range_status)
		{
			GetRangeStatusFuture future{};
			if (get_range_status(PARENT::make_proxy(future)) != 0) return false;
			return future.get(range_status);
		}

	private:
		static constexpr const uint8_t DEFAULT_DEVICE_ADDRESS = 0x52;

		//TODO registers
		static constexpr const uint8_t REG_IDENTIFICATION_MODEL_ID = 0xC0;
		static constexpr const uint8_t REG_IDENTIFICATION_REVISION_ID = 0xC2;
		static constexpr const uint8_t REG_POWER_MANAGEMENT = 0x80;
		static constexpr const uint8_t REG_RESULT_RANGE_STATUS = 0x14;

		// Future to read a byte register
		//TODO better as a template (would ease definition of specialized futures)
		//TODO also use register type (uint8, uint16 or uint32) as template arg and specialize for endianness correction
		//TODO maybe also define template functions (not sure using would work here?)
		class ReadByteRegisterFuture: public FUTURE<uint8_t, uint8_t>
		{
			using PARENT = FUTURE<uint8_t, uint8_t>;
		public:
			explicit ReadByteRegisterFuture(const uint8_t reg) : PARENT{reg} {}
			ReadByteRegisterFuture(ReadByteRegisterFuture&&) = default;
			ReadByteRegisterFuture& operator=(ReadByteRegisterFuture&&) = default;
		};

		//TODO utility functions

		//TODO stop variable (find better name?)
	};
}

#endif /* VL53L0X_H */
/// @endcond
