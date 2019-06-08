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
 * API to handle the MCP23017 chip (16-Bit I/O Expander with I2C interface).
 */
#ifndef MCP23017_H
#define MCP23017_H

#include <math.h>
#include "../i2c_device.h"

/**
 * Defines all API for all external devices supported by FastArduino.
 * Most devices support API define their own namespace within `devices` namespace.
 */
namespace devices
{
}

/**
 * Defines the API for MCP23017 chip support.
 */
namespace devices::mcp23017
{
	/**
	 * The port(s) to use in MCP23017 API. Most API are templates which argument
	 * selects which MCP23017 port the API shall apply to.
	 */
	enum class MCP23017Port : uint8_t
	{
		/** The A port of MCP23017. The API applies only on Port A. */
		PORT_A,
		/** The B port of MCP23017. The API applies only on Port B. */
		PORT_B,
		/**
		 * Both A and B ports of MCP23017. The API applies to both ports
		 * at the same time.
		 * In this configuration, the API takes `uint16_t` type to pass or
		 * return values for both ports at once.
		 * Each `uint16_t` argument is broken down as follows:
		 * - low byte maps to A port
		 * - high byte maps to B port
		 */
		PORT_AB
	};

	/**
	 * The polarity of the MCP23017 INTA and INTB pins.
	 */
	enum class InterruptPolarity : uint8_t
	{
		/**
		 * The INT pins shall be active low, ie they are high by default, and 
		 * changed to low when an interrupt is triggered.
		 */
		ACTIVE_LOW = 0,
		/**
		 * The INT pins shall be active high, ie they are low by default, and 
		 * changed to high when an interrupt is triggered.
		 */
		ACTIVE_HIGH = 1
	};

	/// @cond notdocumented
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
	/// @endcond

