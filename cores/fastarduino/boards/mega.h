//   Copyright 2016-2017 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef BOARDS_MEGA_HH
#define BOARDS_MEGA_HH

#include <avr/io.h>
#include <avr/sleep.h>

/* This board is based on ATmega1280/2560 but only ATmega2560 is supported */

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif

namespace board
{
	//=====================
	// Initialization code
	//=====================
	inline static void init() INLINE;
	inline static void init() {}
	
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
		D0_PE0 = 0,	// PE0/RX0
		D1_PE1,		// PE1/TX0
		D2_PE4,		// PE4
		D3_PE5,		// PE5
		D4_PG5,		// PG5
		D5_PE3,		// PE3
		D6_PH3,		// PH3
		D7_PH4,		// PH4
		D8_PH5,		// PH5
		D9_PH6,		// PH6
		D10_PB4,	// PB4
		D11_PB5,	// PB5
		D12_PB6,	// PB6
		D13_PB7,	// PB7
		D14_PJ1,	// PJ1/TX3
		D15_PJ0,	// PJ0/RX3
		D16_PH1,	// PH1/TX2
		D17_PH0,	// PH0/RX2
		D18_PD3,	// PD3/TX1
		D19_PD2,	// PD2/RX1
		D20_PD1,	// PD1/SDA
		D21_PD0,	// PD0/SCL
		D22_PA0,	// PA0/AD0
		D23_PA1,	// PA1
		D24_PA2,	// PA2
		D25_PA3,	// PA3
		D26_PA4,	// PA4
		D27_PA5,	// PA5
		D28_PA6,	// PA6
		D29_PA7,	// PA7
		D30_PC7,	// PC7
		D31_PC6,	// PC6
		D32_PC5,	// PC5
		D33_PC4,	// PC4
		D34_PC3,	// PC3
		D35_PC2,	// PC2
		D36_PC1,	// PC1
		D37_PC0,	// PC0
		D38_PD7,	// PD7
		D39_PG2,	// PG2
		D40_PG1,	// PG1
		D41_PG0,	// PG0
		D42_PL7,	// PL7
		D43_PL6,	// PL6
		D44_PL5,	// PL5
		D45_PL4,	// PL4
		D46_PL3,	// PL3
		D47_PL2,	// PL2
		D48_PL1,	// PL1
		D49_PL0,	// PL0
		D50_PB3,	// PB3/MISO
		D51_PB2,	// PB2/MOSI
		D52_PB1,	// PB1/SCK
		D53_PB0,	// PB0/SS
		D54_PF0,	// PF0/A0
		D55_PF1,	// PF1/A1
		D56_PF2,	// PF2/A2
		D57_PF3,	// PF3/A3
		D58_PF4,	// PF4/A4
		D59_PF5,	// PF5/A5
		D60_PF6,	// PF6/A6
		D61_PF7,	// PF7/A7
		D62_PK0,	// PK0/A8
		D63_PK1,	// PK1/A9
		D64_PK2,	// PK2/A10
		D65_PK3,	// PK3/A11
		D66_PK4,	// PK4/A12
		D67_PK5,	// PK5/A13
		D68_PK6,	// PK6/A14
		D69_PK7,	// PK7/A15
		LED = DigitalPin::D13_PB7,
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
	
	//============
	// PWM output
	//============
	/**
	 * PWM-enabled pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	namespace PWMPin
	{
		// This pin is also OC1C! We can handle only one timer for one pin!
		constexpr const DigitalPin D13_PB7_OC0A = DigitalPin::D13_PB7;
		constexpr const DigitalPin D4_PG5_OC0B = DigitalPin::D4_PG5;
		
		constexpr const DigitalPin D11_PB5_OC1A = DigitalPin::D11_PB5;
		constexpr const DigitalPin D12_PB6_OC1B = DigitalPin::D12_PB6;
		
		constexpr const DigitalPin D10_PB4_OC2A = DigitalPin::D10_PB4;
		constexpr const DigitalPin D9_PH6_OC2B = DigitalPin::D9_PH6;

		constexpr const DigitalPin D3_PE5_OC3A = DigitalPin::D5_PE3;
		constexpr const DigitalPin D2_PE4_OC3B = DigitalPin::D2_PE4;
		constexpr const DigitalPin D3_PE5_OC3C = DigitalPin::D3_PE5;
		
		constexpr const DigitalPin D6_PH3_OC4A = DigitalPin::D6_PH3;
		constexpr const DigitalPin D7_PH4_OC4B = DigitalPin::D7_PH4;
		constexpr const DigitalPin D8_PH5_OC4C = DigitalPin::D8_PH5;

		constexpr const DigitalPin D44_PL5_OC5C = DigitalPin::D44_PL5;
		constexpr const DigitalPin D45_PL4_OC5B = DigitalPin::D45_PL4;
		constexpr const DigitalPin D46_PL3_OC5A = DigitalPin::D46_PL3;
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
		constexpr const DigitalPin D21_PD0_EXT0 = DigitalPin::D21_PD0;		// PD0
		constexpr const DigitalPin D20_PD1_EXT1 = DigitalPin::D20_PD1;		// PD1
		constexpr const DigitalPin D19_PD2_EXT2 = DigitalPin::D19_PD2;		// PD2
		constexpr const DigitalPin D18_PD3_EXT3 = DigitalPin::D18_PD3;		// PD3
		constexpr const DigitalPin D2_PE4_EXT4 = DigitalPin::D2_PE4;		// PE4
		constexpr const DigitalPin D3_PE5_EXT5 = DigitalPin::D3_PE5;		// PE5
	};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	namespace InterruptPin
	{
		// PB0-7
		constexpr const DigitalPin D53_PB0_PCI0 = DigitalPin::D53_PB0;
		constexpr const DigitalPin D52_PB1_PCI0 = DigitalPin::D52_PB1;
		constexpr const DigitalPin D51_PB2_PCI0 = DigitalPin::D51_PB2;
		constexpr const DigitalPin D50_PB3_PCI0 = DigitalPin::D50_PB3;
		constexpr const DigitalPin D10_PB4_PCI0 = DigitalPin::D10_PB4;
		constexpr const DigitalPin D11_PB5_PCI0 = DigitalPin::D11_PB5;
		constexpr const DigitalPin D12_PB6_PCI0 = DigitalPin::D12_PB6;
		constexpr const DigitalPin D13_PB7_PCI0 = DigitalPin::D13_PB7;

		// PJ0-1
		constexpr const DigitalPin D15_PJ0_PCI1 = DigitalPin::D15_PJ0;
		constexpr const DigitalPin D14_PJ1_PCI1 = DigitalPin::D14_PJ1;
		
		// PK0-7
		constexpr const DigitalPin D62_PK0_PCI2 = DigitalPin::D62_PK0;
		constexpr const DigitalPin D63_PK1_PCI2 = DigitalPin::D63_PK1;
		constexpr const DigitalPin D64_PK2_PCI2 = DigitalPin::D64_PK2;
		constexpr const DigitalPin D65_PK3_PCI2 = DigitalPin::D65_PK3;
		constexpr const DigitalPin D66_PK4_PCI2 = DigitalPin::D66_PK4;
		constexpr const DigitalPin D67_PK5_PCI2 = DigitalPin::D67_PK5;
		constexpr const DigitalPin D68_PK6_PCI2 = DigitalPin::D68_PK6;
		constexpr const DigitalPin D69_PK7_PCI2 = DigitalPin::D69_PK7;
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
	void EE_READY_vect(void) __attribute__ ((signal));
}
#endif /* BOARDS_MEGA_HH */
