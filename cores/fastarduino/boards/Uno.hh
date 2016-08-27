#ifndef BOARDS_UNO_HH
#define BOARDS_UNO_HH

#include <avr/io.h>
#include <avr/sleep.h>

/* This board is based on ATmega328P */
#define BOARD_ATMEGA328P

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
#define _SELECT_REG(REG) REGISTER((uint8_t)(uint16_t)&REG)

	//TODO Replace `volatile uintx*` with REGISTER everywhere?
	
	constexpr volatile uint8_t* const PORT_B = &PINB;
	constexpr volatile uint8_t* const PORT_C = &PINC;
	constexpr volatile uint8_t* const PORT_D = &PIND;

#define _SELECT_PIN_REG(DPIN, REG0, REG1, REG2)		\
	REGISTER(	(uint8_t)(uint16_t)					\
				(	DPIN < 8 ? &REG0 :	\
					DPIN < 14 ? &REG1 :	\
					&REG2))

	constexpr REGISTER PIN_REG(uint8_t pin)
	{
		return _SELECT_PIN_REG(pin, PIND, PINB, PINC);
	}

	constexpr REGISTER DDR_REG(uint8_t pin)
	{
		return _SELECT_PIN_REG(pin, DDRD, DDRB, DDRC);
	}

	constexpr REGISTER PORT_REG(uint8_t pin)
	{
		return _SELECT_PIN_REG(pin, PORTD, PORTB, PORTC);
	}

	//TODO REMNOVE AFTER REPLACING WITH PIN_REG
	constexpr volatile uint8_t* PIN(uint8_t pin)
	{
		return pin < 8  ? PORT_D : pin < 14 ? PORT_B : PORT_C;
	}

	//TODO Replace arg with string type (enum class)
	constexpr uint8_t BIT(uint8_t pin)
	{
		return (pin < 8  ? pin : pin < 14 ? pin - 8 : pin - 14);
	}
	
	/**
	 * Digital pin symbols
	 */
	//TODO Make stronger types for pins (enum class)?
	enum DigitalPin
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
	} __attribute__((packed));

	/**
	 * External interrupt pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	enum ExternalInterruptPin
	{
		EXT0 = D2,			// PD2
		EXT1 = D3			// PD3
	} __attribute__((packed));

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
		PCI0 = D0,			// PD0
		PCI1 = D1,			// PD1
		PCI2 = D2,			// PD2
		PCI3 = D3,			// PD3
		PCI4 = D4,			// PD4
		PCI5 = D5,			// PD5
		PCI6 = D6,			// PD6
		PCI7 = D7,			// PD7
		PCI8 = D8,			// PB0
		PCI9 = D9,			// PB1
		PCI10 = D10,		// PB2
		PCI11 = D11,		// PB3
		PCI12 = D12,		// PB4
		PCI13 = D13,		// PB5
		PCI14 = D14,		// PC0
		PCI15 = D15,		// PC1
		PCI16 = D16,		// PC2
		PCI17 = D17,		// PC3
		PCI18 = D18,		// PC4
		PCI19 = D19			// PC5
	};

#define _SELECT_PCI_PORT(PIN)						\
	((PIN) < 8 ? PCIPort::PCI2 : (PIN) < 14 ? PCIPort::PCI0 : PCIPort::PCI1)
	
#define _SELECT_PCI_REG(PORT, REG0, REG1, REG2)		\
	REGISTER(	(uint8_t)(uint16_t)					\
				(	PORT == PCIPort::PCI0 ? &REG0 :	\
					PORT == PCIPort::PCI1 ? &REG1 :	\
					&REG2))
	
#define _SELECT_PCI_MSK(PORT, MSK0, MSK1, MSK2)	\
	_BV(PORT == PCIPort::PCI0 ? MSK0 :			\
		PORT == PCIPort::PCI1 ? MSK1 :			\
		MSK2)

	constexpr PCIPort PCI_PORT(InterruptPin pin)
	{
		return _SELECT_PCI_PORT((uint8_t) pin);
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

	/**
	 * Size of pin maps.
	 */
	enum
	{
		USART_MAX = 1,
		ANALOG_PIN_MAX = 8,
		DIGITAL_PIN_MAX = 20,
		EXT_PIN_MAX = 2,
		PCI_PIN_MAX = 20,
		PWM_PIN_MAX = 6
	};
	
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
	
	enum class USART: uint8_t
	{
		USART0 = 0
	};
	
	constexpr REGISTER UCSRA_REG(__attribute__((unused)) USART usart)
	{
		return _SELECT_REG(UCSR0A);
	}

	constexpr REGISTER UCSRB_REG(__attribute__((unused)) USART usart)
	{
		return _SELECT_REG(UCSR0B);
	}

	constexpr REGISTER UCSRC_REG(__attribute__((unused)) USART usart)
	{
		return _SELECT_REG(UCSR0C);
	}

	constexpr REGISTER UDR_REG(__attribute__((unused)) USART usart)
	{
		return _SELECT_REG(UDR0);
	}

	constexpr REGISTER UBRR_REG(__attribute__((unused)) USART usart)
	{
		return _SELECT_REG(UBRR0);
	}

	//TODO maybe useless? BETTER DEFINE MACROS FOR DIRECT USE IN SPECIALIZED HEADERS uart.hh, pci.hh...
	enum class Circuits
	{
		TIMER_0,
		TIMER_1,
		TIMER_2,
		USART_0,
		SPI_0,
		TWI_0
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
