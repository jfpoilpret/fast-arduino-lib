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

/// @cond attinyx4

/**
 * @file 
 * ATtinyX4 (24, 44, 84) specific features and pins.
 */

#ifndef BOARDS_ATTINYX4_HH
#define BOARDS_ATTINYX4_HH

#include <avr/io.h>
#include <avr/sleep.h>

/// @cond notdocumented
/* This board is based on ATtinyX4/ATtiny */
#define BOARDS_ATTINYX4
#define BOARD_ATTINY

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif
/// @endcond

/**
 * Defines all types and constants specific to support ATtinyX4 MCU targets.
 */
namespace board
{
	/**
	 * Performs special initialization for ATtinyX4, actually nothing at all.
	 * This must be called first in your `main()` function, even before `sei()`.
	 * In general you should ALWAYS call this function at the beginning of your
	 * `main()` even if you know it will not do anything; this will prevent strange
	 * behaviors when you want to port your code to another MCU target for which
	 * `init()` does perform important initialization, e.g. ATmega32u4 (Arduino
	 * LEONARDO).
	 */
	inline static void init() {}
	
	/**
	 * Defines all available ports of ATtinyX4.
	 */
	enum class Port: uint8_t
	{
		/** Port A (8 IO) */
		PORT_A = 0,
		/** Port B (3 IO) */
		PORT_B,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};

	/**
	 * Defines all available digital input/output pins of ATtinyX4, with 
	 * additional pin imaginary numbering (as for an Arduino board).
	 * Each symbol is in the form `Dxx_Pyz`, where `xx` is the imaginary pin number,
	 * `y` is the port letter (B, C or D) and `z` is the bit number for 
	 * that pin within its port.
	 */
	enum class DigitalPin: uint8_t
	{
		/** Pin PA0 (D0) */
		D0_PA0 = 0,			// PA0
		/** Pin PA1 (D1) */
		D1_PA1,				// PA1
		/** Pin PA2 (D2) */
		D2_PA2,				// PA2
		/** Pin PA3 (D3) */
		D3_PA3,				// PA3
		/** Pin PA4 (D4) */
		D4_PA4,				// PA4
		/** Pin PA5 (D5) */
		D5_PA5,				// PA5
		/** Pin PA6 (D6) */
		D6_PA6,				// PA6
		/** Pin PA6 (D7) */
		D7_PA7,				// PA7
		/** Pin PB0 (D8) */
		D8_PB0,				// PB0
		/** Pin PB1 (D9) */
		D9_PB1,				// PB1
		/** Pin PB2 (D10) */
		D10_PB2,			// PB2
		/** Shortcut for LED pin, arbitrarily chosen to be D7 */
		LED = DigitalPin::D7_PA7,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};

	/**
	 * Defines available clocks of ATtinyX4, used for analog input.
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
	 * Defines available voltage references of ATtinyX4, used for analog input.
	 */
	enum class AnalogReference: uint8_t
	{
		/** Voltage reference is given by the `AREF` (PA0) pin. */
		AREF = 0,
		/** Voltage reference is given by the `Vcc` pin. */
		AVCC,
		/** Voltage reference is internal 1.1V reference, generated from the
		 * internal bandgap reference. */
		INTERNAL_1_1V
	};
	
	/**
	 * Defines all available analog input pins of ATtinyX4, with 
	 * reference to Arduino UNO pins.
	 * Note that this includes also other sources than pin, e.g. the internal
	 * bandgap reference or the temperature sensor.
	 */
	enum class AnalogPin: uint8_t
	{
		/** Pin ADC0 (PA0) */
		A0 = 0,
		/** Pin ADC1 (PA1) */
		A1,
		/** Pin ADC2 (PA2) */
		A2,
		/** Pin ADC3 (PA3) */
		A3,
		/** Pin ADC4 (PA4) */
		A4,
		/** Pin ADC5 (PA5) */
		A5,
		/** Pin ADC6 (PA6) */
		A6,
		/** Pin ADC7 (PA7) */
		A7,
		/** Temperature sensor */
		TEMP,
		/** Bandgap reference */
		BANDGAP,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};
	
	/**
	 * Defines all digital output pins of ATtinyX4, capable of PWM output.
	 * Each symbol is in the form `Dxx_Pyz_OCuv`, where `xx` is the imaginary pin 
	 * number , `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port, `u` is the number of the timer used by this PWM 
	 * pin and `v` the letter indicating which compare register of the timer this 
	 * PWM pin is mapped to.
	 */
	namespace PWMPin
	{
		constexpr const DigitalPin D10_PB2_OC0A = DigitalPin::D10_PB2;
		constexpr const DigitalPin D7_PA7_OC0B = DigitalPin::D7_PA7;
		constexpr const DigitalPin D6_PA6_OC1A = DigitalPin::D6_PA6;
		constexpr const DigitalPin D5_PA5_OC1B = DigitalPin::D5_PA5;
	};
	
