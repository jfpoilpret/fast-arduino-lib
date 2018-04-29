//   Copyright 2016-2018 Jean-Francois Poilpret
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
 * General API for handling Pin Change Interrupts.
 * The basic principle is to instantiate one `interrupt::PCI` per PCINT vector 
 * you need to handle, whatever the number of pins you need in this PCINT.
 * If you add a callback, it will be called for whatever pin may have changed 
 * state in the list of pins supported by your `interrupt::PCI` instance.
 */
#ifndef PCI_HH
#define PCI_HH

#include "boards/board.h"
#include "boards/board_traits.h"
#include <avr/interrupt.h>
#include "interrupts.h"
#include "utilities.h"

/// @cond notdocumented
#define CHECK_PCI_PIN_(PIN, PCI_NUM)                                                                          \
	static_assert(board_traits::PCI_trait<PCI_NUM>::PORT != board::Port::NONE, "PORT must support PCI");      \
	static_assert(board_traits::DigitalPin_trait<PIN>::PORT == board_traits::PCI_trait<PCI_NUM>::PORT,        \
				  "PIN port must match PCI_NUM port");                                                        \
	static_assert(_BV(board_traits::DigitalPin_trait<PIN>::BIT) & board_traits::PCI_trait<PCI_NUM>::PCI_MASK, \
				  "PIN must be a PCINT pin");
/// @endcond

/**
 * Register the necessary ISR (Interrupt Service Routine) for a Pin Change Interrupt 
 * vector.
 * @param PCI_NUM the number of the `PCINT` vector for the given @p PIN pins
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 * @param PIN the `board::DigitalPin` pins for @p PCI_NUM; if any of the given 
 * @p PIN does not match with @p PCI_NUM, compilation will fail.
 */
