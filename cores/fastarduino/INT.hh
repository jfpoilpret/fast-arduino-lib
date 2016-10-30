#ifndef INT_HH
#define	INT_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board.hh"

// Principles:
// One INT<> instance for each INT vector
// Handling is delegated by INT<> to a INTHandler instance
//TODO factor PCIHandler and INTHandler into one unique handler class

// This macro is internally used in further macros and should not be used in your programs
#define _USE_INT(INT_NUM)										\
ISR(INT ## INT_NUM ## _vect)									\
{																\
	INT<Board::ExternalInterruptPin::EXT ## INT_NUM>::handle();	\
}

// This macro is internally used in further macros and should not be used in your programs
#define _USE_EMPTY_INT(INT_NUM)										\
EMPTY_INTERRUPT(INT ## INT_NUM ## _vect)

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use INT for a given INT pin in your program, hence you need the proper ISR vector correctly defined
#define USE_INT0()	_USE_INT(0)
#define USE_INT1()	_USE_INT(1)
#define USE_INT2()	_USE_INT(2)
#define USE_INT3()	_USE_INT(3)
#define USE_INT4()	_USE_INT(4)
#define USE_INT5()	_USE_INT(5)
#define USE_INT6()	_USE_INT(6)
#define USE_INT7()	_USE_INT(7)

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use INTSignal for a given INT pin in your program, hence you need the proper ISR vector correctly defined
// Use these when you want an INT to wake up from sleep but you don't need any handler to be executed
#define USE_EMPTY_INT0()	_USE_EMPTY_INT(0)
#define USE_EMPTY_INT1()	_USE_EMPTY_INT(1)
#define USE_EMPTY_INT2()	_USE_EMPTY_INT(2)
#define USE_EMPTY_INT3()	_USE_EMPTY_INT(3)
#define USE_EMPTY_INT4()	_USE_EMPTY_INT(4)
#define USE_EMPTY_INT5()	_USE_EMPTY_INT(5)
#define USE_EMPTY_INT6()	_USE_EMPTY_INT(6)
#define USE_EMPTY_INT7()	_USE_EMPTY_INT(7)

enum class InterruptTrigger: uint8_t
{
	LOW_LEVEL		= 0x00,
	ANY_CHANGE		= 0x55,
	FALLING_EDGE	= 0xAA,
	RISING_EDGE		= 0xFF
};

template<Board::ExternalInterruptPin PIN>
class INTSignal
{
public:
	INTSignal(InterruptTrigger trigger = InterruptTrigger::ANY_CHANGE)
	{
		_set_trigger(trigger);
	}
	
	inline void set_trigger(InterruptTrigger trigger)
	{
		synchronized set_bit_field(_EICR, _ISC, uint8_t(trigger));
	}
	
	inline void enable()
	{
		synchronized set_mask(_EIMSK, _INT);
	}
	inline void disable()
	{
		synchronized clear_mask(_EIMSK, _INT);
	}
	inline void clear()
	{
		synchronized set_mask(_EIFR, _INTF);
	}

	inline void _set_trigger(InterruptTrigger trigger)
	{
		set_bit_field(_EICR, _ISC, uint8_t(trigger));
	}
	inline void _enable()
	{
		set_mask(_EIMSK, _INT);
	}
	inline void _disable()
	{
		clear_mask(_EIMSK, _INT);
	}
	inline void _clear()
	{
		set_mask(_EIFR, _INTF);
	}
	
private:
	static const constexpr REGISTER	_EICR = Board::EICR_REG(PIN);
	static const constexpr uint8_t	_ISC = Board::EICR_MASK(PIN);
	static const constexpr REGISTER	_EIMSK = Board::EIMSK_REG();
	static const constexpr uint8_t	_INT = Board::EIMSK_MASK(PIN);
	static const constexpr REGISTER	_EIFR = Board::EIFR_REG();
	static const constexpr uint8_t	_INTF = Board::EIFR_MASK(PIN);
};

class INTHandler
{
public:
	virtual bool on_pin_change() = 0;
};

template<Board::ExternalInterruptPin PIN>
class INT: public INTSignal<PIN>
{
public:
	INT(InterruptTrigger trigger = InterruptTrigger::ANY_CHANGE, INTHandler* handler = 0)
		:INTSignal<PIN>{trigger}
	{
		_handler = handler;
	}
	
	inline void set_handler(INTHandler* handler)
	{
		synchronized _handler = handler;
	}

private:
	static INTHandler* _handler;
	static inline void handle()
	{
		if (_handler) _handler->on_pin_change();
	}
	
	friend void INT0_vect();
#if defined(INT1)
	friend void INT1_vect();
#endif
#if defined(INT2)
	friend void INT2_vect();
#endif
#if defined(INT3)
	friend void INT3_vect();
#endif
#if defined(INT4)
	friend void INT4_vect();
#endif
#if defined(INT5)
	friend void INT5_vect();
#endif
#if defined(INT6)
	friend void INT6_vect();
#endif
#if defined(INT7)
	friend void INT7_vect();
#endif
};

template<Board::ExternalInterruptPin INT_NUM>
INTHandler* INT<INT_NUM>::_handler = 0;

#endif	/* INT_HH */

