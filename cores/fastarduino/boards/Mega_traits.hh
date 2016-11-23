#ifndef BOARDS_MEGA_TRAITS_HH
#define BOARDS_MEGA_TRAITS_HH

#include <avr/io.h>
#include "Mega.hh"

namespace Board
{
	//====
	// IO
	//====
	template<Port P>
	struct Port_trait
	{
		static constexpr const REGISTER PIN{};
		static constexpr const REGISTER DDR{};
		static constexpr const REGISTER PORT{};
		static constexpr const uint8_t DPIN_MASK = 0x00;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
	template<>
	struct Port_trait<Port::PORT_A>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINA);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRA);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTA);
		static constexpr const uint8_t DPIN_MASK = 0xFF;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
//	PCI0 = 0,			// PB0-7
	template<>
	struct Port_trait<Port::PORT_B>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINB);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRB);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTB);
		static constexpr const uint8_t DPIN_MASK = 0xFF;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0xFF;
		static constexpr const uint8_t PCICR_MASK = _BV(PCIE0); 
		static constexpr const uint8_t PCIFR_MASK = _BV(PCIF0);
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_ = _SELECT_REG(PCMSK0);
	};
	
	template<>
	struct Port_trait<Port::PORT_C>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINC);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRC);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTC);
		static constexpr const uint8_t DPIN_MASK = 0xFF;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
	template<>
	struct Port_trait<Port::PORT_D>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PIND);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRD);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTD);
		static constexpr const uint8_t DPIN_MASK = 0x8F;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
	template<>
	struct Port_trait<Port::PORT_E>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINE);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRE);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTE);
		static constexpr const uint8_t DPIN_MASK = 0x3B;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
	template<>
	struct Port_trait<Port::PORT_F>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINF);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRF);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTF);
		static constexpr const uint8_t DPIN_MASK = 0xFF;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
	template<>
	struct Port_trait<Port::PORT_G>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PING);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRG);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTG);
		static constexpr const uint8_t DPIN_MASK = 0x27;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
	template<>
	struct Port_trait<Port::PORT_H>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINH);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRH);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTH);
		static constexpr const uint8_t DPIN_MASK = 0x7B;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
//	PCI1 = 1,			// PJ0-1
	template<>
	struct Port_trait<Port::PORT_J>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINJ);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRJ);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTJ);
		static constexpr const uint8_t DPIN_MASK = 0x03;

		static constexpr const uint8_t PCINT = 1;
		static constexpr const uint8_t PCI_MASK = 0x03;
		static constexpr const uint8_t PCICR_MASK = _BV(PCIE1); 
		static constexpr const uint8_t PCIFR_MASK = _BV(PCIF1);
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_ = _SELECT_REG(PCMSK1);
	};
	
