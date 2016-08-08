#ifndef PCI_HH
#define	PCI_HH

#include <avr/interrupt.h>

#include "Board.hh"

// Principles:
// One PCI<> instance for each PCINT vector
// Handling is delegared by PCI<> to a AbstractPCIHandler instance
// Several AbstractPCIHandler subclasses exist to:
// - handle only one call (most efficient)
// - handle a linked list of handlers
// - support exact changes modes (store port state) of PINs

// This macro is internally used in further macros and should not be used in your programs
#define _USE_PCI(PORT)									\
ISR(PCINT ## PORT ## _vect)								\
{														\
	PCI<Board::PCIPort::PCI ## PORT>::_pci->handle();	\
}

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// want to use PCI for a given PORT in your program, hence you need the proper ISR vector correctly defined
#define USE_PCI0()	_USE_PCI(0)
#define USE_PCI1()	_USE_PCI(1)
#define USE_PCI2()	_USE_PCI(2)

#define _FRIEND_PCI_VECT(PORT) friend void PCINT ## PORT ## _vect();

class AbstractPCI
{
protected:
	void _enable_all(volatile uint8_t& pcicr, uint8_t port);
	void _disable_all(volatile uint8_t& pcicr, uint8_t port);
	void _clear(volatile uint8_t& pcifr, uint8_t port);
	
	void _enable(volatile uint8_t& pcmsk, uint8_t mask);
	
	void _enable(volatile uint8_t& pcmsk, Board::InterruptPin pin);
	void _disable(volatile uint8_t& pcmsk, Board::InterruptPin pin);
};

class AbstractPCIHandler
{
public:
	void handle() __attribute__((always_inline))
	{
		_f(_env);
	}

private:
	//TODO refactor to make it used everywhere we need this pattern!
	typedef void (*F)(void* env);
	AbstractPCIHandler(void* env = 0, F f = 0) __attribute__((always_inline))
		:_f{f}, _env{env} {}

	F _f;
	void* _env;
	
	friend class VirtualPCIHandler;
	template<typename FUNCTOR> friend class FunctorPCIHandler;
};

template<Board::PCIPort PORT>
class PCI:public AbstractPCI
{
public:
	PCI(AbstractPCIHandler& handler):_handler(handler)
	{
		_pci = this;
	}
	
	void enable()
	{
		_enable_all(_PCICR, _PCIE);
	}
	void disable()
	{
		_disable_all(_PCICR, _PCIE);
	}
	void clear()
	{
		_clear(_PCIFR, _PCIF);
	}
	void enable(uint8_t mask)
	{
		_enable(_PCMSK, mask);
	}
	void enable(Board::InterruptPin pin)
	{
		_enable(_PCMSK, pin);
	}
	void disable(Board::InterruptPin pin)
	{
		_disable(_PCMSK, pin);
	}
	
private:
	void handle()
	{
		_handler.handle();
	}
	
	static const constexpr REGISTER	_PCICR = Board::PCICR_REG();
	static const constexpr uint8_t	_PCIE = Board::PCIE_MSK(PORT);
	static const constexpr REGISTER	_PCIFR = Board::PCIFR_REG();
	static const constexpr uint8_t	_PCIF = Board::PCIFR_MSK(PORT);
	static const constexpr REGISTER _PCMSK = Board::PCMSK_REG(PORT);
	
	static PCI<PORT>* _pci;
	
	AbstractPCIHandler& _handler;
	
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
PCI<PORT>* PCI<PORT>::_pci = 0;

class VirtualPCIHandler: public AbstractPCIHandler
{
protected:
	VirtualPCIHandler() __attribute__((always_inline))
		: AbstractPCIHandler{this, apply} {}
	virtual void execute() = 0;

private:
	static void apply(void* env) __attribute__((always_inline))
	{
		((VirtualPCIHandler*) env)->execute();
	}
};
	
// Instantiate this template with a Functor when a functor is applicable.
// FUNCTOR must be a class defining:
// void operator()();
// This approach generally gives smaller code and data than VirtualPCIHandler approach
template<typename FUNCTOR>
class FunctorPCIHandler: public AbstractPCIHandler
{
public:
	FunctorPCIHandler() __attribute__((always_inline)) : AbstractPCIHandler{} {}
	FunctorPCIHandler(FUNCTOR f) __attribute__((always_inline))
		: AbstractPCIHandler{this, apply}, _f{f} {}
private:
	static void apply(void* env) __attribute__((always_inline))
	{
		((FunctorPCIHandler<FUNCTOR>*) env)->_f();
	}
	FUNCTOR _f;
};

//TODO More functional subclasses to:
// - allow detection of PCI mode (RAISE, LOWER...)
// - allow chaining PCI handling to several handlers
//TODO this is used when we have different handlers for the same port

#endif	/* PCI_HH */

