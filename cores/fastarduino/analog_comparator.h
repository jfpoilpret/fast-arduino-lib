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

/// @cond api

/**
 * @file 
 * Analog Comparator API.
 */
#ifndef ANALOGCOMPARATOR_HH
#define ANALOGCOMPARATOR_HH

#include "boards/board_traits.h"
#include "interrupts.h"
#include "utilities.h"

/**
 * Register the necessary ISR (Interrupt Service Routine) that will be notified
 * whenever an `analog::AnalogComparator` interrupt is triggered.
 * 
 * @param CALLBACK the function that will be called when the interrupt is
 * triggered
 * 
 * @sa analog::AnalogComparator
 */
#define REGISTER_ANALOG_COMPARE_ISR_FUNCTION(CALLBACK)	\
	ISR(ANALOG_COMP_vect)								\
	{													\
		CALLBACK();										\
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) that will be notified
 * whenever an `analog::AnalogComparator` interrupt is triggered.
 * 
 * @param HANDLER the class holding the callback method
 * @param CALLBACK the method of @p HANDLER that will be called when the interrupt
 * is triggered; this must be a proper PTMF (pointer to member function).
 * 
 * @sa analog::AnalogComparator
 */
#define REGISTER_ANALOG_COMPARE_ISR_METHOD(HANDLER, CALLBACK)				\
	ISR(ANALOG_COMP_vect)													\
	{																		\
		interrupt::CallbackHandler<void (HANDLER::*)(), CALLBACK>::call();	\
	}

/**
 * Register an empty ISR (Interrupt Service Routine) for `analog::AnalogComparator`
 * interrupts.
 * This can be useful if you just need to wake up the MCU from an external signal,
 * but do not need to perform any sepcific stuff with a callback.
 * 
 * @sa analog::AnalogComparator
 */
#define REGISTER_ANALOG_COMPARE_ISR_EMPTY() EMPTY_INTERRUPT(ANALOG_COMP_vect)

/**
 * This macro shall be used in a class containing a private callback method,
 * registered by `REGISTER_ANALOG_COMPARE_ISR_METHOD`.
 * It declares the class where it is used as a friend of all necessary functions
 * so that the private callback method can be called properly.
 */
#define DECL_ANALOG_COMPARE_ISR_HANDLERS_FRIEND friend void ::ANALOG_COMP_vect(void);

namespace analog
{
	/**
	 * Kind of change that will trigger an Analog Comparator Interrupt.
	 * Actual `uint8_t` value matches the related mask for ACSR.
	 */
	enum class ComparatorInterrupt : uint8_t
	{
		/** No interrupt will be generated by the Anolog Comparator. */
		NONE = bits::BV8(ACI),
		/** An interrupt is generated everytime the Analog Comparator output changes. */
		TOGGLE = bits::BV8(ACI, ACIE),
		/** An interrupt is generated everytime the Analog Comparator output changes from 1 to 0. */
		FALLING_EDGE = bits::BV8(ACI, ACIE, ACIS1),
		/** An interrupt is generated everytime the Analog Comparator output changes from 0 to 1. */
		RISING_EDGE = bits::BV8(ACI, ACIE, ACIS1, ACIS0)
	};

	/**
	 * Handler of the Analog Comparator feature.
	 * Only one instance is needed for all related operations.
	 * 
	 * Usage is simple:
	 * 1. start analog comparator with `begin()`; interrupts may then be generated,
	 * depending on given aruments.
	 * 2. Optionally get analog comparator output current value with `output()`
	 * 3. terminate operations with `end()`; no interrupts can be generated anymore.
	 * 
	 * Most methods have 2 flavours:
	 * - asynchronous: `begin_()`, `end_()`
	 * - synchronized: `begin()`, `end()`
	 * 
	 * You shall use asynchronous methods when called from an ISR, otherwise use
	 * synchronized methods.
	 * 
	 * @sa REGISTER_ANALOG_COMPARE_ISR_FUNCTION()
	 * @sa REGISTER_ANALOG_COMPARE_ISR_METHOD()
	 * @sa REGISTER_ANALOG_COMPARE_ISR_EMPTY()
	 */
	class AnalogComparator
	{
	private:
		static constexpr board_traits::REG8 ACSR_{ACSR};
		using GLOBAL_TRAIT = board_traits::GlobalAnalogPin_trait;

	public:
		AnalogComparator() = default;
		AnalogComparator(const AnalogComparator&) = delete;
		AnalogComparator& operator=(const AnalogComparator&) = delete;
		
		/**
		 * Start operations of the Analog Comparator.
		 * Once the Analog Comparator is started, you can possibly poll its output
		 * anytime with `output()`.
		 * 
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `begin_()` instead.
		 * 
		 * @tparam INPUT1 which `board::AnalogPin` is used as comparator negative input;
		 * if `board::AnalogPin::NONE` (the default), then AIN1 pin will be used, instead
		 * of any analog input pin. If the target MCU does not support AIN1 or the provided
		 * `AnalogPin`, compilation will fail with an explicit assertion error.
		 * @tparam INPUT0_BANDGAP if `true`, then use bandgap reference voltage (typically 1.1V)
		 * as comparator positive input, instead of AIN0 pin; if `false`, AIN0 pin will be
		 * used. If the target MCU has no AIN0 pin, and @p INPUT0_BANDGAP is `false`,
		 * compilation will fail with an explicit assertion error.
		 * 
		 * @param mode the interrupt mode to enable for the Analog Comparator; if not
		 * `ComparatorInterrupt::NONE`, then the proper ISR shall have been properly
		 * registered with one of the provided macros.
		 * @param trigger_icp if `true`, then the Analog Comparator output is wired
		 * to the Input Capture of a Timer (which one depends on the MCU target).
		 * Note that some MCU do not support Input Capture, hence this flag will
		 * be left unused for these.
		 * 
		 * @sa end()
		 * @sa output()
		 * @sa begin_()
		 * @sa REGISTER_ANALOG_COMPARE_ISR_FUNCTION()
		 * @sa REGISTER_ANALOG_COMPARE_ISR_METHOD()
		 * @sa REGISTER_ANALOG_COMPARE_ISR_EMPTY()
		 */
		template<board::AnalogPin INPUT1 = board::AnalogPin::NONE, bool INPUT0_BANDGAP = false>
		void begin(ComparatorInterrupt mode = ComparatorInterrupt::NONE, bool trigger_icp = false)
		{
			synchronized begin_<INPUT1, INPUT0_BANDGAP>(mode, trigger_icp);
		}

