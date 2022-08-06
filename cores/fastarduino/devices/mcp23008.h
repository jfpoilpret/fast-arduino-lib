//   Copyright 2016-2021 Jean-Francois Poilpret
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

#include "mcp230xx.h"
#include "../functors.h"
#include "../i2c_device.h"
#include "../i2c_device_utilities.h"

namespace devices::mcp230xx
{
	/**
	 * I2C device driver for Microchip MCP23008 support.
	 * The MCP23008 chip is a 8-Bit I/O Expander with I2C interface.
	 * 
	 * @tparam MANAGER one of FastArduino available I2C Manager
	 * 
	 * @sa devices::mcp23017::MCP23017
	 */
	template<typename MANAGER>
	class MCP23008 : public i2c::I2CDevice<MANAGER>
	{
	private:
		using PARENT = i2c::I2CDevice<MANAGER>;
		template<typename T> using PROXY = typename PARENT::template PROXY<T>;

		// Forward declarations needed by compiler
		template<uint8_t REGISTER, typename T = uint8_t, typename FUNCTOR = functor::Identity<T>>
		using TReadRegisterFuture = i2c::TReadRegisterFuture<MANAGER, REGISTER, T, FUNCTOR>;
		template<uint8_t REGISTER, typename T = uint8_t, typename FUNCTOR = functor::Identity<T>>
		using TWriteRegisterFuture = i2c::TWriteRegisterFuture<MANAGER, REGISTER, T, FUNCTOR>;
		template<typename T, uint8_t... REGISTERS>
		using TWriteMultiRegisterFuture = i2c::TWriteMultiRegisterFuture<MANAGER, T, REGISTERS...>;

		static constexpr uint8_t compute_address(uint8_t address)
		{
			return uint8_t((uint8_t(BASE_ADDRESS) | uint8_t(address & 0x07U)) << 1);
		}

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

		class IOCONConverter
		{
		public:
			using ARG_TYPE = InterruptPolarity;
			using RES_TYPE = uint8_t;
			uint8_t operator()(InterruptPolarity int_polarity) const
			{
				return (int_polarity == InterruptPolarity::ACTIVE_HIGH) ? IOCON_INTPOL : 0;
			}
		};

	public:
		/**
		 * Create a new device driver for an MCP23008 chip. The @p address must match
		 * the actual address set for that chip (through pins A0, A1, A3).
		 * 
		 * @param manager reference to a suitable MANAGER for this device
		 * @param address the address part (0-7) set by A0-3 pins of the chip
		 */
		MCP23008(MANAGER& manager, uint8_t address)
			: PARENT{manager, compute_address(address), i2c::I2C_FAST, true} {}

