#ifndef PCI_HH
#define	PCI_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board.hh"

//TODO Abandon the idea of doing without virtual, that complicates things too much.
// Just limit usage of virtual to the minimum, ie event/interrupt handlers

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
	PCI<Board::PCIPort::PCI ## PORT>::_handler->pin_change();	\
}

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use PCI for a given PORT in your program, hence you need the proper ISR vector correctly defined
#define USE_PCI0()	_USE_PCI(0)
#define USE_PCI1()	_USE_PCI(1)
#define USE_PCI2()	_USE_PCI(2)

#define _FRIEND_PCI_VECT(PORT) friend void PCINT ## PORT ## _vect();

class PCIHandler;

template<Board::PCIPort PORT>
class PCI
{
public:
	PCI(PCIHandler* handler = 0)
	{
		_handler = handler;
	}
	
	inline void set_handler(PCIHandler* handler)
	{
		_handler = handler;
	}
	
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
		enable_pins(_BV(Board::BIT((uint8_t) pin)));
	}
	inline void disable_pin(Board::InterruptPin pin)
	{
		const uint8_t mask = _BV(Board::BIT((uint8_t) pin));
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
		_enable_pins(_BV(Board::BIT((uint8_t) pin)));
	}
	inline void _disable_pin(Board::InterruptPin pin)
	{
		const uint8_t mask = _BV(Board::BIT((uint8_t) pin));
		clear_mask(_PCMSK, mask);
	}
	
private:
	static const constexpr REGISTER	_PCICR = Board::PCICR_REG();
	static const constexpr uint8_t	_PCIE = Board::PCIE_MSK(PORT);
	static const constexpr REGISTER	_PCIFR = Board::PCIFR_REG();
	static const constexpr uint8_t	_PCIF = Board::PCIFR_MSK(PORT);
	static const constexpr REGISTER _PCMSK = Board::PCMSK_REG(PORT);
	
	static PCIHandler* _handler;
	
	_FRIEND_PCI_VECT(0);
#if defined(PCIE1)
	_FRIEND_PCI_VECT(1);
#endif
#if defined(PCIE2)
	_FRIEND_PCI_VECT(2);
#endif
#if defined(PCIE3)
	_FRIEND_PCI_VECT(3);
#endif
};

template<Board::PCIPort PORT>
PCIHandler* PCI<PORT>::_handler = 0;

class PCIHandler
{
public:
	virtual bool pin_change() = 0;
};

//TODO More functional subclasses to:
// - allow detection of PCI mode (RAISE, LOWER...)
// - allow chaining PCI handling to several handlers
//TODO this is used when we have different handlers for the same port

#endif	/* PCI_HH */

