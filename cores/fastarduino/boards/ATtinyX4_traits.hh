#ifndef BOARDS_ATTINYX4_TRAITS_HH
#define BOARDS_ATTINYX4_TRAITS_HH

#include <avr/io.h>
#include "ATtinyX4.hh"

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
	};
	
	template<>
	struct Port_trait<Port::PORT_A>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINA);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRA);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTA);
		static constexpr const uint8_t DPIN_MASK = 0xFF;
	};
	
	template<>
	struct Port_trait<Port::PORT_B>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINB);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRB);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTB);
		static constexpr const uint8_t DPIN_MASK = 0x07;
	};
	
	/**
	 * Digital pin symbols
	 */
	template<DigitalPin DPIN>
	struct DigitalPin_trait
	{
		static constexpr const Port PORT = Port::NONE;
		static constexpr const uint8_t BIT = 0;
		//TODO other traits here? e.g. INT, PCI, USART...
	};
	template<Port P, uint8_t B>
	struct DigitalPin_trait_impl
	{
		static constexpr const Port PORT = P;
		static constexpr const uint8_t BIT = B;
	};

	template<> struct DigitalPin_trait<DigitalPin::NONE>: public DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D0>: public DigitalPin_trait_impl<Port::PORT_A, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1>: public DigitalPin_trait_impl<Port::PORT_A, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D2>: public DigitalPin_trait_impl<Port::PORT_A, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D3>: public DigitalPin_trait_impl<Port::PORT_A, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D4>: public DigitalPin_trait_impl<Port::PORT_A, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D5>: public DigitalPin_trait_impl<Port::PORT_A, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D6>: public DigitalPin_trait_impl<Port::PORT_A, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D7>: public DigitalPin_trait_impl<Port::PORT_A, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D8>: public DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D9>: public DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D10>: public DigitalPin_trait_impl<Port::PORT_B, 2> {};

#define _SELECT_PORT(PORT, ARG0, ARG1)	\
	(	PORT == Port::PORT_A ? ARG0 :	\
		ARG1)

#define _SELECT_PIN(DPIN, ARG0, ARG1)	\
	(uint8_t(DPIN) < uint8_t(DigitalPin::D8) ? ARG0 :	ARG1)
	
#define _SELECT_PIN_REG(DPIN, REG0, REG1)	\
	_SELECT_REG(_SELECT_PIN(DPIN, REG0, REG1))
	
	constexpr REGISTER PIN_REG(Port port)
	{
		return _SELECT_REG(_SELECT_PORT(port, PINA, PINB));
	}
	
	constexpr Port PORT(DigitalPin pin)
	{
		return _SELECT_PIN(pin, Port::PORT_A, Port::PORT_B);
	}

	constexpr REGISTER PIN_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, PINA, PINB);
	}

	constexpr REGISTER DDR_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, DDRA, DDRB);
	}

	constexpr REGISTER PORT_REG(DigitalPin pin)
	{
		return _SELECT_PIN_REG(pin, PORTA, PORTB);
	}

	constexpr uint8_t BIT(DigitalPin pin)
	{
		return _SELECT_PIN(	pin, 
							(uint8_t) pin, 
							(uint8_t) pin - (uint8_t) DigitalPin::D8);
	}

	//===============
	// IO interrupts
	//===============
	
	constexpr REGISTER EICR_REG(UNUSED ExternalInterruptPin PIN)
	{
		return _SELECT_REG(MCUCR);
	}
	
	constexpr uint8_t EICR_MASK(UNUSED ExternalInterruptPin PIN)
	{
		return 0x03 << ISC00;
	}

	constexpr REGISTER EIMSK_REG()
	{
		return _SELECT_REG(GIMSK);
	}

	constexpr uint8_t EIMSK_MASK(UNUSED ExternalInterruptPin PIN)
	{
		return _BV(INT0);
	}

	constexpr REGISTER EIFR_REG()
	{
		return _SELECT_REG(GIFR);
	}

	constexpr uint8_t EIFR_MASK(UNUSED ExternalInterruptPin PIN)
	{
		return _BV(INTF0);
	}

	/**
	 * Pin change interrupt (PCI) pins.
	 */
#define _SELECT_PCI_PIN(PIN, ARG0, ARG1)	\
	(uint8_t(PIN) < uint8_t(InterruptPin::PCI8) ? ARG0 : ARG1)
	
#define _SELECT_PCI_PORT(PIN)					\
	_SELECT_PCI_PIN(PIN, PCIPort::PCI0, PCIPort::PCI1)
	
#define _SELECT_PCI(PORT, ARG0, ARG1)	\
	(PORT == PCIPort::PCI0 ? ARG0 :	ARG1)
	
#define _SELECT_PCI_REG(PORT, REG0, REG1)	\
	_SELECT_REG(_SELECT_PCI(PORT, REG0, REG1))
	
#define _SELECT_PCI_MSK(PORT, MSK0, MSK1)	\
	_BV(_SELECT_PCI(PORT, MSK0, MSK1))
	
	constexpr uint8_t BIT(InterruptPin pin)
	{
		return _SELECT_PCI_PIN(	pin, 
								(uint8_t) pin, 
								(uint8_t) pin - (uint8_t) InterruptPin::PCI8);
	}
	
	constexpr PCIPort PCI_PORT(InterruptPin pin)
	{
		return _SELECT_PCI_PORT(pin);
	}
	constexpr REGISTER PCICR_REG()
	{
		return _SELECT_REG(GIMSK);
	}
	constexpr uint8_t PCIE_MSK(PCIPort PORT)
	{
		return _SELECT_PCI_MSK(PORT, PCIE0, PCIE1);
	}
	
	constexpr REGISTER PCIFR_REG()
	{
		return _SELECT_REG(GIFR);
	}
	constexpr uint8_t PCIFR_MSK(PCIPort PORT)
	{
		return _SELECT_PCI_MSK(PORT, PCIF0, PCIF1);
	}
	
	constexpr REGISTER PCMSK_REG(PCIPort PORT)
	{
		return _SELECT_PCI_REG(PORT, PCMSK0, PCMSK1);
	}

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
	
	//=====
	// SPI
	//=====
	
//	constexpr const REGISTER DDR_SPI_REG = _SELECT_REG(DDRA);
//	constexpr const REGISTER PORT_SPI_REG = _SELECT_REG(PORTA);
//	constexpr const uint8_t SPI_MOSI = PA5;
//	constexpr const uint8_t SPI_MISO = PA6;
//	constexpr const uint8_t SPI_SCK = PA4;

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

#endif /* BOARDS_ATTINYX4_TRAITS_HH */