#define REGISTER_PCI_ISR_METHOD(PCI_NUM, HANDLER, CALLBACK, PIN, ...) \
	FOR_EACH(CHECK_PCI_PIN_, PCI_NUM, PIN, ##__VA_ARGS__)             \
	REGISTER_ISR_METHOD_(CAT3(PCINT, PCI_NUM, _vect), HANDLER, CALLBACK)

/**
 * Register the necessary ISR (Interrupt Service Routine) for a Pin Change Interrupt 
 * vector.
 * @param PCI_NUM the number of the `PCINT` vector for the given @p PIN pins
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 * @param PIN the `board::DigitalPin` pins for @p PCI_NUM; if any of the given 
 * @p PIN does not match with @p PCI_NUM, compilation will fail.
 */
#define REGISTER_PCI_ISR_FUNCTION(PCI_NUM, CALLBACK, PIN, ...) \
	FOR_EACH(CHECK_PCI_PIN_, PCI_NUM, PIN, ##__VA_ARGS__)      \
	REGISTER_ISR_FUNCTION_(CAT3(PCINT, PCI_NUM, _vect), CALLBACK)

/**
 * Register an empty ISR (Interrupt Service Routine) for a Pin Change Interrupt 
 * vector.
 * This can be useful if you just need to wake up the MCU from an external signal,
 * but do not need to perform any sepcific stuff with a callback.
 * @param PCI_NUM the number of the `PCINT` vector for the given @p PIN pins
 * @param PIN the `board::DigitalPin` pins for @p PCI_NUM; if any of the given 
 * @p PIN does not match with @p PCI_NUM, compilation will fail.
 */
#define REGISTER_PCI_ISR_EMPTY(PCI_NUM, PIN, ...)         \
	FOR_EACH(CHECK_PCI_PIN_, PCI_NUM, PIN, ##__VA_ARGS__) \
	EMPTY_INTERRUPT(CAT3(PCINT, PCI_NUM, _vect))

namespace interrupt
{
	/**
	 * Handler of a Pin Change Interrupt vector, i.e. a @p PORT_. For each PCINT
	 * vector you need, you must create one instance of this handler. 
	 * With one instance, you are then able to handle individually each pin 
	 * which interrupt you want to enable or disable. 
	 * If you need a function or method to be called back when a Pin Change Interrupt
	 * occurs for a PCINT vector, then you have to use `REGISTER_PCI_ISR_FUNCTION` or 
	 * `REGISTER_PCI_ISR_METHOD()` macros.
	 * If you don't then use `REGISTER_PCI_ISR_EMPTY` macro.
	 * If you don't know the port you need to handle but only a pin, then you can
	 * use `PCIType<PIN>::TYPE`.
	 * 
	 * @tparam PORT_ the IO port which PCINT vector you want to manage; if @p PORT_
	 * has no associated PCINT vector then the program will not compile.
	 * @sa REGISTER_PCI_ISR_FUNCTION
	 * @sa REGISTER_PCI_ISR_METHOD
	 * @sa REGISTER_PCI_ISR_EMPTY
	 * @sa PCIType
	 */
	template<board::Port PORT_> class PCISignal
	{
	private:
		using PORT_TRAIT = board_traits::Port_trait<PORT_>;
		using TRAIT = board_traits::PCI_trait<PORT_TRAIT::PCINT>;

	public:
		/** The IO port which PCINT vector is managed by this PCISignal. */
		static constexpr const board::Port PORT = PORT_;

		/// @cond notdocumented
		//NOTE this constructor exists only to add a static_assert checked when 
		// PCISignal is constructed not when its template type gets instantiated.
		PCISignal()
		{
			static_assert(PORT_TRAIT::PCINT != 0xFF, "PORT_ must support PCINT");
		}
		/// @endcond

		/** The PCINT vector number for this PCISignal. */
		static constexpr const uint8_t PCINT = PORT_TRAIT::PCINT;

		/**
		 * Enable pin change interrupts for the port of the `PCISignal`.
		 * Enabling interrupts on the port is not sufficient, you should enable 
		 * pin change interrupts for each individual pin you are interested in,
		 * with `enable_pin()` or `enable_pins()`.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `enable_()` instead.
		 * @sa enable_()
		 * @sa disable()
		 * @sa enable_pin()
		 * @sa enable_pins()
		 */
		inline void enable()
		{
			synchronized TRAIT::PCICR_ |= TRAIT::PCICR_MASK;
		}

		/**
		 * Disable all pin change interrupts for the port of the `PCISignal`.
		 * If you need to only diable pin change interrupts for some pins of that
		 * port, then use `disable_pin()` instead.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `disable_()` instead.
		 * @sa enable()
		 * @sa disable_()
		 * @sa disable_pin()
		 */
		inline void disable()
		{
			synchronized TRAIT::PCICR_ &= ~TRAIT::PCICR_MASK;
		}

		/**
		 * Clear the interrupt flag for this pin change interrupt port.
		 * Generally, you would not need this method as that interrupt flag
		 * automatically gets cleared when the matching ISR is executed. 
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `clear_()` instead.
		 * @sa clear_()
		 */
		inline void clear()
		{
			synchronized TRAIT::PCIFR_ |= TRAIT::PCIFR_MASK;
		}

		/**
		 * Set the exact list of pins, in this port, for which Pin Change Interrupts
		 * must be enabled. For other pins, interrupts will be disabled.
		 * This method provides no compile-time safety net if you pass a wrong mask.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `set_enable_pins_()` instead.
		 * @param mask the mask of pin bits which pin change interrupts you want
		 * to enable for this port; other pins will have interrupts disbaled.
		 * @sa set_enable_pins_()
		 * @sa enable_pins()
		 * @sa disable_pins()
		 */
		inline void set_enable_pins(uint8_t mask)
		{
			synchronized TRAIT::PCMSK_ = mask;
		}

		/**
		 * Enable pin change interrupts for several pins of this port.
		 * This does not enable completely interrupts, for this you need to also
		 * call `enable()`.
		 * This method is useful when you have several pins to enable at once; if
		 * you have only one pin, then `enable_pin()` is preferred. 
		 * This method provides no compile-time safety net if you pass a wrong mask.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `enable_pins_()` instead.
		 * @param mask the mask of pin bits which pin change interrupts you want
		 * to enable for this port; only pins included in @p mask will be affected;
		 * if other pins are already enabled, they won't be changed.
		 * @sa enable()
		 * @sa enable_pins_()
		 * @sa enable_pin()
		 * @sa disable_pin()
		 */
		inline void enable_pins(uint8_t mask)
		{
			synchronized TRAIT::PCMSK_ |= mask;
		}

		/**
		 * Disable pin change interrupts for several pins of this port.
		 * This does not disable completely interrupts, for this you need to also
		 * call `disable()`.
		 * This method is useful when you have several pins to disable at once; if
		 * you have only one pin, then `disable_pin()` is preferred. 
		 * This method provides no compile-time safety net if you pass a wrong mask.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `disable_pins_()` instead.
		 * @param mask the mask of pin bits which pin change interrupts you want
		 * to disable for this port; only pins included in @p mask will be affected;
		 * if other pins are already enabled, they won't be changed.
		 * @sa disable()
		 * @sa disable_pins_()
		 * @sa enable_pin()
		 * @sa disable_pin()
		 */
		inline void disable_pins(uint8_t mask)
		{
			synchronized TRAIT::PCMSK_ &= ~mask;
		}

		/**
		 * Enable Pin Change Interrupts for @p PIN.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `enable_pin_()` instead.
		 * @tparam PIN the pin for which to enable Pin Change Interrupts; this must
		 * belong to the handler's @p PORT and must support Pin Change Interrupts,
		 * otherwise compilation will fail.
		 * @sa enable_pin_()
		 * @sa disable_pin()
		 * @sa enable_pins()
		 */
		template<board::DigitalPin PIN> inline void enable_pin()
		{
			static_assert(board_traits::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
			static_assert(TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<PIN>::BIT),
						  "PIN must be a PCI within PORT");
			enable_pins(_BV(board_traits::DigitalPin_trait<PIN>::BIT));
		}

		/**
		 * Disable Pin Change Interrupts for @p PIN.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `disable_pin_()` instead.
		 * @tparam PIN the pin for which to disable Pin Change Interrupts; this must
		 * belong to the handler's @p PORT and must support Pin Change Interrupts,
		 * otherwise compilation will fail.
		 * @sa disable_pin_()
		 * @sa enable_pin()
		 */
		template<board::DigitalPin PIN> inline void disable_pin()
		{
			static_assert(board_traits::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
			static_assert(TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<PIN>::BIT),
						  "PIN must be a PCI within PORT");
			synchronized TRAIT::PCMSK_ &= ~_BV(board_traits::DigitalPin_trait<PIN>::BIT);
		}

		/**
		 * Enable pin change interrupts for the port of the `PCISignal`.
		 * Enabling interrupts on the port is not sufficient, you should enable 
		 * pin change interrupts for each individual pin you are interested in,
		 * with `enable_pin()` or `enable_pins()`.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `enable()` instead.
		 * @sa enable()
		 * @sa disable_()
		 * @sa enable_pin_()
		 * @sa enable_pins_()
		 */
		inline void enable_()
		{
			TRAIT::PCICR_ |= TRAIT::PCICR_MASK;
		}

		/**
		 * Disable all pin change interrupts for the port of the `PCISignal`.
		 * If you need to only diable pin change interrupts for some pins of that
		 * port, then use `disable_pin()` instead.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `disable()` instead.
		 * @sa enable()
		 * @sa disable()
		 * @sa disable_pin_()
		 */
		inline void disable_()
		{
			TRAIT::PCICR_ &= ~TRAIT::PCICR_MASK;
		}

		/**
		 * Clear the interrupt flag for this pin change interrupt port.
		 * Generally, you would not need this method as that interrupt flag
		 * automatically gets cleared when the matching ISR is executed. 
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `clear()` instead.
		 * @sa clear()
		 */
		inline void clear_()
		{
			TRAIT::PCIFR_ |= TRAIT::PCIFR_MASK;
		}

		/**
		 * Set the exact list of pins, in this port, for which Pin Change Interrupts
		 * must be enabled. For other pins, interrupts will be disabled.
		 * This method provides no compile-time safety net if you pass a wrong mask.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `set_enable_pins()` instead.
		 * @param mask the mask of pin bits which pin change interrupts you want
		 * to enable for this port; other pins will have interrupts disbaled.
		 * @sa set_enable_pins()
		 * @sa enable_pins_()
		 * @sa disable_pins_()
		 */
		inline void set_enable_pins_(uint8_t mask)
		{
			TRAIT::PCMSK_ = mask;
		}

		/**
		 * Enable pin change interrupts for several pins of this port.
		 * This does not enable completely interrupts, for this you need to also
		 * call `enable_()`.
		 * This method is useful when you have several pins to enable at once; if
		 * you have only one pin, then `enable_pin_()` is preferred. 
		 * This method provides no compile-time safety net if you pass a wrong mask.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `enable_pin()` instead.
		 * @param mask the mask of pin bits which pin change interrupts you want
		 * to enable for this port; only pins included in @p mask will be affected;
		 * if other pins are already enabled, they won't be changed.
		 * @sa enable_()
		 * @sa enable_pins()
		 * @sa enable_pin_()
		 * @sa disable_pin_()
		 */
		inline void enable_pins_(uint8_t mask)
		{
			TRAIT::PCMSK_ |= mask;
		}

		/**
		 * Disable pin change interrupts for several pins of this port.
		 * This does not disable completely interrupts, for this you need to also
		 * call `disable_()`.
		 * This method is useful when you have several pins to disable at once; if
		 * you have only one pin, then `disable_pin_()` is preferred. 
		 * This method provides no compile-time safety net if you pass a wrong mask.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `disable_pin()` instead.
		 * @param mask the mask of pin bits which pin change interrupts you want
		 * to disable for this port; only pins included in @p mask will be affected;
		 * if other pins are already enabled, they won't be changed.
		 * @sa disable_()
		 * @sa disable_pins()
		 * @sa enable_pin_()
		 * @sa disable_pin_()
		 */
		inline void disable_pins_(uint8_t mask)
		{
			TRAIT::PCMSK_ &= ~mask;
		}

		/**
		 * Enable Pin Change Interrupts for @p PIN.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `enable_pin()` instead.
		 * @tparam PIN the pin for which to enable Pin Change Interrupts; this must
		 * belong to the handler's @p PORT and must support Pin Change Interrupts,
		 * otherwise compilation will fail.
		 * @sa enable_pin()
		 * @sa disable_pin_()
		 * @sa enable_pins_()
		 */
		template<board::DigitalPin PIN> inline void enable_pin_()
		{
			static_assert(board_traits::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
			static_assert(TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<PIN>::BIT),
						  "PIN must be a PCI within PORT");
			enable_pins_(_BV(board_traits::DigitalPin_trait<PIN>::BIT));
		}

		/**
		 * Disable Pin Change Interrupts for @p PIN.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `disable_pin()` instead.
		 * @tparam PIN the pin for which to disable Pin Change Interrupts; this must
		 * belong to the handler's @p PORT and must support Pin Change Interrupts,
		 * otherwise compilation will fail.
		 * @sa disable_pin()
		 * @sa enable_pin_()
		 */
		template<board::DigitalPin PIN> inline void disable_pin_()
		{
			static_assert(board_traits::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
			static_assert(TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<PIN>::BIT),
						  "PIN must be a PCI within PORT");
			TRAIT::PCMSK_ &= ~_BV(board_traits::DigitalPin_trait<PIN>::BIT);
		}
	};

	/**
	 * Helper class that easily converts a @p PIN into the right `PCISignal`.
	 * The following snippet demonstrates usage of `PCIType` to declare a 
	 * `PCISignal` instance for later use in a function:
	 * 
	 * @code
	 * void f()
	 * {
	 *     constexpr const board::DigitalPin PIN = board::DigitalPin::D7;
	 *     interrupt::PCIType<PIN>::TYPE pci;
	 *     pci.enable_pin<PIN>();
	 *     pci.enable();
	 *     ...
	 *     pci.disable();
	 * }
	 * @endcode
	 * @sa board::DigitalPin
	 */
	template<board::DigitalPin PIN> struct PCIType
	{
		/** PCISignal type for @p PIN */
		using TYPE = PCISignal<board_traits::DigitalPin_trait<PIN>::PORT>;
		/** `PCINT` vector number for this @p PIN */
		static constexpr const uint8_t PCINT =
			board_traits::Port_trait<board_traits::DigitalPin_trait<PIN>::PORT>::PCINT;
	};
}

#endif /* PCI_HH */
/// @endcond
