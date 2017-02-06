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

#ifndef BOARDS_ATTINYX4_HH
#define BOARDS_ATTINYX4_HH

#include <avr/io.h>
#include <avr/sleep.h>

/* This board is based on ATtinyX4/ATtiny */
#define BOARDS_ATTINYX4
#define BOARD_ATTINY

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
		INTERNAL_1_1V
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
		TEMP,
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
		constexpr const DigitalPin D10_EXT0 = DigitalPin::D10;		// PB2
	};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	namespace InterruptPin
	{
		constexpr const DigitalPin D0_PCI0 = DigitalPin::D0;
		constexpr const DigitalPin D1_PCI0 = DigitalPin::D1;
		constexpr const DigitalPin D2_PCI0 = DigitalPin::D2;
		constexpr const DigitalPin D3_PCI0 = DigitalPin::D3;
		constexpr const DigitalPin D4_PCI0 = DigitalPin::D4;
		constexpr const DigitalPin D5_PCI0 = DigitalPin::D5;
		constexpr const DigitalPin D6_PCI0 = DigitalPin::D6;
		constexpr const DigitalPin D7_PCI0 = DigitalPin::D7;
		
		constexpr const DigitalPin D8_PCI1 = DigitalPin::D8;
		constexpr const DigitalPin D9_PCI1 = DigitalPin::D9;
		constexpr const DigitalPin D10_PCI1 = DigitalPin::D10;
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
	
	// Nothing special

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