		/**
		 * Stop operations of the Analog Comparator.
		 * This also disables any related interrupts.
		 * 
		 * Note that this method is synchronized, i.e. it disables all interrupts
		 * during its call and restores interrupts on return.
		 * If you do not need synchronization, then you should better use
		 * `end_()` instead.
		 * 
		 * @sa end_()
		 * @sa begin()
		 */
		void end()
		{
			synchronized end_();
		}

		/**
		 * Get current output of Analog Comparator.
		 * 
		 * @retval true if Analog Comparator output is 1 (when +input is greater than -input).
		 * @retval false if Analog Comparator output is 0 (when +input is less than -input).
		 * 
		 * @sa begin()
		 */
		bool output() const
		{
			return ACSR_ & bits::BV8(ACO);
		}

		/**
		 * Start operations of the Analog Comparator.
		 * Once the Analog Comparator is started, you can possibly poll its output
		 * anytime with `output()`.
		 * 
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `begin()` instead.
		 * 
		 * @tparam INPUT1 which `board::AnalogPin` is used as comparator negative input;
		 * if `board::AnalogPin::NONE` (the default), then AIN1 pin will be used, instead
		 * of any analog input pin. If the target MCU does not support AIN1 or the provided
		 * `AnalogPin`, compilation will fail with an explicit assertion error.
		 * @tparam INPUT0_BANDGAP if `true`, then use bandgap reference voltage (typically 1.1V)
		 * as comparator positive input, instead of AIN0 pin; if `false`, AIN0 pin will be
		 * used. If the target MCU has no AIN0 pin, and @p INPUT0_BANDGAP is `false`,
		 * compilation will fail with an explicit assertion error.
		 * 
		 * @param mode the interrupt mode to enable for the Analog Comparator; if not
		 * `ComparatorInterrupt::NONE`, then the proper ISR shall have been properly
		 * registered with one of the provided macros.
		 * @param trigger_icp if `true`, then the Analog Comparator output is wired
		 * to the Input Capture of a Timer (which one depends on the MCU target).
		 * Note that some MCU do not support Input Capture, hence this flag will
		 * be left unused for these.
		 * 
		 * @sa end_()
		 * @sa output()
		 * @sa begin()
		 * @sa REGISTER_ANALOG_COMPARE_ISR_FUNCTION()
		 * @sa REGISTER_ANALOG_COMPARE_ISR_METHOD()
		 * @sa REGISTER_ANALOG_COMPARE_ISR_EMPTY()
		 */
		template<board::AnalogPin INPUT1 = board::AnalogPin::NONE, bool INPUT0_BANDGAP = false>
		void begin_(ComparatorInterrupt mode = ComparatorInterrupt::NONE, bool trigger_icp = false)
		{
			using ATRAIT = board_traits::AnalogPin_trait<INPUT1>;
			static_assert(ATRAIT::IS_ANALOG_PIN || (INPUT1 == board::AnalogPin::NONE), "INPUT must not be TEMP!");
			static_assert(INPUT1 != board::AnalogPin::NONE || GLOBAL_TRAIT::HAS_AIN1, "Target has no AIN1 pin!");
			static_assert(GLOBAL_TRAIT::HAS_AIN0 || INPUT0_BANDGAP, "Target has no AIN0 hence INPUT0_BANDGAP must be true");

			GLOBAL_TRAIT::ADCSRB_ = ((INPUT1 == board::AnalogPin::NONE) ? 0 : bits::BV8(ACME));
			GLOBAL_TRAIT::ADCSRA_ = ATRAIT::MUX_MASK2;
			GLOBAL_TRAIT::ADMUX_ = ATRAIT::MUX_MASK1;
			ACSR_ = (INPUT0_BANDGAP ? bits::BV8(ACBG) : 0) |
					uint8_t(mode) |
					(trigger_icp ? GLOBAL_TRAIT::ICP_TRIGGER : 0);
		}

		/**
		 * Stop operations of the Analog Comparator.
		 * This also disables any related interrupts.
		 * 
		 * Note that this method is not synchronized, hence you should ensure it
		 * is called only while global interrupts are not enabled.
		 * If you need synchronization, then you should better use
		 * `end()` instead.
		 * 
		 * @sa begin_()
		 * @sa end()
		 */
		void end_()
		{
			GLOBAL_TRAIT::ADCSRB_ = 0;
			GLOBAL_TRAIT::ADCSRA_ = 0;
			GLOBAL_TRAIT::ADMUX_ = 0;
			ACSR_ = 0;
		}
	};
}

#endif /* ANALOGCOMPARATOR_HH */
/// @endcond
