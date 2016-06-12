#ifndef BOARDS_ATTINYX4_HH
#define BOARDS_ATTINYX4_HH

#include <avr/io.h>

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
	volatile uint8_t* const PORT_A = &PINA;
	volatile uint8_t* const PORT_B = &PINB;

	static volatile uint8_t* PIN(uint8_t pin)
	{
		return (pin < 8 ? &PINA : &PINB);
	}

	static constexpr uint8_t BIT(uint8_t pin)
	{
		return (pin < 8 ? pin : pin - 8);
	}
	
	/**
	 * Digital pin symbols
	 */
	enum DigitalPin
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
	} __attribute__((packed));

	/**
	 * External interrupt pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	enum ExternalInterruptPin
	{
		EXT0 = D10			// PB2
	} __attribute__((packed));

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	enum InterruptPin
	{
		PCI0 = D0,			// PA0
		PCI1 = D1,			// PA1
		PCI2 = D2,			// PA2
		PCI3 = D3,			// PA3
		PCI4 = D4,			// PA4
		PCI5 = D5,			// PA5
		PCI6 = D6,			// PA6
		PCI7 = D7,			// PA7
		PCI8 = D8,			// PB0
		PCI9 = D9,			// PB1
		PCI10 = D10			// PB2
	} __attribute__((packed));

	/**
	 * Size of pin maps.
	 */
	enum
	{
		ANALOG_PIN_MAX = 8,
		DIGITAL_PIN_MAX = 11,
		EXT_PIN_MAX = 1,
		PCI_PIN_MAX = 11,
		PWM_PIN_MAX = 4
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