	/**
	 * I2C device driver for Microchip MCP23017 support.
	 * The MCP23017 chip is a 16-Bit I/O Expander with I2C interface.
	 * 
	 * @tparam MODE_ the I2C mode to use; MCP23017 supports both `i2c::I2CMode::Standard`
	 * and `i2c:I2CMode::Fast`
	 */
	template<i2c::I2CMode MODE_ = i2c::I2CMode::Fast> class MCP23017 : public i2c::I2CDevice<MODE_>
	{
	private:
		using BusCond = i2c::BusConditions;
		template<MCP23017Port P> using TRAIT = mcp23017_traits::Port_trait<P>;
		template<MCP23017Port P> using T = typename TRAIT<P>::TYPE;

	public:
		/** The I2C mode (speed) used by this instance. */
		static constexpr const i2c::I2CMode MODE = MODE_;

		/** The type of `i2c::I2CManager` that must be used to handle this device.  */
		using MANAGER = typename i2c::I2CDevice<MODE>::MANAGER;

		/**
		 * Create a new device driver for an MCP23017 chip. The @p address must match
		 * the actual address set for that chip (through pins A0, A1, A3).
		 * 
		 * @param manager reference to a suitable i2c::I2CManager for this device
		 * @param address the address part (0-7) set by A0-3 pins of the chip
		 */
		MCP23017(MANAGER& manager, uint8_t address)
			: i2c::I2CDevice<MODE>(manager), device_{uint8_t((BASE_ADDRESS | (address & 0x07)) << 1)}
		{}

		/**
		 * Initialize the chip before operation.
		 * 
		 * @param mirror_interrupts if true then INTA and INTB are mirrored, hence
		 * any interrupt occurring on A or B port will generate a level change on
		 * both pins; hence you can connect any pin to only one interrupt pin on
		 * Arduino if you are lacking available pins.
		 * @param interrupt_polarity the level triggerred on INTA or INTB pin when 
		 * an interrupt occurs
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		bool begin(bool mirror_interrupts = false,
				   InterruptPolarity interrupt_polarity = InterruptPolarity::ACTIVE_HIGH)
		{
			return write_register(IOCON,
								  build_IOCON(mirror_interrupts, interrupt_polarity == InterruptPolarity::ACTIVE_HIGH));
		}

		/**
		 * Configure GPIO on one or both ports of this MCP23017 chip.
		 * 
		 * @tparam P_ which port to configure, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A configuration,
		 * and high byte for port B.
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
		template<MCP23017Port P_> bool configure_gpio(T<P_> direction, T<P_> pullup = T<P_>{}, T<P_> polarity = T<P_>{})
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			return write_register(IODIR_A + SHIFT, direction) && write_register(IPOL_A + SHIFT, polarity)
				   && write_register(GPPU_A + SHIFT, pullup);
		}

		/**
		 * Configure interrupts on one or both ports of this MCP23017 chip.
		 * 
		 * @tparam P_ which port to configure, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A configuration,
		 * and high byte for port B.
		 * @param int_pins each bit sets if the matching pin shall generate interrupts
		 * @param ref contains the reference value for comparison with the actual 
		 * input pin; if input differs, then an interrupt will be triggered for that
		 * pin, provided that @p compare_ref for that bit is also `1`.
		 * @param compare_ref each bit indicates the condition for which the matching 
		 * input pin can generate interrupts; if `0`, an interrupt is generated every
		 * time the input pin changes level, if `1`, an interrupt is generated every
		 * time the input pin level changes to be diferent than @ref matching bit.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		template<MCP23017Port P_>
		bool configure_interrupts(T<P_> int_pins, T<P_> ref = T<P_>{}, T<P_> compare_ref = T<P_>{})
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			return write_register(GPINTEN_A + SHIFT, int_pins) && write_register(DEFVAL_A + SHIFT, ref)
				   && write_register(INTCON_A + SHIFT, compare_ref);
		}

		/**
		 * Set output levels of output pins on one or both ports of this MCP23017 chip.
		 * 
		 * @tparam P_ which port to write to, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @param value each bit indicates the new level of the matching output pin
		 * of the selected port
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed; if so, `i2c::I2CManager.status()`
		 * shall be called for further information on the error.
		 */
		template<MCP23017Port P_> bool values(T<P_> value)
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			return write_register(GPIO_A + SHIFT, value);
		}

		/**
		 * Get levels of pins on one or both ports of this MCP23017 chip.
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @return a value where each bit indicates the current level of the 
		 * matching pin of the selected port
		 */
		template<MCP23017Port P_> T<P_> values()
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			T<P_> value;
			if (read_register(GPIO_A + SHIFT, value))
				return value;
			else
				return T<P_>{};
		}

		/**
		 * Get the pins that generated the latest interrupt on one or both ports
		 * of the MCP23017 chip.
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @return a value where each bit indicates if a pin generated the latest
		 * interrupt or not
		 */
		template<MCP23017Port P_> T<P_> interrupt_flags()
		{
			constexpr uint8_t SHIFT = TRAIT<P_>::REG_SHIFT;
			T<P_> value;
			if (read_register(INTF_A + SHIFT, value))
				return value;
			else
				return T<P_>{};
		}

		/**
		 * Get captured levels, at the time an interrupt was triggered, of pins
		 * on one or both ports of this MCP23017 chip.
		 * This allows to know what generated an interrupt, even if input pins
		 * were modified afterwards.
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @return a value where each bit indicates the level of the matching pin,
		 * captured at the interrupt time.
		 */
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
		static constexpr const uint8_t IOCON_BANK = BV8(7);
		static constexpr const uint8_t IOCON_MIRROR = BV8(6);
		static constexpr const uint8_t IOCON_SEQOP = BV8(5);
		static constexpr const uint8_t IOCON_DISSLW = BV8(4);
		static constexpr const uint8_t IOCON_HAEN = BV8(3);
		static constexpr const uint8_t IOCON_ODR = BV8(2);
		static constexpr const uint8_t IOCON_INTPOL = BV8(1);

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
/// @endcond
