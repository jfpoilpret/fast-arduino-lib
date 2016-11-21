#ifndef PCI_HH
#define	PCI_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board.hh"
#include "ExternalInterrupt.hh"
#include "boards/Uno_traits.hh"

// Principles:
// One PCI<> instance for each PCINT vector
// Handling is delegared by PCI<> to a PCIHandler instance
// Several PCIHandler subclasses exist to:
// - handle only one call (most efficient)
// - handle a linked list of handlers
// - support exact changes modes (store port state) of PINs

// This macro is internally used in further macros and should not be used in your programs
#define _USE_PCI(INT)										\
ISR(PCINT ## INT ## _vect)									\
{															\
	using TRAIT = Board::PCI_trait<INT>;					\
	static_assert(TRAIT::PORT != Board::Port::NONE, "");	\
	PCI<TRAIT::PORT>::handle();								\
}

// This macro is internally used in further macros and should not be used in your programs
#define _USE_EMPTY_PCI(INT)					\
EMPTY_INTERRUPT(PCINT ## INT ## _vect)

//TODO INFER on a way to find out which vector should used when you only know a DigitalPin...

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp, or in main.cpp) 
// to indicate you want to use PCI for a given PORT in your program, hence you need the proper 
// ISR vector correctly defined
#define USE_PCI0()	_USE_PCI(0)
#define USE_PCI1()	_USE_PCI(1)
#define USE_PCI2()	_USE_PCI(2)
#define USE_PCI3()	_USE_PCI(3)

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use PCISignal for a given PCI port in your program, hence you need the proper ISR vector correctly defined
// Use these when you want an PCI to wake up from sleep but you don't need any handler to be executed
#define USE_EMPTY_PCI0()	_USE_EMPTY_PCI(0)
#define USE_EMPTY_PCI1()	_USE_EMPTY_PCI(1)
#define USE_EMPTY_PCI2()	_USE_EMPTY_PCI(2)
#define USE_EMPTY_PCI3()	_USE_EMPTY_PCI(3)

template<Board::Port PORT>
class PCISignal
{
private:
	using TRAIT = Board::Port_trait<PORT>;
	
public:
	inline void enable()
	{
		synchronized set_mask(TRAIT::PCICR_, TRAIT::PCICR_MASK);
	}
	inline void disable()
	{
		synchronized clear_mask(TRAIT::PCICR_, TRAIT::PCICR_MASK);
	}
	inline void clear()
	{
		synchronized set_mask(TRAIT::PCIFR_, TRAIT::PCIFR_MASK);
	}
	inline void enable_pins(uint8_t mask)
	{
		synchronized set_mask(TRAIT::PCMSK_, mask);
	}
	//TODO REWORK with TRAIT
	template<Board::DigitalPin PIN>
	inline void enable_pin()
	{
		static_assert(Board::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
		static_assert(TRAIT::PCI_MASK & _BV(Board::DigitalPin_trait<PIN>::BIT), "PIN must be a PCI within PORT");
		enable_pins(_BV(Board::DigitalPin_trait<PIN>::BIT));
	}
	template<Board::DigitalPin PIN>
	inline void disable_pin()
	{
		static_assert(Board::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
		static_assert(TRAIT::PCI_MASK & _BV(Board::DigitalPin_trait<PIN>::BIT), "PIN must be a PCI within PORT");
		synchronized clear_mask(TRAIT::PCMSK_, _BV(Board::DigitalPin_trait<PIN>::BIT));
	}
//	inline void enable_pin(Board::DigitalPin pin)
//	{
//		enable_pins(_BV(Board::BIT(pin)));
//	}
//	inline void disable_pin(Board::DigitalPin  pin)
//	{
//		const uint8_t mask = _BV(Board::BIT(pin));
//		synchronized clear_mask(TRAIT::PCMSK_, mask);
//	}
	
	inline void _enable()
	{
		set_mask(TRAIT::PCICR_, TRAIT::PCICR_MASK);
	}
	inline void _disable()
	{
		clear_mask(TRAIT::PCICR_, TRAIT::PCICR_MASK);
	}
	inline void _clear()
	{
		set_mask(TRAIT::PCIFR_, TRAIT::PCIFR_MASK);
	}
	inline void _enable_pins(uint8_t mask)
	{
		set_mask(TRAIT::PCMSK_, mask);
	}
	template<Board::DigitalPin PIN>
	inline void _enable_pin()
	{
		static_assert(Board::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
		static_assert(TRAIT::PCI_MASK & _BV(Board::DigitalPin_trait<PIN>::BIT), "PIN must be a PCI within PORT");
		_enable_pins(_BV(Board::DigitalPin_trait<PIN>::BIT));
	}
	template<Board::DigitalPin PIN>
	inline void _disable_pin()
	{
		static_assert(Board::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
		static_assert(TRAIT::PCI_MASK & _BV(Board::DigitalPin_trait<PIN>::BIT), "PIN must be a PCI within PORT");
		clear_mask(TRAIT::PCMSK_, _BV(Board::DigitalPin_trait<PIN>::BIT));
	}
//	inline void _enable_pin(Board::DigitalPin pin)
//	{
//		_enable_pins(_BV(Board::BIT(pin)));
//	}
//	inline void _disable_pin(Board::DigitalPin pin)
//	{
//		const uint8_t mask = _BV(Board::BIT(pin));
//		clear_mask(TRAIT::PCMSK_, mask);
//	}
};

template<Board::Port PORT>
class PCI: public PCISignal<PORT>
{
public:
	PCI(ExternalInterruptHandler* handler = 0) INLINE
	{
		_handler = handler;
	}
	
	inline void set_handler(ExternalInterruptHandler* handler) INLINE
	{
		_handler = handler;
	}
	
private:
	static ExternalInterruptHandler* _handler;
	static inline void handle()
	{
		if (_handler) _handler->on_pin_change();
	}
	
	friend void PCINT0_vect();
#if defined(PCIE1)
	friend void PCINT1_vect();
#endif
#if defined(PCIE2)
	friend void PCINT2_vect();
#endif
#if defined(PCIE3)
	friend void PCINT3_vect();
#endif
};

template<Board::Port PORT>
ExternalInterruptHandler* PCI<PORT>::_handler = 0;

template<Board::DigitalPin PIN>
struct PCIType
{
	using TYPE = PCI<Board::DigitalPin_trait<PIN>::PORT>;
	static constexpr const uint8_t PCINT = Board::Port_trait<Board::DigitalPin_trait<PIN>::PORT>::PCINT;
};

#endif	/* PCI_HH */
