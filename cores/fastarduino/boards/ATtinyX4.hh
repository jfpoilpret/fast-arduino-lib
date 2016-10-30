#ifndef BOARDS_ATTINYX4_HH
#define BOARDS_ATTINYX4_HH

#include <avr/io.h>
#include <avr/sleep.h>

/* This board is based on ATtinyX4/ATtiny */
#define BOARDS_ATTINYX4
#define BOARD_ATTINY

/**
 * Cosa ATTINYX4 Board pin symbol definitions for the ATtinyX4
 * processors. Cosa does not use pin numbers as Arduino/Wiring,
 * instead strong data type is used (enum types) for the specific pin
 * classes; DigitalPin, AnalogPin, PWMPin, etc.
 *
 * The pin numbers are only symbolically mapped, i.e. a pin
 * number/digit will not work, symbols must be used, Board::D2.
 *
 * The static inline functions, SFR, BIT and UART, rely on compiler
 * optimizations to be reduced.
 *
 * @section Circuit
 * @code
 *                       ATinyX4
 *                     +----U----+
 * (VCC)-------------1-|VCC   GND|-14------------(GND)
 * (D8)--------------2-|PB0   PA0|-13----------(D0/A0)
 * (D9)--------------3-|PB1   PA1|-12----------(D1/A1)
 * (/RESET)----------4-|PB3   PA2|-11----------(D2/A2)
 * (EXT0/D10)--------5-|PB2   PA3|-10-------(D3/A3/SS)
 * (LED/D7/A7)-------6-|PA7   PA4|-9---(D4/A4/SCL/SCK)
 * (MISO/SDA/D6/A6)--7-|PA6   PA5|-8------(D5/A5/MOSI)
 *                     +---------+
 * @endcode
 */
namespace Board
{
	//====
	// IO
	//====
	constexpr const REGISTER PORT_A = _SELECT_REG(PINA);
	constexpr const REGISTER PORT_B = _SELECT_REG(PINB);

	/**
	 * Digital pin symbols
	 */
	enum class DigitalPin: uint8_t
	{
		D0 = 0,			// PA0
		D1,				// PA1
		D2,				// PA2
		D3,				// PA3
		D4,				// PA4
		D5,				// PA5
		D6,				// PA6
		D7,				// PA7
		D8,				// PB0
		D9,				// PB1
		D10,			// PB2
		LED = D7
	};

#define _SELECT_PIN(DPIN, ARG0, ARG1)	\
	(uint8_t(DPIN) < uint8_t(DigitalPin::D8) ? ARG0 :	ARG1)
	
#define _SELECT_PIN_REG(DPIN, REG0, REG1)	\
	_SELECT_REG(_SELECT_PIN(DPIN, REG0, REG1))
	
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
	
	/**
	 * External interrupt pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	enum class ExternalInterruptPin: uint8_t
	{
		EXT0 = DigitalPin::D10			// PB2
	};

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
	enum class PCIPort: uint8_t
	{
		PCI0 = 0,			// D0-D7, PA0-7
		PCI1 = 1			// D8-D10, PB0-2 (PB3 used for RESET)
	};

	enum class InterruptPin: uint8_t
	{
		PCI0 = DigitalPin::D0,			// PA0
		PCI1 = DigitalPin::D1,			// PA1
		PCI2 = DigitalPin::D2,			// PA2
		PCI3 = DigitalPin::D3,			// PA3
		PCI4 = DigitalPin::D4,			// PA4/SCK
		PCI5 = DigitalPin::D5,			// PA5/MOSI
		PCI6 = DigitalPin::D6,			// PA6/MISO
		PCI7 = DigitalPin::D7,			// PA7
		PCI8 = DigitalPin::D8,			// PB0
		PCI9 = DigitalPin::D9,			// PB1
		PCI10 = DigitalPin::D10			// PB2
	};

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
	
	enum class USART: uint8_t
	{
	};
	
	//=====
	// SPI
	//=====
	
	constexpr const REGISTER DDR_SPI_REG = _SELECT_REG(DDRA);
	constexpr const REGISTER PORT_SPI_REG = _SELECT_REG(PORTA);
	constexpr const uint8_t SPI_MOSI = PA5;
	constexpr const uint8_t SPI_MISO = PA6;
	constexpr const uint8_t SPI_SCK = PA4;

	//=============
	// Sleep Modes
	//=============

    #define SLEEP_MODE_PWR_SAVE     (_BV(SM0) | _BV(SM1))
	enum class SleepMode: uint8_t
	{
		IDLE = SLEEP_MODE_IDLE,
		ADC_NOISE_REDUCTION = SLEEP_MODE_ADC,
		POWER_DOWN = SLEEP_MODE_PWR_DOWN,
		POWER_SAVE = SLEEP_MODE_PWR_SAVE,
		STANDBY = SLEEP_MODE_PWR_SAVE,
		EXTENDED_STANDBY = SLEEP_MODE_PWR_SAVE,
		
		DEFAULT_MODE = 0xFF
	};
};

/**
 * Redefinition of symbols to allow generic code.
 */
#define ANALOG_COMP_vect ANA_COMP_vect
#define TIMER0_OVF_vect TIM0_OVF_vect
#define TIMER0_COMPA_vect TIM0_COMPA_vect
#define TIMER0_COMPB_vect TIM0_COMPB_vect
#define TIMER1_OVF_vect TIM1_OVF_vect
#define TIMER1_COMPA_vect TIM1_COMPA_vect
#define TIMER1_COMPB_vect TIM1_COMPB_vect

/**
 * Forward declare interrupt service routines to allow them as friends.
 */
extern "C" {
	void ADC_vect(void) __attribute__ ((signal));
	void ANALOG_COMP_vect(void) __attribute__ ((signal));
	void INT0_vect(void) __attribute__ ((signal));
	void PCINT0_vect(void) __attribute__ ((signal));
	void PCINT1_vect(void) __attribute__ ((signal));
	void TIMER0_COMPA_vect(void) __attribute__ ((signal));
	void TIMER0_COMPB_vect(void) __attribute__ ((signal));
	void TIMER0_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_COMPA_vect(void) __attribute__ ((signal));
	void TIMER1_COMPB_vect(void) __attribute__ ((signal));
	void TIMER1_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_CAPT_vect(void)  __attribute__ ((signal));
	void WDT_vect(void) __attribute__ ((signal));
	void USI_START_vect(void) __attribute__ ((signal));
	void USI_OVF_vect(void) __attribute__ ((signal));
}
#endif /* BOARDS_ATTINYX4_HH */
