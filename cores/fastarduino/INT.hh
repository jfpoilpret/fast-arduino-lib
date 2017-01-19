#ifndef INT_HH
#define	INT_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board_traits.hh"
#include "ExternalInterrupt.hh"

// Principles:
// One INT<> instance for each INT vector
// Handling, if needed, can be delegated by INT<> to a ExternalInterruptHandler instance

// These macros are internally used in further macros and should not be used in your programs
#define ALIAS_INT_(PN, P0)	\
	ISR(INT ## PN ## _vect, ISR_ALIASOF(INT ## P0 ## _vect));

#define PREPEND_INT_(PN, ...) Board::ExternalInterruptPin::EXT ## PN

#define USE_INTS(P0, ...)																						\
ISR(INT ## P0 ## _vect)																							\
{																												\
	INT_impl::InterruptHandler<FOR_EACH_SEP(PREPEND_INT_, , EMPTY, COMMA, EMPTY, P0, ##__VA_ARGS__)>::handle();	\
}																												\
																												\
FOR_EACH(ALIAS_INT_, P0, ##__VA_ARGS__)

#define EMPTY_INT_(PN, _0)					\
EMPTY_INTERRUPT(INT ## PN ## _vect);

#define USE_EMPTY_INTS(P0, ...)				\
FOR_EACH(EMPTY_INT_, _0, P0, ##__VA_ARGS__)

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
	using PIN_TRAIT = Board::DigitalPin_trait<PIN>;
	using INT_TRAIT = Board::ExternalInterruptPin_trait<PIN>;
	
public:
	INTSignal(InterruptTrigger trigger = InterruptTrigger::ANY_CHANGE)
	{
		static_assert(PIN_TRAIT::IS_INT, "PIN must be an external interrupt pin");
		_set_trigger(trigger);
	}
	
	inline void set_trigger(InterruptTrigger trigger)
	{
		synchronized set_bit_field(INT_TRAIT::EICR_, INT_TRAIT::EICR_MASK, uint8_t(trigger));
	}
	
	inline void enable()
	{
		synchronized set_mask(INT_TRAIT::EIMSK_, INT_TRAIT::EIMSK_MASK);
	}
	inline void disable()
	{
		synchronized clear_mask(INT_TRAIT::EIMSK_, INT_TRAIT::EIMSK_MASK);
	}
	inline void clear()
	{
		synchronized set_mask(INT_TRAIT::EIFR_, INT_TRAIT::EIFR_MASK);
	}

	inline void _set_trigger(InterruptTrigger trigger)
	{
		set_bit_field(INT_TRAIT::EICR_, INT_TRAIT::EICR_MASK, uint8_t(trigger));
	}
	inline void _enable()
	{
		set_mask(INT_TRAIT::EIMSK_, INT_TRAIT::EIMSK_MASK);
	}
	inline void _disable()
	{
		clear_mask(INT_TRAIT::EIMSK_, INT_TRAIT::EIMSK_MASK);
	}
	inline void _clear()
	{
		set_mask(INT_TRAIT::EIFR_, INT_TRAIT::EIFR_MASK);
	}
};

// Forward declaration necessary to be declared as friend
// Complete declaration can be found at the end of this file
namespace INT_impl
{
	template<Board::DigitalPin P0, Board::DigitalPin... PS> struct InterruptHandler;
}

template<Board::DigitalPin PIN>
class INT: public INTSignal<PIN>
{
public:
	INT(InterruptTrigger trigger = InterruptTrigger::ANY_CHANGE, ExternalInterruptHandler* handler = 0)
		:INTSignal<PIN>{trigger}
	{
		_handler = handler;
	}
	
	inline void set_handler(ExternalInterruptHandler* handler)
	{
		synchronized _handler = handler;
	}

private:
	static ExternalInterruptHandler* _handler;
	
	static void handle_if_needed()
	{
		//FIXME need conversion from PIN to INT in DigitalPin_trait
		if (GPIOR0 == INTSignal<PIN>::INT_TRAIT::INT)
			_handler->on_pin_change();
	}
	
	template<Board::DigitalPin, Board::DigitalPin...> friend struct INT_impl::InterruptHandler;
};

template<Board::DigitalPin INT_NUM>
ExternalInterruptHandler* INT<INT_NUM>::_handler = 0;

namespace INT_impl
{
	template<Board::DigitalPin... PS>
	struct ISRHandler
	{
		static void callback() __attribute__((naked))
		{
			asm volatile(
				"push r1\n\t"
				"push r0\n\t"
				"in r0, __SREG__\n\t"
				"push r0\n\t"
				"eor r1, r1\n\t"
				"push r18\n\t"
				"push r19\n\t"
				"push r20\n\t"
				"push r21\n\t"
				"push r22\n\t"
				"push r23\n\t"
				"push r24\n\t"
				"push r25\n\t"
				"push r26\n\t"
				"push r27\n\t"
				"push r30\n\t"
				"push r31\n\t"
			);
			InterruptHandler<PS...>::handle();
			asm volatile(
				"pop r31\n\t"
				"pop r30\n\t"
				"pop r27\n\t"
				"pop r26\n\t"
				"pop r25\n\t"
				"pop r24\n\t"
				"pop r23\n\t"
				"pop r22\n\t"
				"pop r21\n\t"
				"pop r20\n\t"
				"pop r19\n\t"
				"pop r18\n\t"
				"pop r0\n\t"
				"out __SREG__, r0\n\t"
				"pop r0\n\t"
				"pop r1\n\t"
				"ret\n\t"
			);
		}
	};
	
	template<Board::DigitalPin P0, Board::DigitalPin... PS>
	struct InterruptHandler
	{
		static void handle()
		{
			InterruptHandler<P0>::handle();
			InterruptHandler<PS...>::handle();
		}
	};

	template<Board::DigitalPin P>
	struct InterruptHandler<P>
	{
		static void handle()
		{
			INT<P>::handle_if_needed();
		}
	};
}

#endif	/* INT_HH */