//	PCI2 = 2			// PK0-7
	template<>
	struct Port_trait<Port::PORT_K>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINK);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRK);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTK);
		static constexpr const uint8_t DPIN_MASK = 0xFF;

		static constexpr const uint8_t PCINT = 2;
		static constexpr const uint8_t PCI_MASK = 0xFF;
		static constexpr const uint8_t PCICR_MASK = _BV(PCIE2); 
		static constexpr const uint8_t PCIFR_MASK = _BV(PCIF2);
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_ = _SELECT_REG(PCMSK2);
	};
	
	template<>
	struct Port_trait<Port::PORT_L>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINL);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRL);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTL);
		static constexpr const uint8_t DPIN_MASK = 0xFF;

		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
	/**
	 * Digital pin symbols
	 */
	template<DigitalPin DPIN>
	struct DigitalPin_trait
	{
		static constexpr const Port PORT = Port::NONE;
		static constexpr const uint8_t BIT = 0;
		static constexpr const bool IS_INT = false;
	};
	template<Port P, uint8_t B, bool INT = false>
	struct DigitalPin_trait_impl
	{
		static constexpr const Port PORT = P;
		static constexpr const uint8_t BIT = B;
		static constexpr const bool IS_INT = INT;
	};

	template<> struct DigitalPin_trait<DigitalPin::NONE>: public DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D22>: public DigitalPin_trait_impl<Port::PORT_A, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D23>: public DigitalPin_trait_impl<Port::PORT_A, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D24>: public DigitalPin_trait_impl<Port::PORT_A, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D25>: public DigitalPin_trait_impl<Port::PORT_A, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D26>: public DigitalPin_trait_impl<Port::PORT_A, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D27>: public DigitalPin_trait_impl<Port::PORT_A, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D28>: public DigitalPin_trait_impl<Port::PORT_A, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D29>: public DigitalPin_trait_impl<Port::PORT_A, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D53>: public DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D52>: public DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D51>: public DigitalPin_trait_impl<Port::PORT_B, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D50>: public DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D10>: public DigitalPin_trait_impl<Port::PORT_B, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D11>: public DigitalPin_trait_impl<Port::PORT_B, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D12>: public DigitalPin_trait_impl<Port::PORT_B, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D13>: public DigitalPin_trait_impl<Port::PORT_B, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D37>: public DigitalPin_trait_impl<Port::PORT_C, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D36>: public DigitalPin_trait_impl<Port::PORT_C, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D35>: public DigitalPin_trait_impl<Port::PORT_C, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D34>: public DigitalPin_trait_impl<Port::PORT_C, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D33>: public DigitalPin_trait_impl<Port::PORT_C, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D32>: public DigitalPin_trait_impl<Port::PORT_C, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D31>: public DigitalPin_trait_impl<Port::PORT_C, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D30>: public DigitalPin_trait_impl<Port::PORT_C, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D21>: public DigitalPin_trait_impl<Port::PORT_D, 0, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D20>: public DigitalPin_trait_impl<Port::PORT_D, 1, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D19>: public DigitalPin_trait_impl<Port::PORT_D, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D18>: public DigitalPin_trait_impl<Port::PORT_D, 3, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D38>: public DigitalPin_trait_impl<Port::PORT_D, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D0>: public DigitalPin_trait_impl<Port::PORT_E, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1>: public DigitalPin_trait_impl<Port::PORT_E, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D5>: public DigitalPin_trait_impl<Port::PORT_E, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D2>: public DigitalPin_trait_impl<Port::PORT_E, 4, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D3>: public DigitalPin_trait_impl<Port::PORT_E, 5, true> {};

	template<> struct DigitalPin_trait<DigitalPin::D54>: public DigitalPin_trait_impl<Port::PORT_F, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D55>: public DigitalPin_trait_impl<Port::PORT_F, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D56>: public DigitalPin_trait_impl<Port::PORT_F, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D57>: public DigitalPin_trait_impl<Port::PORT_F, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D58>: public DigitalPin_trait_impl<Port::PORT_F, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D59>: public DigitalPin_trait_impl<Port::PORT_F, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D60>: public DigitalPin_trait_impl<Port::PORT_F, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D61>: public DigitalPin_trait_impl<Port::PORT_F, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D41>: public DigitalPin_trait_impl<Port::PORT_G, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D40>: public DigitalPin_trait_impl<Port::PORT_G, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D39>: public DigitalPin_trait_impl<Port::PORT_G, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D4>: public DigitalPin_trait_impl<Port::PORT_G, 5> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D17>: public DigitalPin_trait_impl<Port::PORT_H, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D16>: public DigitalPin_trait_impl<Port::PORT_H, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D6>: public DigitalPin_trait_impl<Port::PORT_H, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D7>: public DigitalPin_trait_impl<Port::PORT_H, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D8>: public DigitalPin_trait_impl<Port::PORT_H, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D9>: public DigitalPin_trait_impl<Port::PORT_H, 6> {};

	template<> struct DigitalPin_trait<DigitalPin::D15>: public DigitalPin_trait_impl<Port::PORT_J, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D14>: public DigitalPin_trait_impl<Port::PORT_J, 1> {};

	template<> struct DigitalPin_trait<DigitalPin::D62>: public DigitalPin_trait_impl<Port::PORT_K, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D63>: public DigitalPin_trait_impl<Port::PORT_K, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D64>: public DigitalPin_trait_impl<Port::PORT_K, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D65>: public DigitalPin_trait_impl<Port::PORT_K, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D66>: public DigitalPin_trait_impl<Port::PORT_K, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D67>: public DigitalPin_trait_impl<Port::PORT_K, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D68>: public DigitalPin_trait_impl<Port::PORT_K, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D69>: public DigitalPin_trait_impl<Port::PORT_K, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D49>: public DigitalPin_trait_impl<Port::PORT_L, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D48>: public DigitalPin_trait_impl<Port::PORT_L, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D47>: public DigitalPin_trait_impl<Port::PORT_L, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D46>: public DigitalPin_trait_impl<Port::PORT_L, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D45>: public DigitalPin_trait_impl<Port::PORT_L, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D44>: public DigitalPin_trait_impl<Port::PORT_L, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D43>: public DigitalPin_trait_impl<Port::PORT_L, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D42>: public DigitalPin_trait_impl<Port::PORT_L, 7> {};

