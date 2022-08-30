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
 * General API for handling External Interrupt pins.
 */
#ifndef INT_HH
#define INT_HH

#include "boards/board_traits.h"
#include <avr/interrupt.h>
#include "interrupts.h"
#include "utilities.h"

/**
 * Register the necessary ISR (Interrupt Service Routine) for an External Interrupt 
 * pin.
 * @param INT_NUM the number of the `INT` vector for this @p PIN
 * @param PIN the `board::ExternalInterruptPin` for @p INT_NUM; if @p PIN and 
 * @p INT_NUM do not match, compilation will fail.
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 */
#define REGISTER_INT_ISR_METHOD(INT_NUM, PIN, HANDLER, CALLBACK)                   \
	ISR(CAT3(INT, INT_NUM, _vect))                                                 \
	{                                                                              \
		interrupt::isr_handler_int::int_method<INT_NUM, PIN, HANDLER, CALLBACK>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for an External Interrupt 
 * pin.
 * @param INT_NUM the number of the `INT` vector for this @p PIN
 * @param PIN the `board::ExternalInterruptPin` for @p INT_NUM; if @p PIN and 
 * @p INT_NUM do not match, compilation will fail.
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 */
#define REGISTER_INT_ISR_FUNCTION(INT_NUM, PIN, CALLBACK)                   \
	ISR(CAT3(INT, INT_NUM, _vect))                                          \
	{                                                                       \
		interrupt::isr_handler_int::int_function<INT_NUM, PIN, CALLBACK>(); \
	}

/**
 * Register an empty ISR (Interrupt Service Routine) for an External Interrupt 
 * pin.
 * This can be useful if you just need to wake up the MCU from an external signal,
 * but do not need to perform any sepcific stuff with a callback.
 * @param INT_NUM the number of the `INT` vector for this @p PIN
 * @param PIN the `board::ExternalInterruptPin` for @p INT_NUM; if @p PIN and 
 * @p INT_NUM do not match, compilation will fail.
 */
#define REGISTER_INT_ISR_EMPTY(INT_NUM, PIN)                        \
	extern "C" void CAT3(INT, INT_NUM, _vect)(void) NAKED_SIGNAL;   \
	void CAT3(INT, INT_NUM, _vect)(void)                            \
	{                                                               \
		interrupt::isr_handler_int::check_int_pin<INT_NUM, PIN>();  \
		__asm__ __volatile__("reti" ::);                            \
	}

/**
 * This macro shall be used in a class containing a private callback method,
 * registered by `REGISTER_INT_ISR_METHOD`.
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_INT_ISR_HANDLERS_FRIEND			\
	friend struct interrupt::isr_handler_int;	\
	DECL_INT_ISR_FRIENDS

namespace board
{
	template<ExternalInterruptPin EXT> constexpr DigitalPin EXT_PIN() INLINE;
	/**
	 * Convert an `ExternalInterruptPin` to the matching `DigitalPin`.
	 */
	template<ExternalInterruptPin EXT> constexpr DigitalPin EXT_PIN()
	{
		return board_traits::ExternalInterruptPin_trait<EXT>::ACTUAL_PIN;
	}
};

namespace interrupt
{
	/**
	 * Kind of change that will trigger an External Interrupt for a given pin.
	 * Actual `uint8_t` value matches the related mask for EICR, for up to 4
	 * EXT pins (2 bits per pin), hence the 2-bits value is repeated 4 times.
	 */
	enum class InterruptTrigger : uint8_t
	{
		/** Interrupt is triggered whenever pin level is low. */
		LOW_LEVEL = 0x00,
		/** Interrupt is triggered whenever pin level is changing (rising or falling). */
		ANY_CHANGE = 0x55,
		/** Interrupt is triggered whenever pin level is falling from high to low. */
		FALLING_EDGE = 0xAA,
		/** Interrupt is triggered whenever pin level is rising from low to high. */
		RISING_EDGE = 0xFF
	};

