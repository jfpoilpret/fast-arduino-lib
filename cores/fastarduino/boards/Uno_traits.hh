#ifndef BOARDS_UNO_TRAITS_HH
#define BOARDS_UNO_TRAITS_HH

#include <avr/io.h>
#include "Uno.hh"

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
	struct Port_trait<Port::PORT_B>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINB);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRB);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTB);
		static constexpr const uint8_t DPIN_MASK = 0xFF;
		
		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x3F;
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
		static constexpr const uint8_t DPIN_MASK = 0x7F;

		static constexpr const uint8_t PCINT = 1;
		static constexpr const uint8_t PCI_MASK = 0x3F;
		static constexpr const uint8_t PCICR_MASK = _BV(PCIE1); 
		static constexpr const uint8_t PCIFR_MASK = _BV(PCIF1);
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_ = _SELECT_REG(PCMSK1);
	};
	
	template<>
	struct Port_trait<Port::PORT_D>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PIND);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRD);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTD);
		static constexpr const uint8_t DPIN_MASK = 0xFF;

		static constexpr const uint8_t PCINT = 2;
		static constexpr const uint8_t PCI_MASK = 0xFF;
		static constexpr const uint8_t PCICR_MASK = _BV(PCIE2); 
		static constexpr const uint8_t PCIFR_MASK = _BV(PCIF2);
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_ = _SELECT_REG(PCMSK2);
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
	
	template<> struct DigitalPin_trait<DigitalPin::D0>: public DigitalPin_trait_impl<Port::PORT_D, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1>: public DigitalPin_trait_impl<Port::PORT_D, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D2>: public DigitalPin_trait_impl<Port::PORT_D, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D3>: public DigitalPin_trait_impl<Port::PORT_D, 3, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D4>: public DigitalPin_trait_impl<Port::PORT_D, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D5>: public DigitalPin_trait_impl<Port::PORT_D, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D6>: public DigitalPin_trait_impl<Port::PORT_D, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D7>: public DigitalPin_trait_impl<Port::PORT_D, 7> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D8>: public DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D9>: public DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D10>: public DigitalPin_trait_impl<Port::PORT_B, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D11>: public DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D12>: public DigitalPin_trait_impl<Port::PORT_B, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D13>: public DigitalPin_trait_impl<Port::PORT_B, 5> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D14>: public DigitalPin_trait_impl<Port::PORT_C, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D15>: public DigitalPin_trait_impl<Port::PORT_C, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D16>: public DigitalPin_trait_impl<Port::PORT_C, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D17>: public DigitalPin_trait_impl<Port::PORT_C, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D18>: public DigitalPin_trait_impl<Port::PORT_C, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D19>: public DigitalPin_trait_impl<Port::PORT_C, 5> {};
	
#define _SELECT_PORT(PORT, ARG0, ARG1, ARG2)	\
	(	PORT == Port::PORT_B ? ARG0 :			\
		PORT == Port::PORT_C ? ARG1 :			\
		ARG2)

#define _SELECT_PIN(DPIN, ARG0, ARG1, ARG2)	\
	(	DPIN < DigitalPin::D8 ? ARG0 :		\
		DPIN < DigitalPin::D14 ? ARG1 :		\
		ARG2)
	
#define _SELECT_PIN_REG(DPIN, REG0, REG1, REG2)		\
	_SELECT_REG(_SELECT_PIN(DPIN, REG0, REG1, REG2))

	constexpr REGISTER PIN_REG(Port port)
	{
		return _SELECT_REG(_SELECT_PORT(port, PINB, PINC, PIND));
	}
	
	constexpr Port PORT(DigitalPin pin)
	{
		return _SELECT_PIN(pin, Port::PORT_D, Port::PORT_B, Port::PORT_C);
	}

	constexpr REGISTER PIN_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, PIND, PINB, PINC);
	}

	constexpr REGISTER DDR_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, DDRD, DDRB, DDRC);
	}

	constexpr REGISTER PORT_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, PORTD, PORTB, PORTC);
	}

	constexpr uint8_t BIT(DigitalPin pin)
	{
		return _SELECT_PIN(	pin, 
							(uint8_t) pin, 
							(uint8_t) pin - (uint8_t) DigitalPin::D8, 
							(uint8_t) pin - (uint8_t) DigitalPin::D14);
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
		static constexpr const Port PORT = Port::PORT_C;
	};
	template<>
	struct PCI_trait<2>
	{
		static constexpr const Port PORT = Port::PORT_D;
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
	
	//=====
	// SPI
	//=====
	
//	constexpr const REGISTER DDR_SPI_REG = _SELECT_REG(DDRB);
//	constexpr const REGISTER PORT_SPI_REG = _SELECT_REG(PORTB);
//	constexpr const uint8_t SPI_SS = PB2;
//	constexpr const uint8_t SPI_MOSI = PB3;
//	constexpr const uint8_t SPI_MISO = PB4;
//	constexpr const uint8_t SPI_SCK = PB5;

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
};

#endif /* BOARDS_UNO_TRAITS_HH */
