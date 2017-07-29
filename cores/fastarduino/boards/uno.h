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

/// @cond uno

/**
 * @file 
 * ATmega328P specific features and pins.
 */

#ifndef BOARDS_UNO_HH
#define BOARDS_UNO_HH

#include <avr/io.h>
#include <avr/sleep.h>

/* This board is based on ATmega328P */
#define BOARD_ATMEGA328P

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif

namespace board
{
	/**
	 * Performs special initialization for ATmega328P, actually nothing at all.
	 * This must be called first in your `main()` function, even `sei()`.
	 */
	inline static void init() {}
	
	/**
	 * Defines all available ports of ATmega328P.
	 */
	enum class Port: uint8_t
	{
		/** Port B (7 IO) */
		PORT_B = 0,
		/** Port C (6 IO) */
		PORT_C,
		/** Port D (8 IO) */
		PORT_D,
		/** FastArduino internal: DO NOT USE */
		NONE = 0xFF
	};
	
	/**
	 * Defines all available digital input/output pins of ATmega328P, with 
	 * reference to Arduino UNO pins.
	 * Each symbol is in the form `Dxx_Pyz`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, c or D) and `z` is the bit number for 
	 * that pin within its port.
	 */
	enum class DigitalPin: uint8_t
	{
		D0_PD0 = 0,			// PD0
		D1_PD1,				// PD1
		D2_PD2,				// PD2
		D3_PD3,				// PD3
		D4_PD4,				// PD4
		D5_PD5,				// PD5
		D6_PD6,				// PD6
		D7_PD7,				// PD7
		D8_PB0,				// PB0
		D9_PB1,				// PB1
		D10_PB2,			// PB2
		D11_PB3,			// PB3
		D12_PB4,			// PB4
		D13_PB5,			// PB5
		D14_PC0,			// PC0-A0
		D15_PC1,			// PC1-A1
		D16_PC2,			// PC2-A2
		D17_PC3,			// PC3-A3
		D18_PC4,			// PC4-A4
		D19_PC5,			// PC5-A5
		/** Shortcut for LED pin on Arduino */
		LED = DigitalPin::D13_PB5,
		/** FastArduino internal: DO NOT USE */
		NONE = 0xFF
	};

	/**
	 * Defines available clocks of ATmega328P, used for analog input.
	 */
	enum class AnalogClock: uint8_t
	{
		MAX_FREQ_50KHz = 0,
		MAX_FREQ_100KHz,
		MAX_FREQ_200KHz,
		MAX_FREQ_500KHz,
		MAX_FREQ_1MHz
	};
	
	/**
	 * Defines available voltage references of ATmega328P, used for analog input.
	 */
	enum class AnalogReference: uint8_t
	{
		/** Voltage reference is given by the `AREF` pin. */
		AREF = 0,
		/** Voltage reference is given by the `AVcc` pin. */
		AVCC,
		/** Voltage reference is internal 1.1V reference, generated from the
		 * internal bandgap reference. */
		INTERNAL_1_1V
	};
	
	/**
	 * Defines all available analog input pins of ATmega328P, with 
	 * reference to Arduino UNO pins.
	 * Note that this includes also other sources than pin, e.g. the internal
	 * bandgap reference or the temperature sensor.
	 */
	enum class AnalogPin: uint8_t
	{
		A0 = 0,
		A1,
		A2,
		A3,
		A4,
		A5,
#ifdef HAS_8_ANALOG_INPUTS
		A6,
		A7,
#endif
		/** Temperature sensor */
		TEMP,
		/** Bandgap reference */
		BANDGAP,
		/** FastArduino internal: DO NOT USE */
		NONE = 0xFF
	};
	