#define _SELECT_PORT(PORT, ARG_A, ARG_B, ARG_C, ARG_D, ARG_E, ARG_F, ARG_G, ARG_H, ARG_J, ARG_K, ARG_L)	\
	(	PORT == Port::PORT_A ? ARG_A :			\
		PORT == Port::PORT_B ? ARG_B :			\
		PORT == Port::PORT_C ? ARG_C :			\
		PORT == Port::PORT_D ? ARG_D :			\
		PORT == Port::PORT_E ? ARG_E :			\
		PORT == Port::PORT_F ? ARG_F :			\
		PORT == Port::PORT_G ? ARG_G :			\
		PORT == Port::PORT_H ? ARG_H :			\
		PORT == Port::PORT_J ? ARG_J :			\
		PORT == Port::PORT_K ? ARG_K :			\
		ARG_L)

#define _SELECT_PIN(DPIN, ARG_E, ARG_H, ARG_B, ARG_A, ARG_C, ARG_D, ARG_L, ARG_F, ARG_K, ARG_J, ARG_G)	\
	(	(uint8_t) DPIN < 8 ? ARG_E :		\
		(uint8_t) DPIN < 16 ? ARG_H :		\
		(uint8_t) DPIN < 24 ? ARG_B :		\
		(uint8_t) DPIN < 32 ? ARG_A :		\
		(uint8_t) DPIN < 40 ? ARG_C :		\
		(uint8_t) DPIN < 48 ? ARG_D :		\
		(uint8_t) DPIN < 56 ? ARG_L :		\
		(uint8_t) DPIN < 64 ? ARG_F :		\
		(uint8_t) DPIN < 72 ? ARG_K :		\
		(uint8_t) DPIN < 80 ? ARG_J :		\
		ARG_G)
	
