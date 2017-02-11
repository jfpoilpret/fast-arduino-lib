//   Copyright 2016-2017 Jean-Francois Poilpret
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

#ifndef INT_HH
#define	INT_HH

#include <avr/interrupt.h>

#include "utilities.h"
#include "boards/board_traits.h"

// Principles:
// One INT<> instance for each INT vector
// Handling, if needed, can be delegated by INT<> to a ExternalInterruptHandler instance

#define REGISTER_INT_ISR_METHOD(INT_NUM, HANDLER, CALLBACK)	\
REGISTER_ISR_METHOD_(CAT3(INT, INT_NUM, _vect), HANDLER, CALLBACK)

#define REGISTER_INT_ISR_FUNCTION(INT_NUM, CALLBACK)	\
REGISTER_ISR_FUNCTION_(CAT3(INT, INT_NUM, _vect), CALLBACK)

#define REGISTER_INT_ISR_EMPTY(INT_NUM)	EMPTY_INTERRUPT(CAT3(INT, INT_NUM, _vect));

enum class InterruptTrigger: uint8_t
{
	LOW_LEVEL		= 0x00,
	ANY_CHANGE		= 0x55,
	FALLING_EDGE	= 0xAA,
	RISING_EDGE		= 0xFF
};

template<Board::DigitalPin PIN>
class INTSignal
{
protected:
	using PIN_TRAIT = board_traits::DigitalPin_trait<PIN>;
	using INT_TRAIT = board_traits::ExternalInterruptPin_trait<PIN>;
	
public:
	INTSignal(InterruptTrigger trigger = InterruptTrigger::ANY_CHANGE)
	{
		static_assert(PIN_TRAIT::IS_INT, "PIN must be an external interrupt pin");
		_set_trigger(trigger);
	}
	
	inline void set_trigger(InterruptTrigger trigger)
	{
		synchronized INT_TRAIT::EICR_ = (INT_TRAIT::EICR_ & ~INT_TRAIT::EICR_MASK) | (uint8_t(trigger) & INT_TRAIT::EICR_MASK);
	}
	
	inline void enable()
	{
		synchronized INT_TRAIT::EIMSK_ |= INT_TRAIT::EIMSK_MASK;
	}
	inline void disable()
	{
		synchronized INT_TRAIT::EIMSK_ &= ~INT_TRAIT::EIMSK_MASK;
	}
	inline void clear()
	{
		synchronized INT_TRAIT::EIFR_ |= INT_TRAIT::EIFR_MASK;
	}

	inline void _set_trigger(InterruptTrigger trigger)
	{
		INT_TRAIT::EICR_ = (INT_TRAIT::EICR_ & ~INT_TRAIT::EICR_MASK) | (uint8_t(trigger) & INT_TRAIT::EICR_MASK);
	}
	inline void _enable()
	{
		INT_TRAIT::EIMSK_ |= INT_TRAIT::EIMSK_MASK;
	}
	inline void _disable()
	{
		INT_TRAIT::EIMSK_ &= ~INT_TRAIT::EIMSK_MASK;
	}
	inline void _clear()
	{
		INT_TRAIT::EIFR_ |= INT_TRAIT::EIFR_MASK;
	}
};

#endif	/* INT_HH */
