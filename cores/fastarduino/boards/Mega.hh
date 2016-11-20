#ifndef BOARDS_MEGA_HH
#define BOARDS_MEGA_HH

#include <avr/io.h>
#include <avr/sleep.h>

/* This board is based on ATmega1280/2560 but only ATmega2560 is supported */

/**
 * Cosa MEGA Board pin symbol definitions for the ATmega1280 and
 * ATmega2560 based Arduino boards; Mega 1280/2560. Cosa does not use
 * pin numbers as Arduino/Wiring, instead strong data type is used
 * (enum types) for the specific pin classes; DigitalPin, AnalogPin,
 * etc.
 *
 * The pin numbers for ATmega1280 and ATmega2560 are only symbolically
 * mapped, i.e. a pin number/digit will not work, symbols must be
 * used, e.g., Board::D42. Avoid iterations assuming that the symbols
 * are in order.
 *
 * The static inline functions, SFR, BIT and UART, rely on compiler
 * optimizations to be reduced.
 */
namespace Board
{
	//====
	// IO
	//====
	constexpr const REGISTER PORT_A = _SELECT_REG(PINA);
	constexpr const REGISTER PORT_B = _SELECT_REG(PINB);
	constexpr const REGISTER PORT_C = _SELECT_REG(PINC);
	constexpr const REGISTER PORT_D = _SELECT_REG(PIND);
	constexpr const REGISTER PORT_E = _SELECT_REG(PINE);
	constexpr const REGISTER PORT_F = _SELECT_REG(PINF);
	constexpr const REGISTER PORT_G = _SELECT_REG(PING);
	constexpr const REGISTER PORT_H = _SELECT_REG(PINH);
	constexpr const REGISTER PORT_J = _SELECT_REG(PINJ);
	constexpr const REGISTER PORT_K = _SELECT_REG(PINK);
	constexpr const REGISTER PORT_L = _SELECT_REG(PINL);
	
	/**
	 * Digital pin symbols
	 */
	enum class DigitalPin: uint8_t
	{
		D0 = 0,			// PE0/RX0
		D1 = 1,			// PE1/TX0
		D2 = 4,			// PE4
		D3 = 5,			// PE5
		D4 = 85,		// PG5
		D5 = 3,			// PE3
		D6 = 11,		// PH3
		D7 = 12,		// PH4
		D8 = 13,		// PH5
		D9 = 14,		// PH6
		D10 = 20,		// PB4
		D11 = 21,		// PB5
		D12 = 22,		// PB6
		D13 = 23,		// PB7
		D14 = 73,		// PJ1/TX3
		D15 = 72,		// PJ0/RX3
		D16 = 9,		// PH1/TX2
		D17 = 8,		// PH0/RX2
		D18 = 43,		// PD3/TX1
		D19 = 42,		// PD2/RX1
		D20 = 41,		// PD1/SDA
		D21 = 40,		// PD0/SCL
		D22 = 24,		// PA0/AD0
		D23 = 25,		// PA1
		D24 = 26,		// PA2
		D25 = 27,		// PA3
		D26 = 28,		// PA4
		D27 = 29,		// PA5
		D28 = 30,		// PA6
		D29 = 31,		// PA7
		D30 = 39,		// PC7
		D31 = 38,		// PC6
		D32 = 37,		// PC5
		D33 = 36,		// PC4
		D34 = 35,		// PC3
		D35 = 34,		// PC2
		D36 = 33,		// PC1
		D37 = 32,		// PC0
		D38 = 47,		// PD7
		D39 = 82,		// PG2
		D40 = 81,		// PG1
		D41 = 80,		// PG0
		D42 = 55,		// PL7
		D43 = 54,		// PL6
		D44 = 53,		// PL5
		D45 = 52,		// PL4
		D46 = 51,		// PL3
		D47 = 50,		// PL2
		D48 = 49,		// PL1
		D49 = 48,		// PL0
		D50 = 19,		// PB3/MISO
		D51 = 18,		// PB2/MOSI
		D52 = 17,		// PB1/SCK
		D53 = 16,		// PB0/SS
		D54 = 56,		// PF0/A0
		D55 = 57,		// PF1/A1
		D56 = 58,		// PF2/A2
		D57 = 59,		// PF3/A3
		D58 = 60,		// PF4/A4
		D59 = 61,		// PF5/A5
		D60 = 62,		// PF6/A6
		D61 = 63,		// PF7/A7
		D62 = 64,		// PK0/A8
		D63 = 65,		// PK1/A9
		D64 = 66,		// PK2/A10
		D65 = 67,		// PK3/A11
		D66 = 68,		// PK4/A12
		D67 = 69,		// PK5/A13
		D68 = 70,		// PK6/A14
		D69 = 71,		// PK7/A15
		LED = D13,
		NONE = 0XFF
	};

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
	
