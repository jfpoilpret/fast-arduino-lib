//   Copyright 2016-2022 Jean-Francois Poilpret
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

/// @cond attinyx5

/**
 * @file 
 * ATtinyX5 (25, 45, 85) specific features and pins.
 */

#ifndef BOARDS_ATTINYX5_HH
#define BOARDS_ATTINYX5_HH

#include "io.h"
#include "../defines.h"
#include <avr/sleep.h>

/// @cond notdocumented
/* This board is based on ATtinyX5/ATtiny */
#define BOARDS_ATTINYX5
#define BOARD_ATTINY
/// @endcond

/**
 * Defines all types and constants specific to support ATtinyX5 MCU targets.
 */
namespace board
{
	/**
	 * Performs special initialization for ATtinyX5, actually nothing at all.
	 * This must be called first in your `main()` function, even before `sei()`.
	 * In general you should ALWAYS call this function at the beginning of your
	 * `main()` even if you know it will not do anything; this will prevent strange
	 * behaviors when you want to port your code to another MCU target for which
	 * `init()` does perform important initialization, e.g. ATmega32u4 (Arduino
	 * LEONARDO).
	 */
	inline static void init() {}
	
	/**
	 * Defines all available ports of ATtinyX5.
	 */
	enum class Port: uint8_t
	{
		/** Port B (5 IO) */
		PORT_B = 0,
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};

	/**
	 * Defines all available digital input/output pins of ATtinyX5, with 
	 * additional pin imaginary numbering (as for an Arduino board).
	 * Each symbol is in the form `Dxx_Pyz`, where `xx` is the imaginary pin number,
	 * `y` is the port letter (B, C or D) and `z` is the bit number for 
	 * that pin within its port.
	 */
	enum class DigitalPin: uint8_t
	{
		/** Pin PB0 (D0) */
		D0_PB0 = 0,
		/** Pin PB1 (D1) */
		D1_PB1,
		/** Pin PB2 (D2) */
		D2_PB2,
		/** Pin PB3 (D3) */
		D3_PB3,
		/** Pin PB4 (D4) */
		D4_PB4,
		/** Shortcut for LED pin, arbitrarily chosen to be D0 */
		LED = D0_PB0,
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};

	/**
	 * Defines available clocks of ATtinyX5, used for analog input.
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
	 * Defines available voltage references of ATtinyX5, used for analog input.
	 */
	enum class AnalogReference: uint8_t
	{
		/** Voltage reference is given by the `AREF` (PB0) pin. */
		AREF = 0,
		/** Voltage reference is given by the `Vcc` pin. */
		AVCC,
		/** 
		 * Voltage reference is internal 1.1V reference, generated from the
		 * internal bandgap reference. 
		 */
		INTERNAL_1_1V,
		/** 
		 * Voltage reference is internal 2.56V reference, without external
		 * bypass capacitor, disconnected from PB0 (AREF).
		 */
		INTERNAL_2_56V,
		/** 
		 * Voltage reference is internal 2.56V reference, with external
		 * bypass capacitor at PB0 (AREF).
		 */
		INTERNAL_2_56V_BYPASS_CAP
	};
	
	/**
	 * Defines all available analog input pins of ATtinyX5, with 
	 * reference to ATtinyX5 pins.
	 * Note that this includes also other sources than pin, e.g. the internal
	 * bandgap reference or the temperature sensor.
	 */
	enum class AnalogPin: uint8_t
	{
		/** Pin ADC1 (PB2) */
		A1 = 0,
		/** Pin ADC2 (PB4) */
		A2,
		/** Pin ADC3 (PB3) */
		A3,
		/** Bandgap reference */
		BANDGAP,
		/** Ground reference */
		GND,
		/** Temperature sensor */
		TEMP,
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};
	
	/**
	 * Defines all digital output pins of ATtinyX5, capable of PWM output.
	 * Each symbol is in the form `Dxx_Pyz_OCuv`, where `xx` is the imaginary pin 
	 * number , `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port, `u` is the number of the timer used by this PWM 
	 * pin and `v` the letter indicating which compare register of the timer this 
	 * PWM pin is mapped to.
	 */
	enum class PWMPin : uint8_t
	{
		D0_PB0_OC0A = 0,
		D1_PB1_OC0B,
		//TODO Currently PWM for Timer1 is not supported (setup is too much
		// different compared to other MCU, even ATtinyX4)
		// constexpr const DigitalPin D4_PB4_OC1B = DigitalPin::D4_PB4;
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};
	
