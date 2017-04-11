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

#ifndef BOARDS_LEONARDO_HH
#define BOARDS_LEONARDO_HH

#include <avr/io.h>
#include <avr/sleep.h>

/* This board is based on ATmega32u4 */
#define BOARD_ATMEGA32U4

// Arduino Leonardo pinout: http://duino4projects.com/wp-content/uploads/2013/04/Ardunio_leonardo_pinout.jpg
namespace board
{
	//====
	// IO
	//====
	enum class Port: uint8_t
	{
		PORT_B = 0,
		PORT_C,
		PORT_D,
		PORT_E,
		PORT_F,
		NONE = 0xFF
	};
	
	/**
	 * Digital pin symbols: 
	 * XX_YYY
	 *	XX pin id on LEONARDO board
	 *	YYY port/pin number on ATmega32u4 MCU
	 */
	enum class DigitalPin: uint8_t
	{
		D0_PD2 = 0,
		D1_PD3,
		D2_PD1,
		D3_PD0,
		D4_PD4,
		D5_PC6,
		D6_PD7,
		D7_PE6,
		D8_PB4,
		D9_PB5,
		D10_PB6,
		D11_PB7,
		D12_PD6,
		D13_PC7,
		
		A0_PF7,
		A1_PF6,
		A2_PF5,
		A3_PF4,
		A4_PF1,
		A5_PF0,
		
		SCK_PB1,
		MOSI_PB2,
		MISO_PB3,
		
		RXLED_PB0,
		TXLED_PD5,
		
		LED = DigitalPin::D13_PC7,
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
		INTERNAL_2_56V
	};
	
	enum class AnalogPin: uint8_t
	{
		A0_ADC7 = 0,
		A1_ADC6,
		A2_ADC5,
		A3_ADC4,
		A4_ADC1,
		A5_ADC0,
		
		A6_D4_ADC8,
		A7_D6_ADC10,
		A8_D8_ADC11,
		A9_D9_ADC12,
		A10_D10_ADC13,
		A11_D12_ADC9,
		
		TEMP,
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
		constexpr const DigitalPin D11_PB7_OC0A = DigitalPin::D11_PB7;
		constexpr const DigitalPin D3_PD0_OC0B = DigitalPin::D3_PD0;
		constexpr const DigitalPin D9_PB5_OC1A = DigitalPin::D9_PB5;
		constexpr const DigitalPin D10_PB6_OC1B = DigitalPin::D10_PB6;
		constexpr const DigitalPin D5_PC6_OC3A = DigitalPin::D5_PC6;
		constexpr const DigitalPin D13_PC7_OC4A = DigitalPin::D13_PC7;
		constexpr const DigitalPin D6_PD7_OC4D = DigitalPin::D6_PD7;
		constexpr const DigitalPin D12_PD6_OC4D = DigitalPin::D12_PD6;
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
		constexpr const DigitalPin D3_PD0_EXT0 = DigitalPin::D3_PD0;
		constexpr const DigitalPin D2_PD1_EXT1 = DigitalPin::D2_PD1;
		constexpr const DigitalPin D0_PD2_EXT2 = DigitalPin::D0_PD2;
		constexpr const DigitalPin D1_PD3_EXT3 = DigitalPin::D1_PD3;
		constexpr const DigitalPin D7_PE6_EXT6 = DigitalPin::D7_PE6;
	};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	namespace InterruptPin
	{
		constexpr const DigitalPin RXLED_PB0_PCI0 = DigitalPin::RXLED_PB0;
		constexpr const DigitalPin SCK_PB1_PCI0 = DigitalPin::SCK_PB1;
		constexpr const DigitalPin MOSI_PB2_PCI0 = DigitalPin::MOSI_PB2;
		constexpr const DigitalPin MISO_PB3_PCI0 = DigitalPin::MISO_PB3;
		constexpr const DigitalPin D8_PB4_PCI0 = DigitalPin::D8_PB4;
		constexpr const DigitalPin D9_PB5_PCI0 = DigitalPin::D9_PB5;
		constexpr const DigitalPin D10_PB6_PCI0 = DigitalPin::D10_PB6;
		constexpr const DigitalPin D11_PB7_PCI0 = DigitalPin::D11_PB7;
	};

	//=======
	// USART
	//=======
	
	enum class USART: uint8_t
	{
		USART1 = 0
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
		TIMER3,
		TIMER4
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
	void INT6_vect(void) __attribute__ ((signal));
	void PCINT0_vect(void) __attribute__ ((signal));
	void SPI_STC_vect(void) __attribute__ ((signal));
	void TIMER0_COMPA_vect(void) __attribute__ ((signal));
	void TIMER0_COMPB_vect(void) __attribute__ ((signal));
	void TIMER0_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_CAPT_vect(void)  __attribute__ ((signal));
	void TIMER1_COMPA_vect(void) __attribute__ ((signal));
	void TIMER1_COMPB_vect(void) __attribute__ ((signal));
	void TIMER1_COMPC_vect(void) __attribute__ ((signal));
	void TIMER1_OVF_vect(void) __attribute__ ((signal));
	void TIMER3_CAPT_vect(void)  __attribute__ ((signal));
	void TIMER3_COMPA_vect(void) __attribute__ ((signal));
	void TIMER3_COMPB_vect(void) __attribute__ ((signal));
	void TIMER3_COMPC_vect(void) __attribute__ ((signal));
	void TIMER3_OVF_vect(void) __attribute__ ((signal));
	void TIMER4_COMPA_vect(void) __attribute__ ((signal));
	void TIMER4_COMPB_vect(void) __attribute__ ((signal));
	void TIMER4_COMPD_vect(void) __attribute__ ((signal));
	void TIMER4_OVF_vect(void) __attribute__ ((signal));
	void TWI_vect(void) __attribute__ ((signal));
	void WDT_vect(void) __attribute__ ((signal));
	void USART1_RX_vect(void) __attribute__ ((signal));
	void USART1_TX_vect(void) __attribute__ ((signal));
	void USART1_UDRE_vect(void) __attribute__ ((signal));
	void EE_READY_vect(void) __attribute__ ((signal));
}

#endif /* BOARDS_LEONARDO_HH */
