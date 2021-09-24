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
#include "vl53l0x_internals.h"
#include "vl53l0x_registers.h"
#include "vl53l0x_types.h"

namespace devices::vl53l0x
{
	// Forward declaration
	template<typename MANAGER> class VL53L0X;
}

//TODO review all futures to add args checks when needed
namespace devices::vl53l0x_futures
{
	// Shortened aliases for various namespaces
	namespace internals = vl53l0x_internals;
	namespace regs = vl53l0x_registers;
	namespace actions = i2c::actions;

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

	// This fake class gathers all futures specific to VL53L0X device
	template<typename MANAGER> struct Futures
	{
		// Ensure MANAGER is an accepted I2C Manager type
		static_assert(i2c::I2CManager_trait<MANAGER>::IS_I2CMANAGER, "MANAGER_ must be a valid I2C Manager type");

		using DEVICE = vl53l0x::VL53L0X<MANAGER>;
		using ABSTRACT_FUTURE = typename MANAGER::ABSTRACT_FUTURE;
		template<typename OUT, typename IN> using FUTURE = typename MANAGER::template FUTURE<OUT, IN>;

		using FUTURE_OUTPUT_LISTENER = future::FutureOutputListener<ABSTRACT_FUTURE>;
		using FUTURE_STATUS_LISTENER = future::FutureStatusListener<ABSTRACT_FUTURE>;

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

		using GetSequenceStepsFuture = TReadRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, vl53l0x::SequenceSteps>;
		using SetSequenceStepsFuture = TWriteRegisterFuture<regs::REG_SYSTEM_SEQUENCE_CONFIG, vl53l0x::SequenceSteps>;

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
			TReadRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CONFIG_GPIO, vl53l0x::GPIOFunction> read_config_{};
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

			friend DEVICE;
			friend Futures;
		};

		class SetGPIOSettingsFuture : public I2CFuturesGroup
		{
			using PARENT = I2CFuturesGroup;
		public:
			SetGPIOSettingsFuture(const vl53l0x::GPIOSettings& settings)
				:	PARENT{futures_, NUM_FUTURES},
					write_config_{settings.function()},
					write_GPIO_active_high_{uint8_t(settings.high_polarity() ? 0x10 : 0x00)},
					write_low_threshold_{settings.low_threshold()},
					write_high_threshold_{settings.high_threshold()}
			{
				PARENT::init(futures_);
			}

		private:
			TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CONFIG_GPIO, vl53l0x::GPIOFunction> write_config_;
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

			friend DEVICE;
			friend Futures;
		};

		class ClearInterruptFuture : public TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CLEAR>
		{
			using PARENT = TWriteRegisterFuture<regs::REG_SYSTEM_INTERRUPT_CLEAR>;
		public:
			ClearInterruptFuture(uint8_t clear_mask) : PARENT{clear_mask} {}
		};
	};
}

#endif /* VL53L0X_FUTURES_H */
/// @endcond
