//   Copyright 2016-2019 Jean-Francois Poilpret
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

#ifndef MCP23017_H
#define MCP23017_H

#include <math.h>
#include "../i2c_device.h"
#include "../utilities.h"

namespace devices
{
	enum InterruptPolarity : uint8_t
	{
		ACTIVE_LOW = 0,
		ACTIVE_HIGH = 1
	};

	//TODO do we want to support individual pin addressing? That maybe costly in use! Could be done as separate utility
	//TODO provide 3 API per action: one for each port, one for both ports simulatenously
	// This device is always used in mode BANK 0 (ie 16 bits at a time)
	// In uint16_t mode, port A is the low byte, port B is the high byte (TODO double check)
	template<i2c::I2CMode MODE_ = i2c::I2CMode::Fast> class MCP23017 : public i2c::I2CDevice<MODE_>
	{
	private:
		using BusCond = i2c::BusConditions;

	public:
		static constexpr const i2c::I2CMode MODE = MODE_;

		using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

		MCP23017(MANAGER& manager, uint8_t address, bool mirror_interrupts = false,
				 InterruptPolarity interrupt_polarity = InterruptPolarity::ACTIVE_HIGH)
			: i2c::I2CDevice<MODE>(manager), device_{uint8_t((BASE_ADDRESS | (address & 0x07)) << 1)}
		{
			// Initialize device
			if (this->write(device_, IOCON, BusCond::START_NO_STOP) == i2c::Status::OK)
				this->write(device_,
							build_IOCON(mirror_interrupts, interrupt_polarity == InterruptPolarity::ACTIVE_HIGH));
		}

		// API to define for configuration
		//=================================

		// Configure all IO with a call including all needed uint16 args
		bool configure_gpio(uint16_t direction, uint16_t pullup = 0, uint16_t polarity = 0)
		{
			using namespace i2c::Status;

			//TODO Check if we need to swap bytes on uint16_t or not
			return this->write(device_, IODIR_A, BusCond::START_NO_STOP) == OK
				   && this->write(device_, direction, BusCond::NO_START_NO_STOP) == OK
				   && this->write(device_, polarity, BusCond::NO_START_STOP) == OK
				   && this->write(device_, pullup, BusCond::START_STOP) == OK;
		}

		// Configure INTerrupts
		bool configure_interrupts(uint16_t int_pins, uint16_t compare_values = 0, uint16_t on_change = 0)
		{
			using namespace i2c::Status;

			//TODO Check if we need to swap bytes on uint16_t or not
			return this->write(device_, GPINTEN_A, BusCond::START_NO_STOP) == OK
				   && this->write(device_, int_pins, BusCond::NO_START_NO_STOP) == OK
				   && this->write(device_, compare_values, BusCond::NO_START_NO_STOP) == OK
				   && this->write(device_, on_change, BusCond::NO_START_STOP) == OK;
		}

		// API to access IOs
		//===================

		bool values(uint16_t output_values)
		{
			using namespace i2c::Status;

			//TODO Check if we need to swap bytes on uint16_t or not
			return this->write(device_, GPIO_A, BusCond::START_NO_STOP) == OK
				   && this->write(device_, output_values, BusCond::NO_START_STOP) == OK;
		}

		//TODO Do we need an Optional value here?
		uint16_t values()
		{
			using namespace i2c::Status;

			uint16_t gpio_values;
			if (this->write(device_, GPIO_A, BusCond::START_NO_STOP) == OK
				&& this->read(device_, gpio_values, BusCond::REPEAT_START_STOP) == OK)
				//TODO Check if we need to swap bytes on uint16_t or not
				return gpio_values;
			else
				return 0;
		}

		uint16_t interrupt_flags()
		{
			//TODO
		}

		uint16_t captured_values()
		{
			//TODO
		}

		//TODO Port-level API
		bool portA_values(uint8_t output_values) {}
		uint8_t portA_values() {}
		uint8_t portA_interrupt_flags() {}
		uint8_t portA_captured_values() {}

		bool portB_values(uint8_t output_values) {}
		uint8_t portB_values() {}
		uint8_t portB_interrupt_flags() {}
		uint8_t portB_captured_values() {}

	private:
		// Base address of the device (actual address can be in 0x20-0x27)
		static constexpr const uint8_t BASE_ADDRESS = 0x20;

		// All registers addresses (in BANK 0 mode only)
		static constexpr const uint8_t IODIR_A = 0x00;
		static constexpr const uint8_t IODIR_B = 0x01;
		static constexpr const uint8_t IPOL_A = 0x02;
		static constexpr const uint8_t IPOL_B = 0x03;

		static constexpr const uint8_t GPINTEN_A = 0x04;
		static constexpr const uint8_t GPINTEN_B = 0x05;
		static constexpr const uint8_t DEFVAL_A = 0x06;
		static constexpr const uint8_t DEFVAL_B = 0x07;
		static constexpr const uint8_t INTCON_A = 0x08;
		static constexpr const uint8_t INTCON_B = 0x09;

		static constexpr const uint8_t IOCON = 0x0A;

		static constexpr const uint8_t GPPU_A = 0x0C;
		static constexpr const uint8_t GPPU_B = 0x0D;

		static constexpr const uint8_t INTF_A = 0x0E;
		static constexpr const uint8_t INTF_B = 0x0F;
		static constexpr const uint8_t INTCAP_A = 0x10;
		static constexpr const uint8_t INTCAP_B = 0x11;

		static constexpr const uint8_t GPIO_A = 0x12;
		static constexpr const uint8_t GPIO_B = 0x13;
		static constexpr const uint8_t OLAT_A = 0x14;
		static constexpr const uint8_t OLAT_B = 0x15;

		// IOCON bits (not all are used in this implementation)
		static constexpr const uint8_t IOCON_BANK = _BV(7);
		static constexpr const uint8_t IOCON_MIRROR = _BV(6);
		static constexpr const uint8_t IOCON_SEQOP = _BV(5);
		static constexpr const uint8_t IOCON_DISSLW = _BV(4);
		static constexpr const uint8_t IOCON_HAEN = _BV(3);
		static constexpr const uint8_t IOCON_ODR = _BV(2);
		static constexpr const uint8_t IOCON_INTPOL = _BV(1);

		static constexpr uint8_t build_IOCON(bool mirror, bool int_polarity)
		{
			return (mirror ? IOCON_MIRROR : 0) | (int_polarity ? IOCON_INTPOL : 0);
		}

		const uint8_t device_;
	};
}

#endif /* MCP23017_H */
