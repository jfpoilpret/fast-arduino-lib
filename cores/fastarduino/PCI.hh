#ifndef PCI_HH
#define	PCI_HH

#include <avr/interrupt.h>

#include "utilities.hh"
#include "Board.hh"
#include "ExternalInterrupt.hh"
#include "Board_traits.hh"

// Principles:
// One PCI<> instance for each PCINT vector
// Handling, if needed, can be delegated by PCI<> to a ExternalInterruptHandler instance
// Several ExternalInterruptHandler subclasses exist to:
// - handle only one call (most efficient)
// - handle a linked list of handlers
// - support exact changes modes (store port state) of PINs

// These macros are internally used in further macros and should not be used in your programs
#define ALIAS_PCI_(PN, P0)	\
	ISR(PCINT ## PN ## _vect, ISR_ALIASOF(PCINT ## P0 ## _vect));

#define PREPEND_PCI_(PN, ...) Board::PCI_trait< PN >::PORT
#define ASSERT_PCI_(PN, ...) static_assert(Board::PCI_trait< PN >::PORT != Board::Port::NONE, "PORT must support PCI");

#define USE_PCIS(P0, ...)																						\
ISR(PCINT ## P0 ## _vect)																						\
{																												\
	FOR_EACH(ASSERT_PCI_, , ##__VA_ARGS__)																		\
	PCI_impl::InterruptHandler<FOR_EACH_SEP(PREPEND_PCI_, , EMPTY, COMMA, EMPTY, P0, ##__VA_ARGS__)>::handle();	\
}																												\
																												\
FOR_EACH(ALIAS_PCI_, P0, ##__VA_ARGS__)

#define EMPTY_PCI_(PN, _0)					\
EMPTY_INTERRUPT(PCINT ## PN ## _vect);

#define USE_EMPTY_PCIS(P0, ...)				\
FOR_EACH(EMPTY_PCI_, _0, P0, ##__VA_ARGS__)


template<Board::Port PORT>
class PCISignal
{
public:
	using TRAIT = Board::Port_trait<PORT>;
	
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
};

// Forward declaration necessary to be declared as friend
// Complete declaration can be found at the end of this file
namespace PCI_impl
{
	template<Board::Port P0, Board::Port... PS> struct InterruptHandler;
}

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
	
	static void handle_if_needed()
	{
		if (((volatile uint8_t&) PCISignal<PORT>::TRAIT::PCIFR_) & PCISignal<PORT>::TRAIT::PCIFR_MASK)
			_handler->on_pin_change();
	}
	
	template<Board::Port, Board::Port...> friend struct PCI_impl::InterruptHandler;
};

template<Board::Port PORT>
ExternalInterruptHandler* PCI<PORT>::_handler = 0;

template<Board::DigitalPin PIN>
struct PCIType
{
	using TYPE = PCI<Board::DigitalPin_trait<PIN>::PORT>;
	static constexpr const uint8_t PCINT = Board::Port_trait<Board::DigitalPin_trait<PIN>::PORT>::PCINT;
};

namespace PCI_impl
{
	template<Board::Port P0, Board::Port... PS>
	struct InterruptHandler
	{
		static void handle()
		{
			InterruptHandler<P0>::handle();
			InterruptHandler<PS...>::handle();
		}
	};

	template<Board::Port P>
	struct InterruptHandler<P>
	{
		static void handle()
		{
			PCI<P>::handle_if_needed();
		}
	};
}

#endif	/* PCI_HH */
