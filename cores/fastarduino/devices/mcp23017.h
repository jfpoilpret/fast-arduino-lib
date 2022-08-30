//   Copyright 2016-2022 Jean-Francois Poilpret
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

#include "mcp230xx.h"
#include "../bits.h"
#include "../functors.h"
#include "../i2c_device.h"
#include "../i2c_device_utilities.h"

namespace devices::mcp230xx
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

	/// @cond notdocumented
	namespace mcp23017_traits
	{
		template<MCP23017Port PORT_> struct Port_trait
		{
			using TYPE = uint8_t;
			static constexpr uint8_t shift(uint8_t reg)
			{
				return reg;
			}
		};

		template<> struct Port_trait<MCP23017Port::PORT_A>
		{
			using TYPE = uint8_t;
			static constexpr uint8_t shift(uint8_t reg)
			{
				return reg;
			}
		};

		template<> struct Port_trait<MCP23017Port::PORT_B>
		{
			using TYPE = uint8_t;
			static constexpr uint8_t shift(uint8_t reg)
			{
				return reg + 1;
			}
		};

		template<> struct Port_trait<MCP23017Port::PORT_AB>
		{
			using TYPE = uint16_t;
			static constexpr uint8_t shift(uint8_t reg)
			{
				return reg;
			}
		};
	}
	/// @endcond

	/**
	 * I2C device driver for Microchip MCP23017 support.
	 * The MCP23017 chip is a 16-Bit I/O Expander with I2C interface.
	 * 
	 * @tparam MANAGER one of FastArduino available I2C Manager
	 */
	template<typename MANAGER>
	class MCP23017 : public i2c::I2CDevice<MANAGER>
	{
	private:
		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename OUT, typename IN> using FUTURE = typename PARENT::template FUTURE<OUT, IN>;

		template<MCP23017Port P> using TRAIT = mcp23017_traits::Port_trait<P>;
		template<MCP23017Port P> using T = typename TRAIT<P>::TYPE;

		// Forward declarations needed by compiler
		template<MCP23017Port P, uint8_t REGISTER>
		using TReadRegisterFuture = i2c::TReadRegisterFuture<MANAGER, TRAIT<P>::shift(REGISTER), T<P>>;
		template<MCP23017Port P, uint8_t REGISTER>
		using TWriteRegisterFuture = i2c::TWriteRegisterFuture<MANAGER, TRAIT<P>::shift(REGISTER), T<P>>;
		template<MCP23017Port P, uint8_t... REGISTERS>
		using TWriteMultiRegisterFuture = i2c::TWriteMultiRegisterFuture<MANAGER, T<P>, TRAIT<P>::shift(REGISTERS)...>;

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
		static constexpr const uint8_t IOCON_BANK = bits::BV8(7);
		static constexpr const uint8_t IOCON_MIRROR = bits::BV8(6);
		static constexpr const uint8_t IOCON_SEQOP = bits::BV8(5);
		static constexpr const uint8_t IOCON_DISSLW = bits::BV8(4);
		static constexpr const uint8_t IOCON_HAEN = bits::BV8(3);
		static constexpr const uint8_t IOCON_ODR = bits::BV8(2);
		static constexpr const uint8_t IOCON_INTPOL = bits::BV8(1);

		static constexpr uint8_t compute_address(uint8_t address)
		{
			return uint8_t((uint8_t(BASE_ADDRESS) | uint8_t(address & 0x07U)) << 1);
		}

	public:
		/**
		 * Create a new device driver for an MCP23017 chip. The @p address must match
		 * the actual address set for that chip (through pins A0, A1, A3).
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 * @param address the address part (0-7) set by A0-3 pins of the chip
		 */
		MCP23017(MANAGER& manager, uint8_t address)
			: PARENT{manager, compute_address(address), i2c::I2C_FAST, true} {}

		// Asynchronous API
		//==================
		/**
		 * Create a future to be used by asynchronous method begin(BeginFuture&).
		 * This is used by `begin()` to pass input settings, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished, hence
		 * when you may use other methods.
		 * 
		 * @param mirror_interrupts if true then INTA and INTB are mirrored, hence
		 * any interrupt occurring on A or B port will generate a level change on
		 * both pins; hence you can connect any pin to only one interrupt pin on
		 * Arduino if you are lacking available pins.
		 * @param interrupt_polarity the level triggerred on INTA or INTB pin when 
		 * an interrupt occurs
		 * 
		 * @sa begin(BeginFuture&)
		 */
		class BeginFuture : public TWriteRegisterFuture<MCP23017Port::PORT_A, IOCON>
		{
			using PARENT = TWriteRegisterFuture<MCP23017Port::PORT_A, IOCON>;
		public:
			/// @cond notdocumented
			explicit BeginFuture(
				bool mirror_interrupts = false,
				InterruptPolarity interrupt_polarity = InterruptPolarity::ACTIVE_HIGH)
				:	PARENT{build_IOCON(
						mirror_interrupts, interrupt_polarity == InterruptPolarity::ACTIVE_HIGH)} {}
			/// @endcond
		};

		/**
		 * Initialize the chip before operation.
		 * @warning Asynchronous API!
		 * 
		 * @param future a `BeginFuture` passed by the caller, that will be updated
		 * once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa BeginFuture
		 * @sa begin(bool, InterruptPolarity)
		 * @sa errors
		 */
		int begin(BeginFuture& future)
		{
			return this->async_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method configure_gpio(ConfigureGPIOFuture<P>&).
		 * This is used by `configure_gpio()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
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
		 * 
		 * @sa configure_gpio(ConfigureGPIOFuture<P>&)
		 */
		template<MCP23017Port P_>
		class ConfigureGPIOFuture : public TWriteMultiRegisterFuture<P_, IODIR_A, IPOL_A, GPPU_A>
		{
			using PARENT = TWriteMultiRegisterFuture<P_, IODIR_A, IPOL_A, GPPU_A>;
		public:
			/// @cond notdocumented
			ConfigureGPIOFuture(T<P_> direction, T<P_> pullup = T<P_>{}, T<P_> polarity = T<P_>{})
				:	PARENT{direction, polarity, pullup} {}
			/// @endcond
		};

		/**
		 * Configure GPIO on one or both ports of this MCP23017 chip.
		 * @warning Asynchronous API!
		 * 
		 * @tparam P_ which port to configure, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A configuration,
		 * and high byte for port B.
		 * 
		 * @param future a `ConfigureGPIOFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa ConfigureGPIOFuture<P_>
		 * @sa configure_gpio(T<P_>, T<P_>, T<P_>)
		 * @sa errors
		 */
		template<MCP23017Port P_> int configure_gpio(ConfigureGPIOFuture<P_>& future)
		{
			return this->async_multi_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method 
		 * configure_interrupts(ConfigureInterruptsFuture<P_>&).
		 * This is used by `configure_interrupts()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
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
		 * time the input pin level changes to be diferent than the matching bit.
		 * 
		 * @sa configure_interrupts(ConfigureInterruptsFuture<P_>&)
		 */
		template<MCP23017Port P_>
		class ConfigureInterruptsFuture : public TWriteMultiRegisterFuture<P_, GPINTEN_A, DEFVAL_A, INTCON_A>
		{
			using PARENT = TWriteMultiRegisterFuture<P_, GPINTEN_A, DEFVAL_A, INTCON_A>;
		public:
			/// @cond notdocumented
			ConfigureInterruptsFuture(T<P_> int_pins, T<P_> ref = T<P_>{}, T<P_> compare_ref = T<P_>{})
				:	PARENT{int_pins, ref, compare_ref} {}
			/// @endcond
		};

		/**
		 * Configure interrupts on one or both ports of this MCP23017 chip.
		 * @warning Asynchronous API!
		 * 
		 * @tparam P_ which port to configure, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A configuration,
		 * and high byte for port B.
		 * 
		 * @param future a `ConfigureInterruptsFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa ConfigureInterruptsFuture<P_>
		 * @sa configure_interrupts(T<P_>, T<P_>, T<P_>)
		 * @sa errors
		 */
		template<MCP23017Port P_>
		int configure_interrupts(ConfigureInterruptsFuture<P_>& future)
		{
			return this->async_multi_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method values(SetValuesFuture<P_>&).
		 * This is used by `values()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @tparam P_ which port to configure, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A configuration,
		 * and high byte for port B.
		 * @param value each bit indicates the new level of the matching output pin
		 * of the selected port
		 * 
		 * @sa values(SetValuesFuture<P_>&)
		 */
		template<MCP23017Port P_>
		using SetValuesFuture = TWriteRegisterFuture<P_, GPIO_A>;

		/**
		 * Set output levels of output pins on one or both ports of this MCP23017 chip.
		 * @warning Asynchronous API!
		 * 
		 * @tparam P_ which port to write to, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * 
		 * @param future a `SetValuesFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa SetValuesFuture<P_>
		 * @sa values(T<P_> value)
		 * @sa errors
		 */
		template<MCP23017Port P_> int values(SetValuesFuture<P_>& future)
		{
			return this->async_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method values(GetValuesFuture<P_>&).
		 * This is used by `values()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @tparam P_ which port to configure, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A configuration,
		 * and high byte for port B.
		 * 
		 * @sa values(GetValuesFuture<P_>&)
		 */
		template<MCP23017Port P_> 
		using GetValuesFuture = TReadRegisterFuture<P_, GPIO_A>;

		/**
		 * Get levels of pins on one or both ports of this MCP23017 chip.
		 * @warning Asynchronous API!
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @param future a `GetValuesFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa GetValuesFuture<P_>
		 * @sa T<P_> values()
		 * @sa errors
		 */
		template<MCP23017Port P_> int values(GetValuesFuture<P_>& future)
		{
			return this->async_read(future);
		}

		/**
		 * Create a future to be used by asynchronous method 
		 * interrupt_flags(InterruptFlagsFuture<P_>&).
		 * This is used by `interrupt_flags()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * 
		 * @sa interrupt_flags(InterruptFlagsFuture<P_>&)
		 */
		template<MCP23017Port P_> 
		using InterruptFlagsFuture = TReadRegisterFuture<P_, INTF_A>;

		/**
		 * Get the pins that generated the latest interrupt on one or both ports
		 * of the MCP23017 chip.
		 * @warning Asynchronous API!
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * 
		 * @param future a `InterruptFlagsFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa InterruptFlagsFuture<P_>
		 * @sa T<P_> interrupt_flags()
		 * @sa errors
		 */
		template<MCP23017Port P_> int interrupt_flags(InterruptFlagsFuture<P_>& future)
		{
			return this->async_read(future);
		}

		/**
		 * Create a future to be used by asynchronous method 
		 * captured_values(CapturedValuesFuture<P_>&).
		 * This is used by `captured_values()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * 
		 * @sa captured_values(CapturedValuesFuture<P_>&)
		 */
		template<MCP23017Port P_>
		using CapturedValuesFuture = TReadRegisterFuture<P_, INTCAP_A>;

		/**
		 * Get captured levels, at the time an interrupt was triggered, of pins
		 * on one or both ports of this MCP23017 chip.
		 * This allows to know what generated an interrupt, even if input pins
		 * were modified afterwards.
		 * @warning Asynchronous API!
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * 
		 * @param future a `CapturedValuesFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa CapturedValuesFuture<P_>
		 * @sa T<P_> captured_values()
		 * @sa errors
		 */
		template<MCP23017Port P_> int captured_values(CapturedValuesFuture<P_>& future)
		{
			return this->async_read(future);
		}

		// Synchronous API
		//=================
		/**
		 * Initialize the chip before operation.
		 * @warning Blocking API!
		 * 
		 * @param mirror_interrupts if true then INTA and INTB are mirrored, hence
		 * any interrupt occurring on A or B port will generate a level change on
		 * both pins; hence you can connect any pin to only one interrupt pin on
		 * Arduino if you are lacking available pins.
		 * @param interrupt_polarity the level triggerred on INTA or INTB pin when 
		 * an interrupt occurs
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa begin(BeginFuture&)
		 */
		bool begin(bool mirror_interrupts = false,
				   InterruptPolarity interrupt_polarity = InterruptPolarity::ACTIVE_HIGH)
		{
			BeginFuture future{mirror_interrupts, interrupt_polarity};
			if (begin(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Configure GPIO on one or both ports of this MCP23017 chip.
		 * @warning Blocking API!
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
		 * @retval false if the operation failed
		 * 
		 * @sa configure_gpio(ConfigureGPIOFuture<P_>&)
		 */
		template<MCP23017Port P_> bool configure_gpio(T<P_> direction, T<P_> pullup = T<P_>{}, T<P_> polarity = T<P_>{})
		{
			ConfigureGPIOFuture<P_> future{direction, pullup, polarity};
			if (configure_gpio(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Configure interrupts on one or both ports of this MCP23017 chip.
		 * @warning Blocking API!
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
		 * time the input pin level changes to be diferent than the matching bit.
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa configure_interrupts(ConfigureInterruptsFuture<P_>&)
		 */
		template<MCP23017Port P_>
		bool configure_interrupts(T<P_> int_pins, T<P_> ref = T<P_>{}, T<P_> compare_ref = T<P_>{})
		{
			ConfigureInterruptsFuture<P_> future{int_pins, ref, compare_ref};
			if (configure_interrupts(future) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Set output levels of output pins on one or both ports of this MCP23017 chip.
		 * @warning Blocking API!
		 * 
		 * @tparam P_ which port to write to, may be A, B or both; if both, then
		 * all arguments will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @param value each bit indicates the new level of the matching output pin
		 * of the selected port
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa values(SetValuesFuture<P_>&)
		 */
		template<MCP23017Port P_> bool values(T<P_> value)
		{
			return this->template sync_write<SetValuesFuture<P_>>(value);
		}

		/**
		 * Get levels of pins on one or both ports of this MCP23017 chip.
		 * @warning Blocking API!
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @return a value where each bit indicates the current level of the 
		 * matching pin of the selected port
		 * 
		 * @sa values(GetValuesFuture<P_>&)
		 */
		template<MCP23017Port P_> T<P_> values()
		{
			return get_value<GetValuesFuture<P_>, T<P_>>();
		}

		/**
		 * Get the pins that generated the latest interrupt on one or both ports
		 * of the MCP23017 chip.
		 * @warning Blocking API!
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @return a value where each bit indicates if a pin generated the latest
		 * interrupt or not
		 * 
		 * @sa interrupt_flags(InterruptFlagsFuture<P_>&)
		 */
		template<MCP23017Port P_> T<P_> interrupt_flags()
		{
			return get_value<InterruptFlagsFuture<P_>, T<P_>>();
		}

		/**
		 * Get captured levels, at the time an interrupt was triggered, of pins
		 * on one or both ports of this MCP23017 chip.
		 * This allows to know what generated an interrupt, even if input pins
		 * were modified afterwards.
		 * @warning Blocking API!
		 * 
		 * @tparam P_ which port to read from, may be A, B or both; if both, then
		 * the return type will be `uint16_t`, with low byte for port A,
		 * and high byte for port B.
		 * @return a value where each bit indicates the level of the matching pin,
		 * captured at the interrupt time.
		 * 
		 * @sa captured_values(CapturedValuesFuture<P_>&)
		 */
		template<MCP23017Port P_> T<P_> captured_values()
		{
			return get_value<CapturedValuesFuture<P_>, T<P_>>();
		}

	private:
		template<typename F, typename T> T get_value()
		{
			T value;
			if (this->template sync_read<F>(value))
				return value;
			else
				return T{};
		}

		i2c::I2CLightCommand write_stop(uint8_t byte_count = 0) const
		{
			return this->write(byte_count, false, true);
		}

		static constexpr uint8_t build_IOCON(bool mirror, bool int_polarity)
		{
			return bits::ORIF8(mirror, IOCON_MIRROR, int_polarity, IOCON_INTPOL);
		}
	};
}

#endif /* MCP23017_H */
/// @endcond
