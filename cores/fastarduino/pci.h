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

#ifndef PCI_HH
#define	PCI_HH

#include <avr/interrupt.h>

#include "utilities.h"
#include "boards/board.h"
#include "boards/board_traits.h"

// Principles:
// One PCI<> instance for each PCINT vector
// Handling, if needed, can be delegated by PCI<> to a ExternalInterruptHandler instance
// Several ExternalInterruptHandler subclasses exist to:
// - handle only one call (most efficient)
// - handle a linked list of handlers
// - support exact changes modes (store port state) of PINs

#define CHECK_PCI_PIN_(PIN, PCI_NUM)																			\
static_assert(board_traits::PCI_trait< PCI_NUM >::PORT != Board::Port::NONE, "PORT must support PCI");			\
static_assert(board_traits::DigitalPin_trait< PIN >::PORT == board_traits::PCI_trait< PCI_NUM >::PORT,			\
	"PIN port must match PCI_NUM port");																		\
static_assert(_BV(board_traits::DigitalPin_trait< PIN >::BIT) & board_traits::PCI_trait< PCI_NUM >::PCI_MASK,	\
	"PIN must be a PCINT pin");

#define REGISTER_PCI_ISR_METHOD(PCI_NUM, HANDLER, CALLBACK, PIN, ...)			\
FOR_EACH(CHECK_PCI_PIN_, PCI_NUM, PIN, ##__VA_ARGS__)							\
REGISTER_ISR_METHOD_(CAT3(PCINT, PCI_NUM, _vect), HANDLER, CALLBACK)

#define REGISTER_PCI_ISR_FUNCTION(PCI_NUM, CALLBACK, PIN, ...)					\
FOR_EACH(CHECK_PCI_PIN_, PCI_NUM, PIN, ##__VA_ARGS__)							\
REGISTER_ISR_FUNCTION_(CAT3(PCINT, PCI_NUM, _vect), CALLBACK)

#define REGISTER_PCI_ISR_EMPTY(PCI_NUM, PIN, ...)								\
FOR_EACH(CHECK_PCI_PIN_, PCI_NUM, PIN, ##__VA_ARGS__)							\
EMPTY_INTERRUPT(CAT3(PCINT, PCI_NUM, _vect))

template<Board::Port PORT>
class PCISignal
{
public:
	using PORT_TRAIT = board_traits::Port_trait<PORT>;
	using TRAIT = board_traits::PCI_trait<PORT_TRAIT::PCINT>;
	
	inline void enable()
	{
		synchronized TRAIT::PCICR_ |= TRAIT::PCICR_MASK;
	}
	inline void disable()
	{
		synchronized TRAIT::PCICR_ &= ~TRAIT::PCICR_MASK;
	}
	inline void clear()
	{
		synchronized TRAIT::PCIFR_ |= TRAIT::PCIFR_MASK;
	}
	inline void enable_pins(uint8_t mask)
	{
		synchronized TRAIT::PCMSK_ |= mask;
	}
	template<Board::DigitalPin PIN>
	inline void enable_pin()
	{
		static_assert(board_traits::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
		static_assert(TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<PIN>::BIT), "PIN must be a PCI within PORT");
		enable_pins(_BV(board_traits::DigitalPin_trait<PIN>::BIT));
	}
	template<Board::DigitalPin PIN>
	inline void disable_pin()
	{
		static_assert(board_traits::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
		static_assert(TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<PIN>::BIT), "PIN must be a PCI within PORT");
		synchronized TRAIT::PCMSK_ &= ~_BV(board_traits::DigitalPin_trait<PIN>::BIT);
	}
	
	inline void _enable()
	{
		TRAIT::PCICR_ |= TRAIT::PCICR_MASK;
	}
	inline void _disable()
	{
		TRAIT::PCICR_ &= ~TRAIT::PCICR_MASK;
	}
	inline void _clear()
	{
		TRAIT::PCIFR_ |= TRAIT::PCIFR_MASK;
	}
	inline void _enable_pins(uint8_t mask)
	{
		TRAIT::PCMSK_ |= mask;
	}
	template<Board::DigitalPin PIN>
	inline void _enable_pin()
	{
		static_assert(board_traits::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
		static_assert(TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<PIN>::BIT), "PIN must be a PCI within PORT");
		_enable_pins(_BV(board_traits::DigitalPin_trait<PIN>::BIT));
	}
	template<Board::DigitalPin PIN>
	inline void _disable_pin()
	{
		static_assert(board_traits::DigitalPin_trait<PIN>::PORT == PORT, "PIN must be within PORT");
		static_assert(TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<PIN>::BIT), "PIN must be a PCI within PORT");
		TRAIT::PCMSK_ &= ~_BV(board_traits::DigitalPin_trait<PIN>::BIT);
	}
};

template<Board::DigitalPin PIN>
struct PCIType
{
	using TYPE = PCISignal<board_traits::DigitalPin_trait<PIN>::PORT>;
	static constexpr const uint8_t PCINT = board_traits::Port_trait<board_traits::DigitalPin_trait<PIN>::PORT>::PCINT;
};

#endif	/* PCI_HH */