	//============
	// PWM output
	//============
	/**
	 * Defines all digital output pins of ATmega328P, capable of PWM output.
	 * Each symbol is in the form `Dxx_Pyz_OCuv`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, c or D), `z` is the bit number for 
	 * that pin within its port, `u` is the number of the timer used by this PWM 
	 * pin and `v` the letter indicating which compare register of the timer this 
	 * PWM pin is mapped to.
	 */
	namespace PWMPin
	{
		constexpr const DigitalPin D6_PD6_OC0A = DigitalPin::D6_PD6;
		constexpr const DigitalPin D5_PD5_OC0B = DigitalPin::D5_PD5;
		constexpr const DigitalPin D9_PB1_OC1A = DigitalPin::D9_PB1;
		constexpr const DigitalPin D10_PB2_OC1B = DigitalPin::D10_PB2;
		constexpr const DigitalPin D11_PB3_OC2A = DigitalPin::D11_PB3;
		constexpr const DigitalPin D3_PD3_OC2B = DigitalPin::D3_PD3;
	};
	
	
	/**
	 * Defines all digital output pins of ATmega328P, usable as direct external 
	 * interrupt pins.
	 * Each symbol is in the form `Dxx_Pyz_EXTu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, c or D), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the interrupt for that
	 * pin.
	 */
	namespace ExternalInterruptPin
	{
		constexpr const DigitalPin D2_PD2_EXT0 = DigitalPin::D2_PD2;		// PD2
		constexpr const DigitalPin D3_PD3_EXT1 = DigitalPin::D3_PD3;		// PD3
	};

	/**
	 * Defines all digital output pins of ATmega328P, usable as pin change 
	 * interrupt (PCI) pins.
	 * Each symbol is in the form `Dxx_Pyz_PCIu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, c or D), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the PCI vector for that
	 * pin.
	 */
	namespace InterruptPin
	{
		constexpr const DigitalPin D0_PD0_PCI2 = DigitalPin::D0_PD0;
		constexpr const DigitalPin D1_PD1_PCI2 = DigitalPin::D1_PD1;
		constexpr const DigitalPin D2_PD2_PCI2 = DigitalPin::D2_PD2;
		constexpr const DigitalPin D3_PD3_PCI2 = DigitalPin::D3_PD3;
		constexpr const DigitalPin D4_PD4_PCI2 = DigitalPin::D4_PD4;
		constexpr const DigitalPin D5_PD5_PCI2 = DigitalPin::D5_PD5;
		constexpr const DigitalPin D6_PD6_PCI2 = DigitalPin::D6_PD6;
		constexpr const DigitalPin D7_PD7_PCI2 = DigitalPin::D7_PD7;
		
		constexpr const DigitalPin D8_PB0_PCI0 = DigitalPin::D8_PB0;
		constexpr const DigitalPin D9_PB1_PCI0 = DigitalPin::D9_PB1;
		constexpr const DigitalPin D10_PB2_PCI0 = DigitalPin::D10_PB2;
		constexpr const DigitalPin D11_PB3_PCI0 = DigitalPin::D11_PB3;
		constexpr const DigitalPin D12_PB4_PCI0 = DigitalPin::D12_PB4;
		constexpr const DigitalPin D13_PB5_PCI0 = DigitalPin::D13_PB5;
		
		constexpr const DigitalPin D14_PC0_PCI1 = DigitalPin::D14_PC0;
		constexpr const DigitalPin D15_PC1_PCI1 = DigitalPin::D15_PC1;
		constexpr const DigitalPin D16_PC2_PCI1 = DigitalPin::D16_PC2;
		constexpr const DigitalPin D17_PC3_PCI1 = DigitalPin::D17_PC3;
		constexpr const DigitalPin D18_PC4_PCI1 = DigitalPin::D18_PC4;
		constexpr const DigitalPin D19_PC5_PCI1 = DigitalPin::D19_PC5;
	};

	/**
	 * Defines all USART modules of ATmega328P, actually only one.
	 */
	enum class USART: uint8_t
	{
		USART0 = 0
	};
	
	/**
	 * Defines all timers available for ATmega328P.
	 */
	enum class Timer: uint8_t
	{
		/** Timer0 (8 bits) */
		TIMER0,
		/** Timer1 (16 bits) */
		TIMER1,
		/** Timer2 (8 bits) */
		TIMER2
	};
	
	/**
	 * Defines all available sleep modes for ATmega328P.
	 */
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
	void EE_READY_vect(void) __attribute__ ((signal));
}

#define USART0_RX_vect USART_RX_vect
#define USART0_TX_vect USART_TX_vect
#define USART0_UDRE_vect USART_UDRE_vect

#endif /* BOARDS_UNO_HH */
/// @endcond
