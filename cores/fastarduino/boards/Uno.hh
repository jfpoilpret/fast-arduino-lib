#ifndef BOARDS_UNO_HH
#define BOARDS_UNO_HH

#include <avr/io.h>
#include <avr/sleep.h>

/* This board is based on ATmega328P */
#define BOARD_ATMEGA328P

//TODO Use traits for more settings and reduce macros used in here to the strict minimum
//TODO Externalize traits (to Uno_traits.hh, MEGA_traits.hh ...)

/**
 * Cosa pin symbol and hardware definitions for the ATmega328P based
 * board Arduino Uno. Cosa does not use pin numbers as Arduino/Wiring,
 * instead strong data type is used (enum types) for the specific pin
 * classes; DigitalPin, AnalogPin, PWMPin, etc.
 *
 * The pin numbers for ATmega328P are mapped as in Arduino. The static
 * inline functions, SFR, BIT and UART, rely on compiler optimizations
 * to be reduced.
 *
 * @section Board
 * @code
 *                         Arduino Uno
 *                  -----              -------
 *                +-|(o)|--------------| USB |---+
 *                | |   |              |     |   |
 *                | -----              |     |   |
 *                |                    -------   |
 *                |                              |
 *                |                            []| SCL
 *                |                            []| SDA
 *                |                            []| AREF
 *                |                            []| GND
 *             NC |[]                          []| D13/SCK/LED
 *          IOREF |[]                          []| D12/MISO
 *          RESET |[]                          []| D11/MOSI/PWM5
 *            3V3 |[]                          []| D10/SS/PWM4
 *             5V |[]                          []| D9/PWM3
 *            GND |[]                          []| D8
 *            GND |[]                            |
 *            Vin |[]                          []| D7
 *                |                            []| D6/PWM2
 *         A0/D14 |[]                          []| D5/PWM1
 *         A1/D15 |[]                          []| D4
 *         A2/D16 |[]                          []| D3/EXT1/PWM0
 *         A3/D17 |[]                          []| D2/EXT0
 *     SDA/A4/D18 |[]            ICSP          []| D1/TX
 *     SCL/A5/D19 |[]           o-o-o*         []| D0/RX
 *                 \            o-o-o           /
 *                  +--------------------------+
 * @endcode
 */
namespace Board
{
	//====
	// IO
	//====
	constexpr const REGISTER PORT_B = _SELECT_REG(PINB);
	constexpr const REGISTER PORT_C = _SELECT_REG(PINC);
	constexpr const REGISTER PORT_D = _SELECT_REG(PIND);

	/**
	 * Digital pin symbols
	 */
	enum class DigitalPin: uint8_t
	{
		D0 = 0,			// PD0
		D1,				// PD1
		D2,				// PD2
		D3,				// PD3
		D4,				// PD4
		D5,				// PD5
		D6,				// PD6
		D7,				// PD7
		D8,				// PB0
		D9,				// PB1
		D10,			// PB2
		D11,			// PB3
		D12,			// PB4
		D13,			// PB5
		D14,			// PC0-A0
		D15,			// PC1-A1
		D16,			// PC2-A2
		D17,			// PC3-A3
		D18,			// PC4-A4
		D19,			// PC5-A5
		LED = D13
	};

#define _SELECT_PIN(DPIN, ARG0, ARG1, ARG2)		\
	(	DPIN < DigitalPin::D8 ? ARG0 :	\
		DPIN < DigitalPin::D14 ? ARG1 :	\
		ARG2)
	
#define _SELECT_PIN_REG(DPIN, REG0, REG1, REG2)		\
	_SELECT_REG(_SELECT_PIN(DPIN, REG0, REG1, REG2))

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
	