	/**
	 * Defines all digital output pins of ATtinyX4, usable as direct external 
	 * interrupt pins.
	 * Each symbol is in the form `Dxx_Pyz_EXTu`, where `xx` is the imaginary pin 
	 * number , `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the interrupt for that
	 * pin.
	 * This namespace exists for the sole purpose of quickly finding an external
	 * interrupt pin among all digital IO pins.
	 */
	namespace ExternalInterruptPin
	{
		constexpr const DigitalPin D10_PB2_EXT0 = DigitalPin::D10_PB2;		// PB2
	};

	/**
	 * Defines all digital output pins of ATtinyX4, usable as pin change 
	 * interrupt (PCI) pins.
	 * Each symbol is in the form `Dxx_Pyz_PCIu`, where `xx` is the imaginary pin 
	 * number, `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the PCI vector for that
	 * pin.
	 * This namespace exists for the sole purpose of quickly finding an pin change
	 * interrupt pin among all digital IO pins.
	 */
	namespace InterruptPin
	{
		constexpr const DigitalPin D0_PA0_PCI0 = DigitalPin::D0_PA0;
		constexpr const DigitalPin D1_PA1_PCI0 = DigitalPin::D1_PA1;
		constexpr const DigitalPin D2_PA2_PCI0 = DigitalPin::D2_PA2;
		constexpr const DigitalPin D3_PA3_PCI0 = DigitalPin::D3_PA3;
		constexpr const DigitalPin D4_PA4_PCI0 = DigitalPin::D4_PA4;
		constexpr const DigitalPin D5_PA5_PCI0 = DigitalPin::D5_PA5;
		constexpr const DigitalPin D6_PA6_PCI0 = DigitalPin::D6_PA6;
		constexpr const DigitalPin D7_PA7_PCI0 = DigitalPin::D7_PA7;
		
		constexpr const DigitalPin D8_PB0_PCI1 = DigitalPin::D8_PB0;
		constexpr const DigitalPin D9_PB1_PCI1 = DigitalPin::D9_PB1;
		constexpr const DigitalPin D10_PB2_PCI1 = DigitalPin::D10_PB2;
	};

	/**
	 * Defines all USART modules of ATtinyX4, actually none at all.
	 */
	enum class USART: uint8_t
	{
	};
	
	// IMPORTANT: on my setup, Timer runs faster than expected (9.5s for 10s)
	//TODO check how we can calibrate clock?
	/**
	 * Defines all timers available for ATtinyX4.
	 */
	enum class Timer: uint8_t
	{
		/** Timer0 (8 bits) */
		TIMER0,
		/** Timer1 (16 bits) */
		TIMER1
	};

	/// @cond notdocumented
    #define SLEEP_MODE_PWR_SAVE     (_BV(SM0) | _BV(SM1))
	/// @endcond
	
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
		 * interrupts, I2C slave (if enabled) and Watchdog Timer (if enabled).
		 * This is lowest current consumption mode, typically a few uA, depending
		 * on other factors (voltage, watchdog enabled or not).
		 * Waking up from this mode may take significant time until internal 
		 * oscillator is restarted and stabilized; refer to datasheet for more
		 * detailed data (look for `Start-up times`).
		 */
		POWER_DOWN = SLEEP_MODE_PWR_DOWN,
		/**
		 * This mode is similar to `POWER_DOWN`, except Timer2 is still running
		 * if enabled.
		 * Waking up from this mode may take significant time until internal 
		 * oscillator is restarted and stabilized; refer to datasheet for more
		 * detailed data (look for `Start-up times`).
		 */
		POWER_SAVE = SLEEP_MODE_PWR_SAVE,
		/**
		 * This mode is exactly the same `POWER_DOWN`.
		 */
		STANDBY = SLEEP_MODE_PWR_SAVE,
		/**
		 * This mode is exactly the same `POWER_SAVE`.
		 */
		EXTENDED_STANDBY = SLEEP_MODE_PWR_SAVE,
		
		//FIXME it seems that this enum value is just plain useless, why did I
		// introduce it? If no justification, then simply remove it!
		DEFAULT_MODE = 0xFF
	};
};

/// @cond notdocumented
#define ANALOG_COMP_vect ANA_COMP_vect
#define TIMER0_OVF_vect TIM0_OVF_vect
#define TIMER0_COMPA_vect TIM0_COMPA_vect
#define TIMER0_COMPB_vect TIM0_COMPB_vect
#define TIMER1_OVF_vect TIM1_OVF_vect
#define TIMER1_COMPA_vect TIM1_COMPA_vect
#define TIMER1_COMPB_vect TIM1_COMPB_vect
#define EE_READY_vect EE_RDY_vect

// Forward declare interrupt service routines to allow them as friends.
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
	void EE_READY_vect(void) __attribute__ ((signal));
}
/// @endcond

#endif /* BOARDS_ATTINYX4_HH */
/// @endcond