	/**
	 * Defines all digital output pins of ATtinyX5, usable as direct external 
	 * interrupt pins.
	 * Each symbol is in the form `Dxx_Pyz_EXTu`, where `xx` is the imaginary pin 
	 * number , `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the interrupt for that
	 * pin.
	 */
	enum class ExternalInterruptPin : uint8_t
	{
		D2_PB2_EXT0 = 0
	};

	/**
	 * Defines all digital output pins of ATtinyX5, usable as pin change 
	 * interrupt (PCI) pins.
	 * Each symbol is in the form `Dxx_Pyz_PCIu`, where `xx` is the imaginary pin 
	 * number, `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the PCI vector for that
	 * pin.
	 */
	enum class InterruptPin : uint8_t
	{
		D0_PB0_PCI0 = uint8_t(DigitalPin::D0_PB0),
		D1_PB1_PCI0 = uint8_t(DigitalPin::D1_PB1),
		D2_PB2_PCI0 = uint8_t(DigitalPin::D2_PB2),
		D3_PB3_PCI0 = uint8_t(DigitalPin::D3_PB3),
		D4_PB4_PCI0 = uint8_t(DigitalPin::D4_PB4)
	};

	/**
	 * Defines all USART modules of ATtinyX5, actually none at all.
	 */
	enum class USART: uint8_t
	{
	};
	
	/**
	 * Defines all timers available for ATtinyX5.
	 */
	enum class Timer: uint8_t
	{
		/** Timer0 (8 bits) */
		TIMER0 = 0,
		/** Timer1 (8 bits) */
		TIMER1 = 1
	};

	/**
	 * Defines all available sleep modes for ATmega328P.
	 */
	enum class SleepMode: uint8_t
	{
		/** 
		 * In this mode, CPU is stopped but all other peripherals and interrupts
		 * work normally. In this mode, current consumption is reduced to about 
		 * 25% of active mode consumption.
		 */
		IDLE = SLEEP_MODE_IDLE,
		/** 
		 * In this mode, CPU is stopped but other peripherals and interrupts
		 * work normally, except IO. This mode is actually very similar to `IDLE`.
		 */
		ADC_NOISE_REDUCTION = SLEEP_MODE_ADC,
		/**
		 * In this mode, everything is stopped (including oscillator) but external
		 * interrupts, USI start condition (if enabled) and Watchdog Timer (if enabled).
		 * This is lowest current consumption mode, typically a few uA, depending
		 * on other factors (voltage, watchdog enabled or not).
		 * Waking up from this mode may take significant time until internal 
		 * oscillator is restarted and stabilized; refer to datasheet for more
		 * detailed data (look for `Start-up times`).
		 */
		POWER_DOWN = SLEEP_MODE_PWR_DOWN,
		/**
		 * This mode is exactly the same `POWER_DOWN`.
		 */
		POWER_SAVE = SLEEP_MODE_PWR_DOWN,
		/**
		 * This mode is exactly the same `POWER_DOWN`.
		 */
		STANDBY = SLEEP_MODE_PWR_DOWN,
		/**
		 * This mode is exactly the same `POWER_SAVE`.
		 */
		EXTENDED_STANDBY = SLEEP_MODE_PWR_DOWN
	};
};

/// @cond notdocumented
#define ANALOG_COMP_vect ANA_COMP_vect
#define EE_READY_vect EE_RDY_vect

// Forward declare interrupt service routines to allow them as friends.
extern "C" {
	void ADC_vect(void) SIGNAL_HANDLER;
	void ANALOG_COMP_vect(void) SIGNAL_HANDLER;
	void INT0_vect(void) SIGNAL_HANDLER;
	void PCINT0_vect(void) SIGNAL_HANDLER;
	void TIMER0_COMPA_vect(void) SIGNAL_HANDLER;
	void TIMER0_COMPB_vect(void) SIGNAL_HANDLER;
	void TIMER0_OVF_vect(void) SIGNAL_HANDLER;
	void TIMER1_COMPA_vect(void) SIGNAL_HANDLER;
	void TIMER1_COMPB_vect(void) SIGNAL_HANDLER;
	void TIMER1_OVF_vect(void) SIGNAL_HANDLER;
	void WDT_vect(void) SIGNAL_HANDLER;
	void USI_START_vect(void) SIGNAL_HANDLER;
	void USI_OVF_vect(void) SIGNAL_HANDLER;
	void EE_READY_vect(void) SIGNAL_HANDLER;
}
/// @endcond

#endif /* BOARDS_ATTINYX5_HH */
/// @endcond