	/**
	 * External interrupt pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	enum class ExternalInterruptPin: uint8_t
	{
		EXT0 = DigitalPin::D2,		// PD2
		EXT1 = DigitalPin::D3		// PD3
	};

#define _SELECT_INT(INT_NUM, ARG0, ARG1)	\
	(INT_NUM == ExternalInterruptPin::EXT0 ? ARG0 : ARG1)
	
	constexpr REGISTER EICR_REG(UNUSED ExternalInterruptPin PIN)
	{
		return _SELECT_REG(EICRA);
	}
	
	constexpr uint8_t EICR_MASK(ExternalInterruptPin PIN)
	{
		return _SELECT_INT(PIN, 0x03 << ISC00, 0x03 << ISC10);
	}

	constexpr REGISTER EIMSK_REG()
	{
		return _SELECT_REG(EIMSK);
	}

	constexpr uint8_t EIMSK_MASK(ExternalInterruptPin PIN)
	{
		return _SELECT_INT(PIN, _BV(INT0), _BV(INT1));
	}

	constexpr REGISTER EIFR_REG()
	{
		return _SELECT_REG(EIFR);
	}

	constexpr uint8_t EIFR_MASK(ExternalInterruptPin PIN)
	{
		return _SELECT_INT(PIN, _BV(INTF0), _BV(INTF1));
	}

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	enum class PCIPort: uint8_t
	{
		PCI0 = 0,			// D8-D13, PB0-5 (PB6 & PB7 are for XTAL on Arduino UNO)
		PCI1 = 1,			// A0-A5, PC0-5 (PC6 used for RESET)
		PCI2 = 2			// D0-D7, PD0-7
	};
	enum class InterruptPin: uint8_t
	{
		PCI0 = DigitalPin::D0,			// PD0
		PCI1 = DigitalPin::D1,			// PD1
		PCI2 = DigitalPin::D2,			// PD2
		PCI3 = DigitalPin::D3,			// PD3
		PCI4 = DigitalPin::D4,			// PD4
		PCI5 = DigitalPin::D5,			// PD5
		PCI6 = DigitalPin::D6,			// PD6
		PCI7 = DigitalPin::D7,			// PD7
		PCI8 = DigitalPin::D8,			// PB0
		PCI9 = DigitalPin::D9,			// PB1
		PCI10 = DigitalPin::D10,		// PB2
		PCI11 = DigitalPin::D11,		// PB3
		PCI12 = DigitalPin::D12,		// PB4
		PCI13 = DigitalPin::D13,		// PB5
		PCI14 = DigitalPin::D14,		// PC0
		PCI15 = DigitalPin::D15,		// PC1
		PCI16 = DigitalPin::D16,		// PC2
		PCI17 = DigitalPin::D17,		// PC3
		PCI18 = DigitalPin::D18,		// PC4
		PCI19 = DigitalPin::D19			// PC5
	};

#define _SELECT_PCI_PIN(PIN, ARG0, ARG1, ARG2)	\
	(PIN < InterruptPin::PCI8 ? ARG0 : PIN < InterruptPin::PCI14 ? ARG1 : ARG2)
	
#define _SELECT_PCI_PORT(PIN)					\
	_SELECT_PCI_PIN(PIN, PCIPort::PCI2, PCIPort::PCI0, PCIPort::PCI1)
	
#define _SELECT_PCI(PORT, ARG0, ARG1, ARG2)	\
	(	PORT == PCIPort::PCI0 ? ARG0 :		\
		PORT == PCIPort::PCI1 ? ARG1 :		\
		ARG2)
	
#define _SELECT_PCI_REG(PORT, REG0, REG1, REG2)	\
	_SELECT_REG(_SELECT_PCI(PORT, REG0, REG1, REG2))
	
#define _SELECT_PCI_MSK(PORT, MSK0, MSK1, MSK2)	\
	_BV(_SELECT_PCI(PORT, MSK0, MSK1, MSK2))

	constexpr uint8_t BIT(InterruptPin pin)
	{
		return _SELECT_PCI_PIN(	pin, 
								(uint8_t) pin, 
								(uint8_t) pin - (uint8_t) InterruptPin::PCI8, 
								(uint8_t) pin - (uint8_t) InterruptPin::PCI14);
	}
	
	constexpr PCIPort PCI_PORT(InterruptPin pin)
	{
		return _SELECT_PCI_PORT(pin);
	}
	constexpr REGISTER PCICR_REG()
	{
		return _SELECT_REG(PCICR);
	}
	constexpr uint8_t PCIE_MSK(PCIPort PORT)
	{
		return _SELECT_PCI_MSK(PORT, PCIE0, PCIE1, PCIE2);
	}
	
	constexpr REGISTER PCIFR_REG()
	{
		return _SELECT_REG(PCIFR);
	}
	constexpr uint8_t PCIFR_MSK(PCIPort PORT)
	{
		return _SELECT_PCI_MSK(PORT, PCIF0, PCIF1, PCIF2);
	}
	
	constexpr REGISTER PCMSK_REG(PCIPort PORT)
	{
		return _SELECT_PCI_REG(PORT, PCMSK0, PCMSK1, PCMSK2);
	}

	//=======
	// USART
	//=======
	
	enum class USART: uint8_t
	{
		USART0 = 0
	};
	
	constexpr REGISTER UCSRA_REG(UNUSED USART usart)
	{
		return _SELECT_REG(UCSR0A);
	}

	constexpr REGISTER UCSRB_REG(UNUSED USART usart)
	{
		return _SELECT_REG(UCSR0B);
	}

	constexpr REGISTER UCSRC_REG(UNUSED USART usart)
	{
		return _SELECT_REG(UCSR0C);
	}

	constexpr REGISTER UDR_REG(UNUSED USART usart)
	{
		return _SELECT_REG(UDR0);
	}

	constexpr REGISTER UBRR_REG(UNUSED USART usart)
	{
		return _SELECT_REG(UBRR0);
	}
	
	//=====
	// SPI
	//=====
	
	constexpr const REGISTER DDR_SPI_REG = _SELECT_REG(DDRB);
	constexpr const REGISTER PORT_SPI_REG = _SELECT_REG(PORTB);
	constexpr const uint8_t SPI_SS = PB2;
	constexpr const uint8_t SPI_MOSI = PB3;
	constexpr const uint8_t SPI_MISO = PB4;
	constexpr const uint8_t SPI_SCK = PB5;

	//========
	// Timers
	//========
	
	enum class Timer: uint8_t
	{
		TIMER0,
		TIMER1,
		TIMER2
	};
	
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
		static constexpr const uint8_t TCCRB_VALUE  = _BV(CS20) | _BV(CS21);
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
	
	//=============
	// Sleep Modes
	//=============

	enum class SleepMode: uint8_t
	{
		IDLE = SLEEP_MODE_IDLE,
		ADC_NOISE_REDUCTION = SLEEP_MODE_ADC,
		POWER_DOWN = SLEEP_MODE_PWR_DOWN,
		POWER_SAVE = SLEEP_MODE_PWR_SAVE,
		STANDBY = SLEEP_MODE_STANDBY,
		EXTENDED_STANDBY = SLEEP_MODE_EXT_STANDBY,
		
		DEFAULT_MODE = 0xFF
	};
	
};

/**
 * Forward declare interrupt service routines to allow them as friends.
 */
extern "C" {
	void ADC_vect(void) __attribute__ ((signal));
	void ANALOG_COMP_vect(void) __attribute__ ((signal));
	void INT0_vect(void) __attribute__ ((signal));
	void INT1_vect(void) __attribute__ ((signal));
	void PCINT0_vect(void) __attribute__ ((signal));
	void PCINT1_vect(void) __attribute__ ((signal));
	void PCINT2_vect(void) __attribute__ ((signal));
	void SPI_STC_vect(void) __attribute__ ((signal));
	void TIMER0_COMPA_vect(void) __attribute__ ((signal));
	void TIMER0_COMPB_vect(void) __attribute__ ((signal));
	void TIMER0_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_CAPT_vect(void)  __attribute__ ((signal));
	void TIMER1_COMPA_vect(void) __attribute__ ((signal));
	void TIMER1_COMPB_vect(void) __attribute__ ((signal));
	void TIMER1_OVF_vect(void) __attribute__ ((signal));
	void TIMER2_COMPA_vect(void) __attribute__ ((signal));
	void TIMER2_COMPB_vect(void) __attribute__ ((signal));
	void TIMER2_OVF_vect(void) __attribute__ ((signal));
	void TWI_vect(void) __attribute__ ((signal));
	void WDT_vect(void) __attribute__ ((signal));
	void USART_RX_vect(void) __attribute__ ((signal));
	void USART_TX_vect(void) __attribute__ ((signal));
	void USART_UDRE_vect(void) __attribute__ ((signal));
}

#define USART0_RX_vect USART_RX_vect
#define USART0_TX_vect USART_TX_vect
#define USART0_UDRE_vect USART_UDRE_vect

#endif /* BOARDS_UNO_HH */
