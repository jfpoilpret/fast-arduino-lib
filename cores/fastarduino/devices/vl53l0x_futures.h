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

/// @cond notdocumented

#ifndef VL53L0X_FUTURES_H
#define VL53L0X_FUTURES_H

#include "../array.h"
#include "../flash.h"
#include "../i2c.h"
#include "../future.h"
#include "../utilities.h"
#include "../i2c_handler.h"
#include "../i2c_device.h"
#include "../i2c_device_utilities.h"
#include "vl53l0x_registers.h"
#include "vl53l0x_types.h"

namespace devices::vl53l0x
{
	// Forward declaration
	template<typename MANAGER> class VL53L0X;
}

//TODO possibly remove this header (almost empty now) and add it directly to main vl53l0x header?
namespace devices::vl53l0x_futures
{
	// Shortened aliases for various namespaces
	using Register = devices::vl53l0x::Register;

	// This fake class gathers all futures specific to VL53L0X device
	template<typename MANAGER> struct Futures
	{
		// Ensure MANAGER is an accepted I2C Manager type
		static_assert(i2c::I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER_ must be a valid I2C Manager type");

		using DEVICE = vl53l0x::VL53L0X<MANAGER>;
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;

		template<Register REGISTER, typename T = uint8_t>
		using TReadRegisterFuture = i2c::TReadRegisterFuture<MANAGER, uint8_t(REGISTER), T, true>;
		template<Register REGISTER, typename T = uint8_t>
		using TWriteRegisterFuture = i2c::TWriteRegisterFuture<MANAGER, uint8_t(REGISTER), T, true>;

		using I2CFuturesGroup = i2c::I2CFuturesGroup<MANAGER>;
		using I2CSameFutureGroup = i2c::I2CSameFutureGroup<MANAGER>;

		class GetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			GetGPIOSettingsFuture() : PARENT{futures_, NUM_FUTURES}
			{
				PARENT::init(futures_);
			}

			bool get(vl53l0x::GPIOSettings& settings)
			{
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
				settings = vl53l0x::GPIOSettings{function, bool(active_high & 0x10), low_threshold, high_threshold};
				return true;
			}

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

			friend DEVICE;
			friend Futures;
		};

		//TODO shall we always clear interrupt (0) at the end of GPIO settings?
		class SetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			SetGPIOSettingsFuture(const vl53l0x::GPIOSettings& settings)
				:	PARENT{futures_, NUM_FUTURES},
					write_config_{settings.function()},
					// The following hard-coded values look OK but this is not how it should be done!
					//TODO GPIO_HV_MUX_ACTIVE_HIGH should first be read and then bit 4 clear or set
					write_GPIO_active_high_{uint8_t(settings.high_polarity() ? 0x11 : 0x01)},
					// Threshold values must be divided by 2, but nobody knows why
					write_low_threshold_{settings.low_threshold() / 2},
					write_high_threshold_{settings.high_threshold() / 2}
			{
				PARENT::init(futures_);
			}

		private:
			TWriteRegisterFuture<Register::SYSTEM_INTERRUPT_CONFIG_GPIO, vl53l0x::GPIOFunction> write_config_;
			TWriteRegisterFuture<Register::GPIO_HV_MUX_ACTIVE_HIGH> write_GPIO_active_high_;
			TWriteRegisterFuture<Register::SYSTEM_THRESH_LOW, uint16_t> write_low_threshold_;
			TWriteRegisterFuture<Register::SYSTEM_THRESH_HIGH, uint16_t> write_high_threshold_;

			static constexpr uint8_t NUM_FUTURES = 4;
			ABSTRACT_FUTURE* futures_[NUM_FUTURES] =
			{
				&write_config_,
				&write_GPIO_active_high_,
				&write_low_threshold_,
				&write_high_threshold_
			};

			friend DEVICE;
			friend Futures;
		};

		//TODO could be replaced with a simple using?
		//TODO In original API the method is more complex
		class ClearInterruptFuture : public TWriteRegisterFuture<Register::SYSTEM_INTERRUPT_CLEAR>
		{
			using PARENT = TWriteRegisterFuture<Register::SYSTEM_INTERRUPT_CLEAR>;
		public:
			ClearInterruptFuture(uint8_t clear_mask) : PARENT{clear_mask} {}
		};
	};
}

#endif /* VL53L0X_FUTURES_H */
/// @endcond