		// Asynchronous API
		//==================
		/**
		 * Create a future to be used by asynchronous method begin(BeginFuture&).
		 * This is used by `begin()` to pass input settings, and it shall be used 
		 * by the caller to determine when the I2C transaction is finished, hence
		 * when you may use other methods.
		 * 
		 * @param interrupt_polarity the level triggerred on INT pin when 
		 * an interrupt occurs
		 * 
		 * @sa begin(BeginFuture&)
		 */
		using BeginFuture = TWriteRegisterFuture<IOCON, uint8_t, IOCONConverter>;

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
		 * @sa begin(InterruptPolarity)
		 * @sa errors
		 */
		int begin(PROXY<BeginFuture> future)
		{
			return this->async_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method configure_gpio(ConfigureGPIOFuture&).
		 * This is used by `configure_gpio()` to asynchronously launch the I2C transaction,
		 * and it shall be used by the caller to determine when the I2C transaction
		 * is finished.
		 * 
		 * @param direction each bit sets the direction of one pin of the port;
		 * `1` means **I**nput, `0` means **O**utput.
		 * @param pullup each bit (only for input pins) sets if a pullup resistor
		 * shall be internally connected to the pin; if `1`, a pullup is added,
		 * if `0`, no pullup is added.
		 * @param polarity each bit (only for input pins) let you invert polarity of
		 * the matching input pin; if `1`, polarity is inverted, ie one the level
		 * on the input pin is `0`, then it is read as `1`, and conversely.
		 * 
		 * @sa configure_gpio(ConfigureGPIOFuture&)
		 */
		class ConfigureGPIOFuture : public TWriteMultiRegisterFuture<uint8_t, IODIR, IPOL, GPPU>
		{
			using PARENT = TWriteMultiRegisterFuture<uint8_t, IODIR, IPOL, GPPU>;
		public:
			/// @cond notdocumented
			ConfigureGPIOFuture(uint8_t direction, uint8_t pullup = 0, uint8_t polarity = 0)
				:	PARENT{direction, polarity, pullup} {}
			ConfigureGPIOFuture(ConfigureGPIOFuture&&) = default;
			ConfigureGPIOFuture& operator=(ConfigureGPIOFuture&&) = default;
			/// @endcond
		};

		/**
		 * Configure GPIO on the port of this MCP23008 chip.
		 * @warning Asynchronous API!
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
		 * @sa ConfigureGPIOFuture
		 * @sa configure_gpio(uint8_t, uint8_t, uint8_t)
		 * @sa errors
		 */
		int configure_gpio(PROXY<ConfigureGPIOFuture> future)
		{
			return this->async_multi_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method 
		 * configure_interrupts(ConfigureInterruptsFuture&).
		 * This is used by `configure_interrupts()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @param int_pins each bit sets if the matching pin shall generate interrupts
		 * @param ref contains the reference value for comparison with the actual 
		 * input pin; if input differs, then an interrupt will be triggered for that
		 * pin, provided that @p compare_ref for that bit is also `1`.
		 * @param compare_ref each bit indicates the condition for which the matching 
		 * input pin can generate interrupts; if `0`, an interrupt is generated every
		 * time the input pin changes level, if `1`, an interrupt is generated every
		 * time the input pin level changes to be diferent than the matching bit.
		 * 
		 * @sa configure_interrupts(ConfigureInterruptsFuture&)
		 */
		class ConfigureInterruptsFuture : public TWriteMultiRegisterFuture<uint8_t, GPINTEN, DEFVAL, INTCON>
		{
			using PARENT = TWriteMultiRegisterFuture<uint8_t, GPINTEN, DEFVAL, INTCON>;
		public:
			/// @cond notdocumented
			ConfigureInterruptsFuture(uint8_t int_pins, uint8_t ref = 0, uint8_t compare_ref = 0)
				:	PARENT{int_pins, ref, compare_ref} {}
			ConfigureInterruptsFuture(ConfigureInterruptsFuture&&) = default;
			ConfigureInterruptsFuture& operator=(ConfigureInterruptsFuture&&) = default;
			/// @endcond
		};

		/**
		 * Configure interrupts on the port of this MCP23008 chip.
		 * @warning Asynchronous API!
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
		 * @sa ConfigureInterruptsFuture
		 * @sa configure_interrupts(uint8_t, uint8_t, uint8_t)
		 * @sa errors
		 */
		int configure_interrupts(PROXY<ConfigureInterruptsFuture> future)
		{
			return this->async_multi_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method values(SetValuesFuture&).
		 * This is used by `values()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @param value each bit indicates the new level of the matching output pin
		 * of the selected port
		 * 
		 * @sa values(SetValuesFuture&)
		 */
		using SetValuesFuture = TWriteRegisterFuture<GPIO>;

		/**
		 * Set output levels of output pins on the port of this MCP23008 chip.
		 * @warning Asynchronous API!
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
		 * @sa SetValuesFuture
		 * @sa values(uint8_t value)
		 * @sa errors
		 */
		int values(PROXY<SetValuesFuture> future)
		{
			return this->async_write(future);
		}

		/**
		 * Create a future to be used by asynchronous method values(GetValuesFuture&).
		 * This is used by `values()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @sa values(GetValuesFuture&)
		 */
		using GetValuesFuture = TReadRegisterFuture<GPIO>;

		/**
		 * Get levels of pins on the port of this MCP23008 chip.
		 * @warning Asynchronous API!
		 * 
		 * @param future a `GetValuesFuture` passed by the caller, that will be 
		 * updated once the current I2C action is finished.
		 * @retval 0 if no problem occurred during the preparation of I2C transaction
		 * @return an error code if something bad happened; for an asynchronous
		 * I2C Manager, this typically happens when its queue of I2CCommand is full;
		 * for a synchronous I2C Manager, any error on the I2C bus or on the 
		 * target device will trigger an error here. the list of possible errors
		 * is in namespace `errors`.
		 * 
		 * @sa GetValuesFuture
		 * @sa uint8_t values()
		 * @sa errors
		 */
		int values(PROXY<GetValuesFuture> future)
		{
			return this->async_read(future);
		}

		/**
		 * Create a future to be used by asynchronous method 
		 * interrupt_flags(InterruptFlagsFuture&).
		 * This is used by `interrupt_flags()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @sa interrupt_flags(InterruptFlagsFuture&)
		 */
		using InterruptFlagsFuture = TReadRegisterFuture<INTF>;

		/**
		 * Get the pins that generated the latest interrupt on the port
		 * of the MCP23008 chip.
		 * @warning Asynchronous API!
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
		 * @sa InterruptFlagsFuture
		 * @sa uint8_t interrupt_flags()
		 * @sa errors
		 */
		int interrupt_flags(PROXY<InterruptFlagsFuture> future)
		{
			return this->async_read(future);
		}

		/**
		 * Create a future to be used by asynchronous method 
		 * captured_values(CapturedValuesFuture&).
		 * This is used by `captured_values()` to asynchronously launch the I2C 
		 * transaction, and it shall be used by the caller to determine when the I2C
		 * transaction is finished.
		 * 
		 * @sa captured_values(CapturedValuesFuture&)
		 */
		using CapturedValuesFuture = TReadRegisterFuture<INTCAP>;

		/**
		 * Get captured levels, at the time an interrupt was triggered, of pins
		 * on the port of this MCP23008 chip.
		 * This allows to know what generated an interrupt, even if input pins
		 * were modified afterwards.
		 * @warning Asynchronous API!
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
		 * @sa CapturedValuesFuture
		 * @sa uint8_t captured_values()
		 * @sa errors
		 */
		int captured_values(PROXY<CapturedValuesFuture> future)
		{
			return this->async_read(future);
		}

		// Synchronous API
		//=================
		/**
		 * Initialize the chip before operation.
		 * @warning Blocking API!
		 * 
		 * @param interrupt_polarity the level triggerred on INTA or INTB pin when 
		 * an interrupt occurs
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa begin(BeginFuture&)
		 */
		bool begin(InterruptPolarity interrupt_polarity = InterruptPolarity::ACTIVE_HIGH)
		{
			return this->template sync_write<BeginFuture, InterruptPolarity>(interrupt_polarity);
		}

		/**
		 * Configure GPIO on the port of this MCP23008 chip.
		 * @warning Blocking API!
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
		 * @retval false if the operation failed
		 * 
		 * @sa configure_gpio(ConfigureGPIOFuture&)
		 */
		bool configure_gpio(uint8_t direction, uint8_t pullup = 0, uint8_t polarity = 0)
		{
			ConfigureGPIOFuture future{direction, pullup, polarity};
			if (configure_gpio(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Configure interrupts on the port of this MCP23008 chip.
		 * @warning Blocking API!
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
		 * @retval false if the operation failed
		 * 
		 * @sa configure_interrupts(ConfigureInterruptsFuture&)
		 */
		bool configure_interrupts(uint8_t int_pins, uint8_t ref = 0, uint8_t compare_ref = 0)
		{
			ConfigureInterruptsFuture future{int_pins, ref, compare_ref};
			if (configure_interrupts(PARENT::make_proxy(future)) != 0) return false;
			return (future.await() == future::FutureStatus::READY);
		}

		/**
		 * Set output levels of output pins on the port of this MCP23008 chip.
		 * @warning Blocking API!
		 * 
		 * @param value each bit indicates the new level of the matching output pin
		 * of the selected port
		 * @retval true if the operation succeeded
		 * @retval false if the operation failed
		 * 
		 * @sa values(SetValuesFuture&)
		 */
		bool values(uint8_t value)
		{
			return this->template sync_write<SetValuesFuture>(value);
		}

		/**
		 * Get levels of pins on the port of this MCP23008 chip.
		 * @warning Blocking API!
		 * 
		 * @return a value where each bit indicates the current level of the 
		 * matching pin of the selected port
		 * 
		 * @sa values(GetValuesFuture&)
		 */
		uint8_t values()
		{
			return get_value<GetValuesFuture>();
		}

		/**
		 * Get the pins that generated the latest interrupt on the port
		 * of the MCP23008 chip.
		 * @warning Blocking API!
		 * 
		 * @return a value where each bit indicates if a pin generated the latest
		 * interrupt or not
		 * 
		 * @sa interrupt_flags(InterruptFlagsFuture&)
		 */
		uint8_t interrupt_flags()
		{
			return get_value<InterruptFlagsFuture>();
		}

		/**
		 * Get captured levels, at the time an interrupt was triggered, of pins
		 * on the port of this MCP23008 chip.
		 * This allows to know what generated an interrupt, even if input pins
		 * were modified afterwards.
		 * @warning Blocking API!
		 * 
		 * @return a value where each bit indicates the level of the matching pin,
		 * captured at the interrupt time.
		 * 
		 * @sa captured_values(CapturedValuesFuture&)
		 */
		uint8_t captured_values()
		{
			return get_value<CapturedValuesFuture>();
		}

	private:
		template<typename F> uint8_t get_value()
		{
			uint8_t value = 0;
			this->template sync_read<F>(value);
			return value;
		}

		i2c::I2CLightCommand write_stop(uint8_t byte_count = 0) const
		{
			return this->write(byte_count, false, true);
		}
	};
}

#endif /* MCP23008_H */
/// @endcond
