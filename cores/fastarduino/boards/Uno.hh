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
	enum class Port: uint8_t
	{
		PORT_B = 0,
		PORT_C,
		PORT_D,
		NONE = 0xFF
	};
	
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
		LED = D13,
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
		EXT0 = DigitalPin::D2,		// PD2
		EXT1 = DigitalPin::D3		// PD3
	};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
//	using InterruptPin = DigitalPin;
	
	//TODO try another way (struct?) with implicit conversion from/to DigitalPin
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
		constexpr const DigitalPin D11 = DigitalPin::D11;
		constexpr const DigitalPin D12 = DigitalPin::D12;
		constexpr const DigitalPin D13 = DigitalPin::D13;
		constexpr const DigitalPin D14 = DigitalPin::D14;
		constexpr const DigitalPin D15 = DigitalPin::D15;
		constexpr const DigitalPin D16 = DigitalPin::D16;
		constexpr const DigitalPin D17 = DigitalPin::D17;
		constexpr const DigitalPin D18 = DigitalPin::D18;
		constexpr const DigitalPin D19 = DigitalPin::D19;
	};

	//=======
	// USART
	//=======
	
	enum class USART: uint8_t
	{
		USART0 = 0
	};
	
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
