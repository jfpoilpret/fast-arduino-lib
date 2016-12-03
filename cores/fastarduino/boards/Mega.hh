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
	enum class Port: uint8_t
	{
		PORT_A = 0,
		PORT_B,
		PORT_C,
		PORT_D,
		PORT_E,
		PORT_F,
		PORT_G,
		PORT_H,
		PORT_J,
		PORT_K,
		PORT_L,
		NONE = 0xFF
	};
	
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

	//==============
	// Analog Input
	//==============
	enum class AnalogClock: uint8_t
	{
		MAX_FREQ_50KHz = 0,
		MAX_FREQ_100KHz,
		MAX_FREQ_200KHz,
		MAX_FREQ_500KHz,
		MAX_FREQ_1MHz
	};
	enum class AnalogReference: uint8_t
	{
		AREF = 0,
		AVCC,
		INTERNAL_1_1V,
		INTERNAL_2_56V
	};
	
	enum class AnalogPin: uint8_t
	{
		A0 = 0,
		A1,
		A2,
		A3,
		A4,
		A5,
		A6,
		A7,
		A8,
		A9,
		A10,
		A11,
		A12,
		A13,
		A14,
		A15,
		BANDGAP,
		NONE = 0xFF
	};
	
	//===============
	// IO interrupts
	//===============
	
	/**
	 * External interrupt pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	namespace ExternalInterruptPin
	{
		constexpr const DigitalPin D21 = DigitalPin::D21;		// PD0
		constexpr const DigitalPin D20 = DigitalPin::D20;		// PD1
		constexpr const DigitalPin D19 = DigitalPin::D19;		// PD2
		constexpr const DigitalPin D18 = DigitalPin::D18;		// PD3
		constexpr const DigitalPin D2 = DigitalPin::D2;			// PE4
		constexpr const DigitalPin D3 = DigitalPin::D3;			// PE5
		constexpr const DigitalPin EXT0 = DigitalPin::D21;		// PD0
		constexpr const DigitalPin EXT1 = DigitalPin::D20;		// PD1
		constexpr const DigitalPin EXT2 = DigitalPin::D19;		// PD2
		constexpr const DigitalPin EXT3 = DigitalPin::D18;		// PD3
		constexpr const DigitalPin EXT4 = DigitalPin::D2;		// PE4
		constexpr const DigitalPin EXT5 = DigitalPin::D3;		// PE5
	};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	namespace InterruptPin
	{
		// PB0-7
		constexpr const DigitalPin D53 = DigitalPin::D53;
		constexpr const DigitalPin D52 = DigitalPin::D52;
		constexpr const DigitalPin D51 = DigitalPin::D51;
		constexpr const DigitalPin D50 = DigitalPin::D50;
		constexpr const DigitalPin D10 = DigitalPin::D10;
		constexpr const DigitalPin D11 = DigitalPin::D11;
		constexpr const DigitalPin D12 = DigitalPin::D12;
		constexpr const DigitalPin D13 = DigitalPin::D13;

		// PJ0-1
		constexpr const DigitalPin D15 = DigitalPin::D15;
		constexpr const DigitalPin D14 = DigitalPin::D14;
		
		// PK0-7
		constexpr const DigitalPin D62 = DigitalPin::D62;
		constexpr const DigitalPin D63 = DigitalPin::D63;
		constexpr const DigitalPin D64 = DigitalPin::D64;
		constexpr const DigitalPin D65 = DigitalPin::D65;
		constexpr const DigitalPin D66 = DigitalPin::D66;
		constexpr const DigitalPin D67 = DigitalPin::D67;
		constexpr const DigitalPin D68 = DigitalPin::D68;
		constexpr const DigitalPin D69 = DigitalPin::D69;
	};

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

	//=====
	// SPI
	//=====
	
	// Nothing special

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
