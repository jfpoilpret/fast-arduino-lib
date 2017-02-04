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
#include "board.h"
#include "board_traits.h"

// Principles:
// One PCI<> instance for each PCINT vector
// Handling, if needed, can be delegated by PCI<> to a ExternalInterruptHandler instance
// Several ExternalInterruptHandler subclasses exist to:
// - handle only one call (most efficient)
// - handle a linked list of handlers
// - support exact changes modes (store port state) of PINs

//TODO IMPROVE static checks by adding PIN as argument to check PIN is consistent with PCINT selected
#define REGISTER_PCI_ISR_METHOD(PCI_NUM, HANDLER, CALLBACK)										\
static_assert(Board::PCI_trait< PCI_NUM >::PORT != Board::Port::NONE, "PORT must support PCI");	\
REGISTER_ISR_METHOD_(CAT3(PCINT, PCI_NUM, _vect), HANDLER, CALLBACK)

#define REGISTER_PCI_ISR_FUNCTION(PCI_NUM, CALLBACK)											\
static_assert(Board::PCI_trait< PCI_NUM >::PORT != Board::Port::NONE, "PORT must support PCI");	\
REGISTER_ISR_FUNCTION_(CAT3(PCINT, PCI_NUM, _vect), CALLBACK)

#define REGISTER_PCI_ISR_EMPTY(PCI_NUM)															\
static_assert(Board::PCI_trait< PCI_NUM >::PORT != Board::Port::NONE, "PORT must support PCI");	\
EMPTY_INTERRUPT(CAT3(PCINT, PCI_NUM, _vect));

template<Board::Port PORT>
class PCISignal
{
public:
	using PORT_TRAIT = Board::Port_trait<PORT>;
	using TRAIT = Board::PCI_trait<PORT_TRAIT::PCINT>;
	
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

template<Board::DigitalPin PIN>
struct PCIType
{
	using TYPE = PCISignal<Board::DigitalPin_trait<PIN>::PORT>;
	static constexpr const uint8_t PCINT = Board::Port_trait<Board::DigitalPin_trait<PIN>::PORT>::PCINT;
};

#endif	/* PCI_HH */
