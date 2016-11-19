#ifndef PCI_HH
#define	PCI_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board.hh"
#include "ExternalInterrupt.hh"

// Principles:
// One PCI<> instance for each PCINT vector
// Handling is delegared by PCI<> to a PCIHandler instance
// Several PCIHandler subclasses exist to:
// - handle only one call (most efficient)
// - handle a linked list of handlers
// - support exact changes modes (store port state) of PINs

// This macro is internally used in further macros and should not be used in your programs
#define _USE_PCI(PORT)											\
ISR(PCINT ## PORT ## _vect)										\
{																\
	PCI<Board::PCIPort::PCI ## PORT>::handle();\
}

// This macro is internally used in further macros and should not be used in your programs
#define _USE_EMPTY_PCI(PORT)										\
EMPTY_INTERRUPT(PCINT ## PORT ## _vect)

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

template<Board::PCIPort PORT>
class PCISignal
{
public:
	inline void enable()
	{
		synchronized set_mask(_PCICR, _PCIE);
	}
	inline void disable()
	{
		synchronized clear_mask(_PCICR, _PCIE);
	}
	inline void clear()
	{
		synchronized set_mask(_PCIFR, _PCIF);
	}
	inline void enable_pins(uint8_t mask)
	{
		synchronized set_mask(_PCMSK, mask);
	}
	inline void enable_pin(Board::InterruptPin pin)
	{
		enable_pins(_BV(Board::BIT(pin)));
	}
	inline void disable_pin(Board::InterruptPin pin)
	{
		const uint8_t mask = _BV(Board::BIT(pin));
		synchronized clear_mask(_PCMSK, mask);
	}
	
	inline void _enable()
	{
		set_mask(_PCICR, _PCIE);
	}
	inline void _disable()
	{
		clear_mask(_PCICR, _PCIE);
	}
	inline void _clear()
	{
		set_mask(_PCIFR, _PCIF);
	}
	inline void _enable_pins(uint8_t mask)
	{
		set_mask(_PCMSK, mask);
	}
	inline void _enable_pin(Board::InterruptPin pin)
	{
		_enable_pins(_BV(Board::BIT(pin)));
	}
	inline void _disable_pin(Board::InterruptPin pin)
	{
		const uint8_t mask = _BV(Board::BIT(pin));
		clear_mask(_PCMSK, mask);
	}
	
private:
	static const constexpr REGISTER	_PCICR = Board::PCICR_REG();
	static const constexpr uint8_t	_PCIE = Board::PCIE_MSK(PORT);
	static const constexpr REGISTER	_PCIFR = Board::PCIFR_REG();
	static const constexpr uint8_t	_PCIF = Board::PCIFR_MSK(PORT);
	static const constexpr REGISTER _PCMSK = Board::PCMSK_REG(PORT);
};

template<Board::PCIPort PORT>
class PCI: public PCISignal<PORT>
{
public:
	PCI(ExternalInterruptHandler* handler = 0)
	{
		_handler = handler;
	}
	
	inline void set_handler(ExternalInterruptHandler* handler)
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

template<Board::PCIPort PORT>
ExternalInterruptHandler* PCI<PORT>::_handler = 0;

#endif	/* PCI_HH */

