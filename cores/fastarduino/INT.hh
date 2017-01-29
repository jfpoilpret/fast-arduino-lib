#ifndef INT_HH
#define	INT_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board_traits.hh"

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

#endif	/* INT_HH */
