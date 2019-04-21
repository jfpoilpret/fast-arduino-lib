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

namespace devices
{
	enum MCP23017Port : uint8_t
	{
		PORT_A,
		PORT_B,
		PORT_AB
	};

	enum InterruptPolarity : uint8_t
	{
		ACTIVE_LOW = 0,
		ACTIVE_HIGH = 1
	};

	namespace mcp23017_traits
	{
		template<MCP23017Port PORT_> struct Port_trait
		{
			using TYPE = uint8_t;
			static constexpr const uint8_t REG_SHIFT = 0;
		};

		template<> struct Port_trait<MCP23017Port::PORT_A>
		{
			using TYPE = uint8_t;
			static constexpr const uint8_t REG_SHIFT = 0;
		};

		template<> struct Port_trait<MCP23017Port::PORT_B>
		{
			using TYPE = uint8_t;
			static constexpr const uint8_t REG_SHIFT = 1;
		};

		template<> struct Port_trait<MCP23017Port::PORT_AB>
		{
			using TYPE = uint16_t;
			static constexpr const uint8_t REG_SHIFT = 0;
		};
	}

	// This device is always used in mode BANK 0 (ie possibly 16 bits at a time)
	// In uint16_t mode, port A is the low byte, port B is the high byte
	template<i2c::I2CMode MODE_ = i2c::I2CMode::Fast> class MCP23017 : public i2c::I2CDevice<MODE_>
	{
	private:
		using BusCond = i2c::BusConditions;
		template<MCP23017Port P> using TRAIT = mcp23017_traits::Port_trait<P>;
		template<MCP23017Port P> using T = typename TRAIT<P>::TYPE;

	public:
		static constexpr const i2c::I2CMode MODE = MODE_;

		using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

		MCP23017(MANAGER& manager, uint8_t address)
			: i2c::I2CDevice<MODE>(manager), device_{uint8_t((BASE_ADDRESS | (address & 0x07)) << 1)}
		{}

		void begin(bool mirror_interrupts = false,
				   InterruptPolarity interrupt_polarity = InterruptPolarity::ACTIVE_HIGH)
		{
			// Initialize device
			write_register(IOCON, build_IOCON(mirror_interrupts, interrupt_polarity == InterruptPolarity::ACTIVE_HIGH));
		}

		// API to define for configuration
		//=================================

		// Configure all IO
		template<MCP23017Port P_> bool configure_gpio(T<P_> direction, T<P_> pullup = T<P_>{}, T<P_> polarity = T<P_>{})
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			return write_register(IODIR_A + SHIFT, direction) && write_register(IPOL_A + SHIFT, polarity)
				   && write_register(GPPU_A + SHIFT, pullup);
		}

		// Configure INTerrupts
		template<MCP23017Port P_>
		bool configure_interrupts(T<P_> int_pins, T<P_> ref = T<P_>{}, T<P_> compare_ref = T<P_>{})
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			return write_register(GPINTEN_A + SHIFT, int_pins) && write_register(DEFVAL_A + SHIFT, ref)
				   && write_register(INTCON_A + SHIFT, compare_ref);
		}

		// API to access IOs
		//===================

		template<MCP23017Port P_> bool values(T<P_> value)
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			return write_register(GPIO_A + SHIFT, value);
		}

		//TODO Do we need an Optional value here?
		template<MCP23017Port P_> T<P_> values()
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			T<P_> value;
			if (read_register(GPIO_A + SHIFT, value))
				return value;
			else
				return T<P_>{};
		}

		//TODO Do we need an Optional value here?
		template<MCP23017Port P_> T<P_> interrupt_flags()
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			T<P_> value;
			if (read_register(INTF_A + SHIFT, value))
				return value;
			else
				return T<P_>{};
		}

		//TODO Do we need an Optional value here?
		template<MCP23017Port P_> T<P_> captured_values()
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			T<P_> value;
			if (read_register(INTCAP_A + SHIFT, value))
				return value;
			else
				return T<P_>{};
		}

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

		template<typename T> bool write_register(uint8_t address, T value)
		{
			using namespace i2c::Status;
			return this->write(device_, address, BusCond::START_NO_STOP) == OK
				   && this->write(device_, value, BusCond::NO_START_STOP) == OK;
		}

		template<typename T> bool read_register(uint8_t address, T& value)
		{
			using namespace i2c::Status;
			return this->write(device_, address, BusCond::START_NO_STOP) == OK
				   && this->read(device_, value, BusCond::REPEAT_START_STOP) == OK;
		}

		const uint8_t device_;
	};
}

#endif /* MCP23017_H */
