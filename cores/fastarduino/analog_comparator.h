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
 * Analog Comparator API.
 */
#ifndef ANALOGCOMPARATOR_HH
#define ANALOGCOMPARATOR_HH

#include "boards/board_traits.h"
#include "interrupts.h"
#include "utilities.h"

//TODO DOCS
#define REGISTER_ANALOG_COMPARE_ISR_FUNCTION(CALLBACK)		\
	ISR(ANALOG_COMP_vect)									\
	{														\
		CALLBACK();											\
	}

#define REGISTER_ANALOG_COMPARE_ISR_METHOD(HANDLER, CALLBACK)					\
	ISR(ANALOG_COMP_vect)														\
	{																			\
		interrupt::CallbackHandler<void (HANDLER_::*)(), CALLBACK_>::call();	\
	}

#define REGISTER_ANALOG_COMPARE_ISR_EMPTY() EMPTY_INTERRUPT(ANALOG_COMP_vect)

//TODO DOCS
// - supported inputs: all AnalogPins or AIN1, bandgap ref or AIN0
// - supported modes: interrupt (toggle, falling, rising), timer ICP
// Open point: reuse same instance whatever inputs (eg all info in begin()) or not?
namespace analog
{

	enum class ComparatorInterrupt : uint8_t
	{
		NONE = bits::BV8(ACI),
		TOGGLE = bits::BV8(ACI, ACIE),
		FALLING_EDGE = bits::BV8(ACI, ACIE, ACIS1),
		RISING_EDGE = bits::BV8(ACI, ACIE, ACIS1, ACIS0)
	};

	class AnalogComparator
	{
	private:
		static constexpr board_traits::REG8 ACSR_{ACSR};
		using GLOBAL_TRAIT = board_traits::GlobalAnalogPin_trait;

	public:
		// Several ways to begin
		// - use ADC as negative input or AIN1
		// - use Bandgap or AIN0 as positive input
		// - use ISR (with interrupt mode) or not
		// - use Timer1 ICP
		template<board::AnalogPin INPUT>
		void begin(bool use_bandgap = false, ComparatorInterrupt mode = ComparatorInterrupt::NONE, 
					bool trigger_icp = false)
		{
			synchronized begin_<INPUT>(use_bandgap, mode, trigger_icp);
		}

		void end()
		{
			synchronized end_();
		}

		bool output() const
		{
			return ACSR_ & bits::BV8(ACO);
		}

		template<board::AnalogPin INPUT>
		void begin_(bool use_bandgap = false, ComparatorInterrupt mode = ComparatorInterrupt::NONE, 
					bool trigger_icp = false)
		{
			using ATRAIT = board_traits::AnalogPin_trait<INPUT>;
			static_assert(ATRAIT::IS_ANALOG_PIN || INPUT == board::AnalogPin::NONE, "INPUT must not be TEMP!");

			GLOBAL_TRAIT::ADCSRB_ = ((INPUT == board::AnalogPin::NONE) ? 0 : bits::BV8(ACME));
			GLOBAL_TRAIT::ADCSRA_ = ATRAIT::MUX_MASK2;
			GLOBAL_TRAIT::ADMUX_ = ATRAIT::MUX_MASK1;
			ACSR_ = (use_bandgap ? bits::BV8(ACBG) : 0) | uint8_t(mode) | (trigger_icp ? bits::BV8(ACIC) : 0);
		}

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