	/**
	 * External interrupt pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	enum class ExternalInterruptPin: uint8_t
	{
		EXT0 = DigitalPin::D21,			// PD0
		EXT1 = DigitalPin::D20,			// PD1
		EXT2 = DigitalPin::D19,			// PD2
		EXT3 = DigitalPin::D18,			// PD3
		EXT4 = DigitalPin::D2,			// PE4
		EXT5 = DigitalPin::D3			// PE5
	};

#define _SELECT_INT(INT_NUM, ARG0, ARG1, ARG2, ARG3, ARG4, ARG5)	\
	(	INT_NUM == ExternalInterruptPin::EXT0 ? ARG0 :				\
		INT_NUM == ExternalInterruptPin::EXT1 ? ARG1 :				\
		INT_NUM == ExternalInterruptPin::EXT2 ? ARG2 :				\
		INT_NUM == ExternalInterruptPin::EXT3 ? ARG3 :				\
		INT_NUM == ExternalInterruptPin::EXT4 ? ARG4 :				\
		ARG5)
	
	constexpr REGISTER EICR_REG(ExternalInterruptPin PIN)
	{
		return _SELECT_REG(_SELECT_INT(PIN, EICRA, EICRA, EICRA, EICRA, EICRB, EICRB));
	}
	
	constexpr uint8_t EICR_MASK(ExternalInterruptPin PIN)
	{
		return _SELECT_INT(
			PIN, 0x03 << ISC00, 0x03 << ISC10, 0x03 << ISC20, 0x03 << ISC30, 0x03 << ISC40, 0x03 << ISC50);
	}

	constexpr REGISTER EIMSK_REG()
	{
		return _SELECT_REG(EIMSK);
	}

	constexpr uint8_t EIMSK_MASK(ExternalInterruptPin PIN)
	{
		return _SELECT_INT(
			PIN, _BV(INT0), _BV(INT1), _BV(INT2), _BV(INT3), _BV(INT4), _BV(INT5));
	}

	constexpr REGISTER EIFR_REG()
	{
		return _SELECT_REG(EIFR);
	}

	constexpr uint8_t EIFR_MASK(ExternalInterruptPin PIN)
	{
		return _SELECT_INT(
			PIN, _BV(INTF0), _BV(INTF1), _BV(INTF2), _BV(INTF3), _BV(INTF4), _BV(INTF5));
	}

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	enum class PCIPort: uint8_t
	{
		PCI0 = 0,			// PB0-7
		PCI1 = 1,			// PJ0-1
		PCI2 = 2			// PK0-7
	};
	enum class InterruptPin: uint8_t
	{
		PCI0 = DigitalPin::D53,			// PB0
		PCI1 = DigitalPin::D52,			// PB1
		PCI2 = DigitalPin::D51,			// PB2
		PCI3 = DigitalPin::D50,			// PB3
		PCI4 = DigitalPin::D10,			// PB4
		PCI5 = DigitalPin::D11,			// PB5
		PCI6 = DigitalPin::D12,			// PB6
		PCI7 = DigitalPin::D13,			// PB7
		
		PCI9 = DigitalPin::D15,			// PJ0
		PCI10 = DigitalPin::D14,		// PJ1
		
		PCI16 = DigitalPin::D62,		// PK0
		PCI17 = DigitalPin::D63,		// PK1
		PCI18 = DigitalPin::D64,		// PK2
		PCI19 = DigitalPin::D65,		// PK3
		PCI20 = DigitalPin::D66,		// PK4
		PCI21 = DigitalPin::D67,		// PK5
		PCI22 = DigitalPin::D68,		// PK6
		PCI23 = DigitalPin::D69			// PK7
	};
	
#define _SELECT_PCI_PORT(PIN)					\
	((uint8_t) PIN < 24 ? PCIPort::PCI0 : (uint8_t) PIN < 72 ? PCIPort::PCI2 : PCIPort::PCI1)
	
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
		return ((uint8_t) pin) & 0x7;
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
		USART0 = 0,
		USART1 = 1,
		USART2 = 2,
		USART3 = 3
	};

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
	
	constexpr const REGISTER DDR_SPI_REG = _SELECT_REG(DDRB);
	constexpr const REGISTER PORT_SPI_REG = _SELECT_REG(PORTB);
	constexpr const uint8_t SPI_SS = PB0;
	constexpr const uint8_t SPI_MOSI = PB2;
	constexpr const uint8_t SPI_MISO = PB3;
	constexpr const uint8_t SPI_SCK = PB1;

	//========
	// Timers
	//========
	
	enum class Timer: uint8_t
	{
		TIMER0,
		TIMER1,
		TIMER2,
		TIMER3,
		TIMER4,
		TIMER5
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
	void INT2_vect(void) __attribute__ ((signal));
	void INT3_vect(void) __attribute__ ((signal));
	void INT4_vect(void) __attribute__ ((signal));
	void INT5_vect(void) __attribute__ ((signal));
	void INT6_vect(void) __attribute__ ((signal));
	void INT7_vect(void) __attribute__ ((signal));
	void PCINT0_vect(void) __attribute__ ((signal));
	void PCINT1_vect(void) __attribute__ ((signal));
	void PCINT2_vect(void) __attribute__ ((signal));
	void SPI_STC_vect(void) __attribute__ ((signal));
	void TIMER0_COMPA_vect(void) __attribute__ ((signal));
	void TIMER0_COMPB_vect(void) __attribute__ ((signal));
	void TIMER0_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_COMPA_vect(void) __attribute__ ((signal));
	void TIMER1_COMPB_vect(void) __attribute__ ((signal));
	void TIMER1_COMPC_vect(void) __attribute__ ((signal));
	void TIMER1_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_CAPT_vect(void)  __attribute__ ((signal));
	void TIMER2_COMPA_vect(void) __attribute__ ((signal));
	void TIMER2_COMPB_vect(void) __attribute__ ((signal));
	void TIMER2_OVF_vect(void) __attribute__ ((signal));
	void TIMER3_COMPA_vect(void) __attribute__ ((signal));
	void TIMER3_COMPB_vect(void) __attribute__ ((signal));
	void TIMER3_COMPC_vect(void) __attribute__ ((signal));
	void TIMER3_OVF_vect(void) __attribute__ ((signal));
	void TIMER3_CAPT_vect(void)  __attribute__ ((signal));
	void TIMER4_COMPA_vect(void) __attribute__ ((signal));
	void TIMER4_COMPB_vect(void) __attribute__ ((signal));
	void TIMER4_COMPC_vect(void) __attribute__ ((signal));
	void TIMER4_OVF_vect(void) __attribute__ ((signal));
	void TIMER4_CAPT_vect(void)  __attribute__ ((signal));
	void TIMER5_COMPA_vect(void) __attribute__ ((signal));
	void TIMER5_COMPB_vect(void) __attribute__ ((signal));
	void TIMER5_COMPC_vect(void) __attribute__ ((signal));
	void TIMER5_OVF_vect(void) __attribute__ ((signal));
	void TIMER5_CAPT_vect(void)  __attribute__ ((signal));
	void TWI_vect(void) __attribute__ ((signal));
	void WDT_vect(void) __attribute__ ((signal));
	void USART0_UDRE_vect(void) __attribute__ ((signal));
	void USART0_RX_vect(void) __attribute__ ((signal));
	void USART0_TX_vect(void) __attribute__ ((signal));
	void USART1_UDRE_vect(void) __attribute__ ((signal));
	void USART1_RX_vect(void) __attribute__ ((signal));
	void USART1_TX_vect(void) __attribute__ ((signal));
	void USART2_UDRE_vect(void) __attribute__ ((signal));
	void USART2_RX_vect(void) __attribute__ ((signal));
	void USART2_TX_vect(void) __attribute__ ((signal));
	void USART3_UDRE_vect(void) __attribute__ ((signal));
	void USART3_RX_vect(void) __attribute__ ((signal));
	void USART3_TX_vect(void) __attribute__ ((signal));
}
#endif /* BOARDS_MEGA_HH */
