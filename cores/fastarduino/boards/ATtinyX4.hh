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
	enum class Port: uint8_t
	{
		PORT_A = 0,
		PORT_B,
		NONE = 0xFF
	};

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
		LED = D7,
		NONE = 0xFF
	};

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

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	namespace InterruptPin
	{
		constexpr const DigitalPin D0 = DigitalPin::D0;
		constexpr const DigitalPin D1 = DigitalPin::D1;
		constexpr const DigitalPin D2 = DigitalPin::D2;
		constexpr const DigitalPin D3 = DigitalPin::D3;
		constexpr const DigitalPin D4 = DigitalPin::D4;
		constexpr const DigitalPin D5 = DigitalPin::D5;
		constexpr const DigitalPin D6 = DigitalPin::D6;
		constexpr const DigitalPin D7 = DigitalPin::D7;
		constexpr const DigitalPin D8 = DigitalPin::D8;
		constexpr const DigitalPin D9 = DigitalPin::D9;
		constexpr const DigitalPin D10 = DigitalPin::D10;
	};

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

	//========
	// Timers
	//========
	
	// IMPORTANT: on my setup, Timer runs faster than expected (9.5s for 10s)
	//TODO check how we can calibrate clock?
	enum class Timer: uint8_t
	{
		TIMER0,
		TIMER1
	};
	
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
