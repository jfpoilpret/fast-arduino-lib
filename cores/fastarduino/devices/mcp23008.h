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

/// @cond api

/**
 * @file
 * API to handle the MCP23008 chip (16-Bit I/O Expander with I2C interface).
 */
#ifndef MCP23008_H
#define MCP23008_H

// #include <math.h>
#include "mcp230xx.h"
#include "../i2c_device.h"

/**
 * Defines all API for all external devices supported by FastArduino.
 * Most devices support API define their own namespace within `devices` namespace.
 */
namespace devices
{
}

/**
 * Defines the API for MCP23008/MCP23017 chips support.
 */
namespace devices::mcp230xx
{
	/**
	 * I2C device driver for Microchip MCP23008 support.
	 * The MCP23008 chip is a 8-Bit I/O Expander with I2C interface.
	 * 
	 * @tparam MODE_ the I2C mode to use; MCP23008 supports both `i2c::I2CMode::STANDARD`
	 * and `i2c::I2CMode::FAST`
	 * 
	 * @sa devices::mcp23017::MCP23017
	 */
	template<i2c::I2CMode MODE_ = i2c::I2CMode::FAST> class MCP23008 : public i2c::I2CDevice<MODE_>
	{
	private:
		using BUSCOND = i2c::BusConditions;

	public:
		/** The I2C mode (speed) used by this instance. */
		static constexpr const i2c::I2CMode MODE = MODE_;

		/** The type of `i2c::I2CManager` that must be used to handle this device.  */
		using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

		/**
		 * Create a new device driver for an MCP23008 chip. The @p address must match
		 * the actual address set for that chip (through pins A0, A1, A3).
		 * 
		 * @param manager reference to a suitable i2c::I2CManager for this device
		 * @param address the address part (0-7) set by A0-3 pins of the chip
		 */
		MCP23008(MANAGER& manager, uint8_t address)
			: i2c::I2CDevice<MODE>(manager), device_{uint8_t(uint8_t(BASE_ADDRESS | uint8_t(address & 0x07)) << 1)}
		{}

		/**
		 * Initialize the chip before operation.
		 * 
		 * @param interrupt_polarity the level triggerred on INTA or INTB pin when 
		 * an interrupt occurs
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool begin(InterruptPolarity interrupt_polarity = InterruptPolarity::ACTIVE_HIGH)
		{
			return write_register(IOCON, build_IOCON(interrupt_polarity == InterruptPolarity::ACTIVE_HIGH));
		}

		/**
		 * Configure GPIO on the port of this MCP23008 chip.
		 * 
		 * @param direction each bit sets the direction of one pin of the selected
		 * port; `1` means **I**nput, `0` means **O**utput.
		 * @param pullup each bit (only for input pins) sets if a pullup resistor
		 * shall be internally connected to the pin; if `1`, a pullup is added,
		 * if `0`, no pullup is added.
		 * @param polarity each bit (only for input pins) let you invert polarity of
		 * the matching input pin; if `1`, polarity is inverted, ie one the level
		 * on the input pin is `0`, then it is read as `1`, and conversely.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool configure_gpio(uint8_t direction, uint8_t pullup = 0, uint8_t polarity = 0)
		{
			return write_register(IODIR, direction) && write_register(IPOL, polarity)
				   && write_register(GPPU, pullup);
		}

		/**
		 * Configure interrupts on the port of this MCP23008 chip.
		 * 
		 * @param int_pins each bit sets if the matching pin shall generate interrupts
		 * @param ref contains the reference value for comparison with the actual 
		 * input pin; if input differs, then an interrupt will be triggered for that
		 * pin, provided that @p compare_ref for that bit is also `1`.
		 * @param compare_ref each bit indicates the condition for which the matching 
		 * input pin can generate interrupts; if `0`, an interrupt is generated every
		 * time the input pin changes level, if `1`, an interrupt is generated every
		 * time the input pin level changes to be diferent than the matching bit.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool configure_interrupts(uint8_t int_pins, uint8_t ref = 0, uint8_t compare_ref = 0)
		{
			return write_register(GPINTEN, int_pins) && write_register(DEFVAL, ref)
				   && write_register(INTCON, compare_ref);
		}

		/**
		 * Set output levels of output pins on the port of this MCP23008 chip.
		 * 
		 * @param value each bit indicates the new level of the matching output pin
		 * of the selected port
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool values(uint8_t value)
		{
			return write_register(GPIO, value);
		}

		/**
		 * Get levels of pins on the port of this MCP23008 chip.
		 * 
		 * @return a value where each bit indicates the current level of the 
		 * matching pin of the selected port
		 */
		uint8_t values()
		{
			uint8_t value;
			if (read_register(GPIO, value))
				return value;
			else
				return 0;
		}

		/**
		 * Get the pins that generated the latest interrupt on the port
		 * of the MCP23008 chip.
		 * 
		 * @return a value where each bit indicates if a pin generated the latest
		 * interrupt or not
		 */
		uint8_t interrupt_flags()
		{
			uint8_t value;
			if (read_register(INTF, value))
				return value;
			else
				return 0;
		}

		/**
		 * Get captured levels, at the time an interrupt was triggered, of pins
		 * on the port of this MCP23008 chip.
		 * This allows to know what generated an interrupt, even if input pins
		 * were modified afterwards.
		 * 
		 * @return a value where each bit indicates the level of the matching pin,
		 * captured at the interrupt time.
		 */
		uint8_t captured_values()
		{
			uint8_t value;
			if (read_register(INTCAP, value))
				return value;
			else
				return 0;
		}

	private:
		// Base address of the device (actual address can be in 0x20-0x27)
		static constexpr const uint8_t BASE_ADDRESS = 0x20;

		// All registers addresses
		static constexpr const uint8_t IODIR = 0x00;
		static constexpr const uint8_t IPOL = 0x01;

		static constexpr const uint8_t GPINTEN = 0x02;
		static constexpr const uint8_t DEFVAL = 0x03;
		static constexpr const uint8_t INTCON = 0x04;

		static constexpr const uint8_t IOCON = 0x05;

		static constexpr const uint8_t GPPU = 0x06;

		static constexpr const uint8_t INTF = 0x07;
		static constexpr const uint8_t INTCAP = 0x08;

		static constexpr const uint8_t GPIO = 0x09;
		static constexpr const uint8_t OLAT = 0x0A;

		// IOCON bits (not all are used in this implementation)
		static constexpr const uint8_t IOCON_SEQOP = bits::BV8(5);
		static constexpr const uint8_t IOCON_DISSLW = bits::BV8(4);
		static constexpr const uint8_t IOCON_HAEN = bits::BV8(3);
		static constexpr const uint8_t IOCON_ODR = bits::BV8(2);
		static constexpr const uint8_t IOCON_INTPOL = bits::BV8(1);

		static constexpr uint8_t build_IOCON(bool int_polarity)
		{
			return (int_polarity ? IOCON_INTPOL : 0);
		}

		bool write_register(uint8_t address, uint8_t value)
		{
			using i2c::Status::OK;
			return this->write(device_, address, BUSCOND::START_NO_STOP) == OK
				   && this->write(device_, value, BUSCOND::NO_START_STOP) == OK;
		}

		bool read_register(uint8_t address, uint8_t& value)
		{
			using i2c::Status::OK;
			return this->write(device_, address, BUSCOND::START_NO_STOP) == OK
				   && this->read(device_, value, BUSCOND::REPEAT_START_STOP) == OK;
		}

		const uint8_t device_;
	};
}

#endif /* MCP23008_H */
/// @endcond