	/**
	 * Handler of an External Interrupt. You must create as many as you have pins 
	 * you want to handle as External Interrupts.
	 * If you need a function or method to be called back when an External Interrupt
	 * occurs for @p PIN, then you have to use `REGISTER_INT_ISR_FUNCTION()` or 
	 * `REGISTER_INT_ISR_METHOD()` macros.
	 * If you don't, then use `REGISTER_INT_ISR_EMPTY()` macro.
	 * @tparam EXTPIN_ the External Interrupt pin
	 * @sa board::ExternalInterruptPin
	 * @sa REGISTER_INT_ISR_FUNCTION
	 * @sa REGISTER_INT_ISR_METHOD
	 * @sa REGISTER_INT_ISR_EMPTY
	 */
	template<board::ExternalInterruptPin EXTPIN_> class INTSignal
	{
	public:
		/** The External Interrupt pin managed by this INTSignal. */
		static constexpr const board::ExternalInterruptPin EXTPIN = EXTPIN_;
		/** The actual connected pin managed by this INTSignal. */
		static constexpr const board::DigitalPin PIN = board::EXT_PIN<EXTPIN>();

	private:
		using INT_TRAIT = board_traits::ExternalInterruptPin_trait<EXTPIN>;

	public:
		INTSignal(const INTSignal&) = delete;
		INTSignal& operator=(const INTSignal&) = delete;
		
		/**
		 * Create a handler for @p EXTPIN external interrupt pin.
		 * This does not automatically enable the interrupt.
		 * @param trigger the kind of level event that shall trigger an External 
		 * Interrupt for @p EXTPIN
		 * @sa enable()
		 * @sa set_trigger()
		 */
		explicit INTSignal(InterruptTrigger trigger = InterruptTrigger::ANY_CHANGE)
		{
			set_trigger_(trigger);
		}

		/**
		 * Change the kind of level event that shall trigger an External 
		 * Interrupt for @p EXTPIN.
		 * Note that this method is synchronized, i.e. it disables interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `set_trigger_()` instead.
		 * 
		 * @param trigger the new kind of level event 
		 * @sa set_trigger_()
		 */
		void set_trigger(InterruptTrigger trigger)
		{
			synchronized INT_TRAIT::EICR_ =
				(INT_TRAIT::EICR_ & bits::COMPL(INT_TRAIT::EICR_MASK)) | (uint8_t(trigger) & INT_TRAIT::EICR_MASK);
		}

		/**
		 * Enable interrupts for this external interrupt pin.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `enable_()` instead.
		 */
		void enable()
		{
			synchronized INT_TRAIT::EIMSK_ |= INT_TRAIT::EIMSK_MASK;
		}

		/**
		 * Disable interrupts for this external interrupt pin.
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `disable_()` instead.
		 */
		void disable()
		{
			synchronized INT_TRAIT::EIMSK_ &= bits::COMPL(INT_TRAIT::EIMSK_MASK);
		}

		/**
		 * Clear the interrupt flag for this external interrupt pin.
		 * Generally, you would not need this method as that interrupt flag
		 * automatically gets cleared when the matching ISR is executed. 
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `clear_()` instead.
		 */
		void clear()
		{
			synchronized INT_TRAIT::EIFR_ |= INT_TRAIT::EIFR_MASK;
		}

		/**
		 * Enable interrupts for this external interrupt pin.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `set_trigger()` instead.
		 */
		void set_trigger_(InterruptTrigger trigger)
		{
			INT_TRAIT::EICR_ = (INT_TRAIT::EICR_ & bits::COMPL(INT_TRAIT::EICR_MASK)) 
							   | (uint8_t(trigger) & INT_TRAIT::EICR_MASK);
		}

		/**
		 * Enable interrupts for this external interrupt pin.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `enable()` instead.
		 */
		void enable_()
		{
			INT_TRAIT::EIMSK_ |= INT_TRAIT::EIMSK_MASK;
		}

		/**
		 * Disable interrupts for this external interrupt pin.
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `disable()` instead.
		 */
		void disable_()
		{
			INT_TRAIT::EIMSK_ &= bits::COMPL(INT_TRAIT::EIMSK_MASK);
		}

		/**
		 * Clear the interrupt flag for this external interrupt pin.
		 * Generally, you would not need this method as that interrupt flag
		 * automatically gets cleared when the matching ISR is executed. 
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `clear()` instead.
		 */
		void clear_()
		{
			INT_TRAIT::EIFR_ |= INT_TRAIT::EIFR_MASK;
		}
	};

	/// @cond notdocumented

	// All INT-related methods called by pre-defined ISR are defined here
	//====================================================================

	struct isr_handler_int
	{
		template<uint8_t INT_NUM_, board::ExternalInterruptPin INT_PIN_> static void check_int_pin()
		{
			static_assert(board_traits::ExternalInterruptPin_trait<INT_PIN_>::INT == INT_NUM_,
						  "PIN INT number must match INT_NUM");
		}

		template<uint8_t INT_NUM_, board::ExternalInterruptPin INT_PIN_, typename HANDLER_, void (HANDLER_::*CALLBACK_)()>
		static void int_method()
		{
			// Check pin is compliant
			check_int_pin<INT_NUM_, INT_PIN_>();
			// Call handler back
			interrupt::CallbackHandler<void (HANDLER_::*)(), CALLBACK_>::call();
		}

		template<uint8_t INT_NUM_, board::ExternalInterruptPin INT_PIN_, void (*CALLBACK_)()> static void int_function()
		{
			// Check pin is compliant
			check_int_pin<INT_NUM_, INT_PIN_>();
			// Call handler back
			CALLBACK_();
		}
	};
	/// @endcond
}

#endif /* INT_HH */
/// @endcond