//TODO Could that be simplified by using REG ## E, REG## H, ...?
#define _SELECT_PIN_REG(DPIN, REG_E, REG_H, REG_B, REG_A, REG_C, REG_D, REG_L, REG_F, REG_K, REG_J, REG_G)	\
	_SELECT_REG(_SELECT_PIN(DPIN, REG_E, REG_H, REG_B, REG_A, REG_C, REG_D, REG_L, REG_F, REG_K, REG_J, REG_G))

	constexpr REGISTER PIN_REG(Port port)
	{
		return _SELECT_REG(_SELECT_PORT(port, PINA, PINB, PINC, PIND, PINE, PINF, PING, PINH, PINJ, PINK, PINL));
	}
	
	constexpr Port PORT(DigitalPin pin)
	{
		return _SELECT_PIN(pin, Port::PORT_E, Port::PORT_H, Port::PORT_B, Port::PORT_A, Port::PORT_C, 
			Port::PORT_D, Port::PORT_L, Port::PORT_F, Port::PORT_K, Port::PORT_J, Port::PORT_G);
	}

	constexpr REGISTER PIN_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, PINE, PINH, PINB, PINA, PINC, PIND, PINL, PINF, PINK, PINJ, PING);
	}

	constexpr REGISTER DDR_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, DDRE, DDRH, DDRB, DDRA, DDRC, DDRD, DDRL, DDRF, DDRK, DDRJ, DDRG);
	}

	constexpr REGISTER PORT_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, PORTE, PORTH, PORTB, PORTA, PORTC, PORTD, PORTL, PORTF, PORTK, PORTJ, PORTG);
	}

	constexpr uint8_t BIT(DigitalPin pin)
	{
		return ((uint8_t) pin) & 0x7;
	}

	//===============
	// IO interrupts
	//===============
	
	template<DigitalPin DPIN>
	struct ExternalInterruptPin_trait
	{
		static constexpr const REGISTER EICR_{};
		static constexpr const uint8_t EICR_MASK = 0x00;
		static constexpr const REGISTER EIMSK_{};
		static constexpr const uint8_t EIMSK_MASK = 0x00;
		static constexpr const REGISTER EIFR_{};
		static constexpr const uint8_t EIFR_MASK = 0x00;
	};

	template<>
	struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT0>
	{
		static constexpr const REGISTER EICR_ = _SELECT_REG(EICRA);
		static constexpr const uint8_t EICR_MASK = _BV(ISC00) | _BV(ISC01);
		static constexpr const REGISTER EIMSK_ = _SELECT_REG(EIMSK);
		static constexpr const uint8_t EIMSK_MASK = _BV(INT0);
		static constexpr const REGISTER EIFR_ = _SELECT_REG(EIFR);
		static constexpr const uint8_t EIFR_MASK = _BV(INTF0);
	};

	template<>
	struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT1>
	{
		static constexpr const REGISTER EICR_ = _SELECT_REG(EICRA);
		static constexpr const uint8_t EICR_MASK = _BV(ISC10) | _BV(ISC11);
		static constexpr const REGISTER EIMSK_ = _SELECT_REG(EIMSK);
		static constexpr const uint8_t EIMSK_MASK = _BV(INT1);
		static constexpr const REGISTER EIFR_ = _SELECT_REG(EIFR);
		static constexpr const uint8_t EIFR_MASK = _BV(INTF1);
	};

	template<>
	struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT2>
	{
		static constexpr const REGISTER EICR_ = _SELECT_REG(EICRA);
		static constexpr const uint8_t EICR_MASK = _BV(ISC20) | _BV(ISC21);
		static constexpr const REGISTER EIMSK_ = _SELECT_REG(EIMSK);
		static constexpr const uint8_t EIMSK_MASK = _BV(INT2);
		static constexpr const REGISTER EIFR_ = _SELECT_REG(EIFR);
		static constexpr const uint8_t EIFR_MASK = _BV(INTF2);
	};

	template<>
	struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT3>
	{
		static constexpr const REGISTER EICR_ = _SELECT_REG(EICRA);
		static constexpr const uint8_t EICR_MASK = _BV(ISC30) | _BV(ISC31);
		static constexpr const REGISTER EIMSK_ = _SELECT_REG(EIMSK);
		static constexpr const uint8_t EIMSK_MASK = _BV(INT3);
		static constexpr const REGISTER EIFR_ = _SELECT_REG(EIFR);
		static constexpr const uint8_t EIFR_MASK = _BV(INTF3);
	};

	template<>
	struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT4>
	{
		static constexpr const REGISTER EICR_ = _SELECT_REG(EICRB);
		static constexpr const uint8_t EICR_MASK = _BV(ISC40) | _BV(ISC41);
		static constexpr const REGISTER EIMSK_ = _SELECT_REG(EIMSK);
		static constexpr const uint8_t EIMSK_MASK = _BV(INT4);
		static constexpr const REGISTER EIFR_ = _SELECT_REG(EIFR);
		static constexpr const uint8_t EIFR_MASK = _BV(INTF4);
	};

	template<>
	struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT5>
	{
		static constexpr const REGISTER EICR_ = _SELECT_REG(EICRB);
		static constexpr const uint8_t EICR_MASK = _BV(ISC50) | _BV(ISC51);
		static constexpr const REGISTER EIMSK_ = _SELECT_REG(EIMSK);
		static constexpr const uint8_t EIMSK_MASK = _BV(INT5);
		static constexpr const REGISTER EIFR_ = _SELECT_REG(EIFR);
		static constexpr const uint8_t EIFR_MASK = _BV(INTF5);
	};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<uint8_t PCINT>
	struct PCI_trait
	{
		static constexpr const Port PORT = Port::NONE;
	};
	template<>
	struct PCI_trait<0>
	{
		static constexpr const Port PORT = Port::PORT_B;
	};
	template<>
	struct PCI_trait<1>
	{
		static constexpr const Port PORT = Port::PORT_J;
	};
	template<>
	struct PCI_trait<2>
	{
		static constexpr const Port PORT = Port::PORT_K;
	};
	
	//=======
	// USART
	//=======
	
	template<USART USART>
	struct USART_trait
	{
		static constexpr const REGISTER UCSRA{};
		static constexpr const REGISTER UCSRB{};
		static constexpr const REGISTER UCSRC{};
		static constexpr const REGISTER UDR{};
		static constexpr const REGISTER UBRR{};
	};
	
	template<>
	struct USART_trait<USART::USART0>
	{
		static constexpr const REGISTER UCSRA = _SELECT_REG(UCSR0A);
		static constexpr const REGISTER UCSRB = _SELECT_REG(UCSR0B);
		static constexpr const REGISTER UCSRC = _SELECT_REG(UCSR0C);
		static constexpr const REGISTER UDR = _SELECT_REG(UDR0);
		static constexpr const REGISTER UBRR = _SELECT_REG(UBRR0);
	};
	
	template<>
	struct USART_trait<USART::USART1>
	{
		static constexpr const REGISTER UCSRA = _SELECT_REG(UCSR1A);
		static constexpr const REGISTER UCSRB = _SELECT_REG(UCSR1B);
		static constexpr const REGISTER UCSRC = _SELECT_REG(UCSR1C);
		static constexpr const REGISTER UDR = _SELECT_REG(UDR1);
		static constexpr const REGISTER UBRR = _SELECT_REG(UBRR1);
	};
	
	template<>
	struct USART_trait<USART::USART2>
	{
		static constexpr const REGISTER UCSRA = _SELECT_REG(UCSR2A);
		static constexpr const REGISTER UCSRB = _SELECT_REG(UCSR2B);
		static constexpr const REGISTER UCSRC = _SELECT_REG(UCSR2C);
		static constexpr const REGISTER UDR = _SELECT_REG(UDR2);
		static constexpr const REGISTER UBRR = _SELECT_REG(UBRR2);
	};
	
	template<>
	struct USART_trait<USART::USART3>
	{
		static constexpr const REGISTER UCSRA = _SELECT_REG(UCSR3A);
		static constexpr const REGISTER UCSRB = _SELECT_REG(UCSR3B);
		static constexpr const REGISTER UCSRC = _SELECT_REG(UCSR3C);
		static constexpr const REGISTER UDR = _SELECT_REG(UDR3);
		static constexpr const REGISTER UBRR = _SELECT_REG(UBRR3);
	};
		
	//=====
	// SPI
	//=====
	
	struct SPI_trait
	{
		static constexpr const REGISTER DDR = _SELECT_REG(DDRB);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTB);
		static constexpr const uint8_t SS = PB0;
		static constexpr const uint8_t MOSI = PB2;
		static constexpr const uint8_t MISO = PB3;
		static constexpr const uint8_t SCK = PB1;
	};

	//========
	// Timers
	//========
	
	template<Timer TIMER>
	struct Timer_trait
	{
		using COUNTER = uint8_t;
		static constexpr const uint16_t PRESCALER  = 0;
		static constexpr const uint8_t TCCRA_VALUE  = 0;
		static constexpr const uint8_t TCCRB_VALUE  = 0;
		static constexpr const REGISTER TCCRA{};
		static constexpr const REGISTER TCCRB{};
		static constexpr const REGISTER TCNT{};
		static constexpr const REGISTER OCRA{};
		static constexpr const REGISTER OCRB{};
		static constexpr const REGISTER TIMSK{};
		static constexpr const REGISTER TIFR{};
	};
	
	template<>
	struct Timer_trait<Timer::TIMER0>
	{
		using TYPE = uint8_t;
		static constexpr const uint16_t PRESCALER  = 64;
		static constexpr const uint8_t TCCRA_VALUE  = _BV(WGM01);
		static constexpr const uint8_t TCCRB_VALUE  = _BV(CS00) | _BV(CS01);
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR0A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR0B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT0);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR0A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR0B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK0);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR0);
	};
	
	template<>
	struct Timer_trait<Timer::TIMER2>
	{
		using TYPE = uint8_t;
		static constexpr const uint16_t PRESCALER  = 64;
		static constexpr const uint8_t TCCRA_VALUE  = _BV(WGM21);
		static constexpr const uint8_t TCCRB_VALUE  = _BV(CS22);
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR2A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR2B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT2);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR2A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR2B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK2);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR2);
	};
	
	template<>
	struct Timer_trait<Timer::TIMER1>
	{
		using TYPE = uint16_t;
		static constexpr const uint16_t PRESCALER  = 1;
		static constexpr const uint8_t TCCRA_VALUE  = 0;
		static constexpr const uint8_t TCCRB_VALUE  = _BV(WGM12) | _BV(CS10);
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR1A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR1B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT1);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR1A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR1B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK1);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR1);
	};
	
	template<>
	struct Timer_trait<Timer::TIMER3>
	{
		using TYPE = uint16_t;
		static constexpr const uint16_t PRESCALER  = 1;
		static constexpr const uint8_t TCCRA_VALUE  = 0;
		static constexpr const uint8_t TCCRB_VALUE  = _BV(WGM32) | _BV(CS30);
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR3A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR3B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT3);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR3A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR3B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK3);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR3);
	};
	
	template<>
	struct Timer_trait<Timer::TIMER4>
	{
		using TYPE = uint16_t;
		static constexpr const uint16_t PRESCALER  = 1;
		static constexpr const uint8_t TCCRA_VALUE  = 0;
		static constexpr const uint8_t TCCRB_VALUE  = _BV(WGM42) | _BV(CS40);
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR4A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR4B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT4);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR4A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR4B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK4);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR4);
	};
	
	template<>
	struct Timer_trait<Timer::TIMER5>
	{
		using TYPE = uint16_t;
		static constexpr const uint16_t PRESCALER  = 1;
		static constexpr const uint8_t TCCRA_VALUE  = 0;
		static constexpr const uint8_t TCCRB_VALUE  = _BV(WGM52) | _BV(CS50);
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR5A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR5B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT5);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR5A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR5B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK5);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR5);
	};
};

#endif /* BOARDS_MEGA_TRAITS_HH */
