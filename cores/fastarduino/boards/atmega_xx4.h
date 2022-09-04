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

/// @cond atmegaxx4

/**
 * @file 
 * ATmega644/1284 specific features and pins.
 */

#ifndef BOARDS_ATMEGA_XX4_HH
#define BOARDS_ATMEGA_XX4_HH

#include "io.h"
#include "../defines.h"
#include <avr/sleep.h>

/// @cond notdocumented
/* This board is based on ATmega644 or 1284 */
#define BOARDS_ATMEGAXX4
/// @endcond

/**
 * Defines all types and constants specific to support Arduino MEGA board
 * (ATmega644 MCU target).
 */
namespace board
{
	/**
	 * Performs special initialization for ATmega644, actually nothing at all.
	 * This must be called first in your `main()` function, even before `sei()`.
	 * In general you should ALWAYS call this function at the beginning of your
	 * `main()` even if you know it will not do anything; this will prevent strange
	 * behaviors when you want to port your code to another MCU target for which
	 * `init()` does perform important initialization, e.g. ATmega32u4 (Arduino
	 * LEONARDO).
	 */
	inline static void init() {}
	
	/**
	 * Defines all available ports of ATmega644.
	 */
	enum class Port: uint8_t
	{
		/** Port A (8 IO) */
		PORT_A = 0,
		/** Port B (8 IO) */
		PORT_B,
		/** Port C (8 IO) */
		PORT_C,
		/** Port D (8 IO) */
		PORT_D,
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};
	
	/**
	 * Defines all available digital input/output pins of ATmega644, with 
	 * reference to Arduino MEGA pins. Only Arduino MEGA pins are listed here.
	 * Each symbol is in the form `Dxx_Pyz`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter and `z` is the bit number for 
	 * that pin within its port.
	 */
	enum class DigitalPin: uint8_t
	{
		/** Pin PA0 (ADC0/PCINT0) */
		D0_PA0 = 0,
		/** Pin PA1 (ADC1/PCINT1) */
		D1_PA1,
		/** Pin PA2 (ADC2/PCINT2) */
		D2_PA2,
		/** Pin PA3 (ADC3/PCINT3) */
		D3_PA3,
		/** Pin PA4 (ADC4/PCINT4) */
		D4_PA4,
		/** Pin PA5 (ADC5/PCINT5) */
		D5_PA5,
		/** Pin PA6 (ADC6/PCINT6) */
		D6_PA6,
		/** Pin PA7 (ADC7/PCINT7) */
		D7_PA7,

		/** Pin PB0 (PCINT8) */
		D8_PB0,
		/** Pin PB1 (PCINT9) */
		D9_PB1,
		/** Pin PB2 (PCINT10/INT2) */
		D10_PB2,
		/** Pin PB3 (PCINT11/OC0A) */
		D11_PB3,
		/** Pin PB4 (PCINT12/OC0B/SS) */
		D12_PB4,
		/** Pin PB5 (PCINT13/MOSI) */
		D13_PB5,
		/** Pin PB6 (PCINT14/MISO) */
		D14_PB6,
		/** Pin PB7 (PCINT15/SCK) */
		D15_PB7,

		/** Pin PC0 (PCINT16/SCL) */
		D16_PC0,
		/** Pin PC1 (PCINT17/SDA) */
		D17_PC1,
		/** Pin PC2 (PCINT18) */
		D18_PC2,
		/** Pin PC3 (PCINT19) */
		D19_PC3,
		/** Pin PC4 (PCINT20) */
		D20_PC4,
		/** Pin PC5 (PCINT21) */
		D21_PC5,
		/** Pin PC6 (PCINT22) */
		D22_PC6,
		/** Pin PC7 (PCINT23) */
		D23_PC7,

		/** Pin PD0 (PCINT24/RX0) */
		D24_PD0,
		/** Pin PD1 (PCINT25/TX0) */
		D25_PD1,
		/** Pin PD2 (PCINT26/RX1/INT0) */
		D26_PD2,
		/** Pin PD3 (PCINT27/TX1/INT1) */
		D27_PD3,
		/** Pin PD4 (PCINT28/OC1B) */
		D28_PD4,
		/** Pin PD5 (PCINT29/OC1A) */
		D29_PD5,
		/** Pin PD6 (PCINT30/OC2B/ICP) */
		D30_PD6,
		/** Pin PD7 (PCINT31/OC2A) */
		D31_PD7,

		/** Shortcut for LED pin (PB0 on pin 1 for ATmega644 PDIP) */
		LED = D8_PB0,
		// FastArduino internal: DO NOT USE
		NONE = 0XFF
	};

	/**
	 * Defines available clocks of ATmega644, used for analog input.
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
	 * Defines available voltage references of ATmega644, used for analog input.
	 */
	enum class AnalogReference: uint8_t
	{
		/** Voltage reference is given by the `AREF` pin. */
		AREF = 0,
		/** Voltage reference is given by the `AVcc` pin. */
		AVCC,
		/** Voltage reference is internal 1.1V reference, generated from the
		 * internal bandgap reference. */
		INTERNAL_1_1V,
		/** Voltage reference is internal 2.56V reference, generated from the
		 * internal bandgap reference. */
		INTERNAL_2_56V
	};
	
	/**
	 * Defines all available analog input pins of ATmega644, with 
	 * reference to Arduino MEGA pins.
	 * Note that this includes also other sources than pin, e.g. the internal
	 * bandgap reference.
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
		/** Bandgap reference */
		BANDGAP,
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};
	
	/**
	 * Defines all digital output pins of ATmega644, capable of PWM output.
	 * Each symbol is in the form `Dxx_Pyz_OCuv`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter, `z` is the bit number for 
	 * that pin within its port, `u` is the number of the timer used by this PWM 
	 * pin and `v` the letter indicating which compare register of the timer this 
	 * PWM pin is mapped to.
	 */
	enum class PWMPin : uint8_t
	{
		D11_PB3_OC0A = 0,
		D12_PB4_OC0B,
		D28_PD4_OC1B,
		D29_PD5_OC1A,
		D30_PD6_OC2B,
		D31_PD7_OC2A,

#ifdef __AVR_ATmega1284P__
		D14_PB6_OC3A,
		D15_PB7_OC3B,
#endif
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};
	
	/**
	 * Defines all digital output pins of ATmega644, usable as direct external 
	 * interrupt pins.
	 * Each symbol is in the form `Dxx_Pyz_EXTu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter, `z` is the bit number for 
	 * that pin within its port and `u` is the number of the interrupt for that
	 * pin.
	 */
	enum class ExternalInterruptPin : uint8_t
	{
		D26_PD2_EXT0 = 0,
		D27_PD3_EXT1,
		D10_PB2_EXT2
	};

	/**
	 * Defines all digital output pins of ATmega644, usable as pin change 
	 * interrupt (PCI) pins.
	 * Each symbol is in the form `Dxx_Pyz_PCIu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter, `z` is the bit number for 
	 * that pin within its port and `u` is the number of the PCI vector for that
	 * pin.
	 */
	enum class InterruptPin : uint8_t
	{
		// PA0-7
		D0_PA0_PCI0 = uint8_t(DigitalPin::D0_PA0),
		D1_PA1_PCI0 = uint8_t(DigitalPin::D1_PA1),
		D2_PA2_PCI0 = uint8_t(DigitalPin::D2_PA2),
		D3_PA3_PCI0 = uint8_t(DigitalPin::D3_PA3),
		D4_PA4_PCI0 = uint8_t(DigitalPin::D4_PA4),
		D5_PA5_PCI0 = uint8_t(DigitalPin::D5_PA5),
		D6_PA6_PCI0 = uint8_t(DigitalPin::D6_PA6),
		D7_PA7_PCI0 = uint8_t(DigitalPin::D7_PA7),

		// PB0-7
		D8_PB0_PCI1 = uint8_t(DigitalPin::D8_PB0),
		D9_PB1_PCI1 = uint8_t(DigitalPin::D9_PB1),
		D10_PB2_PCI1 = uint8_t(DigitalPin::D10_PB2),
		D11_PB3_PCI1 = uint8_t(DigitalPin::D11_PB3),
		D12_PB4_PCI1 = uint8_t(DigitalPin::D12_PB4),
		D13_PB5_PCI1 = uint8_t(DigitalPin::D13_PB5),
		D14_PB6_PCI1 = uint8_t(DigitalPin::D14_PB6),
		D15_PB7_PCI1 = uint8_t(DigitalPin::D15_PB7),

		// PC0-7
		D16_PC0_PCI2 = uint8_t(DigitalPin::D16_PC0),
		D17_PC1_PCI2 = uint8_t(DigitalPin::D17_PC1),
		D18_PC2_PCI2 = uint8_t(DigitalPin::D18_PC2),
		D19_PC3_PCI2 = uint8_t(DigitalPin::D19_PC3),
		D20_PC4_PCI2 = uint8_t(DigitalPin::D20_PC4),
		D21_PC5_PCI2 = uint8_t(DigitalPin::D21_PC5),
		D22_PC6_PCI2 = uint8_t(DigitalPin::D22_PC6),
		D23_PC7_PCI2 = uint8_t(DigitalPin::D23_PC7),

		// PD0-7
		D24_PD0_PCI3 = uint8_t(DigitalPin::D24_PD0),
		D25_PD1_PCI3 = uint8_t(DigitalPin::D25_PD1),
		D26_PD2_PCI3 = uint8_t(DigitalPin::D26_PD2),
		D27_PD3_PCI3 = uint8_t(DigitalPin::D27_PD3),
		D28_PD4_PCI3 = uint8_t(DigitalPin::D28_PD4),
		D29_PD5_PCI3 = uint8_t(DigitalPin::D29_PD5),
		D30_PD6_PCI3 = uint8_t(DigitalPin::D30_PD6),
		D31_PD7_PCI3 = uint8_t(DigitalPin::D31_PD7),
	};

	/**
	 * Defines all USART modules of ATmega644.
	 */
	enum class USART: uint8_t
	{
		/** USART0 is connected to pins D24_PD0 (RX0) and D25_PD1 (TX0). */
		USART0 = 0,
		/** USART1 is connected to pins D26_PD2 (RX1) and D27_PD3 (TX1). */
		USART1 = 1
	};

	/**
	 * Defines all timers available for ATmega644.
	 */
	enum class Timer: uint8_t
	{
		/** Timer0 (8 bits) */
		TIMER0 = 0,
		/** Timer1 (16 bits) */
		TIMER1 = 1,
		/** Timer2 (8 bits) */
		TIMER2 = 2,
#ifdef __AVR_ATmega1284P__
		/** Timer3 (16 bits) */
		TIMER3 = 3
#endif
	};
	
	/**
	 * Defines all available sleep modes for ATmega644.
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
		 * This mode is similar to `POWER_DOWN`, except the oscillator is kept
		 * running, hence waking up from this mode takes only 6 clock cycles.
		 */
		STANDBY = SLEEP_MODE_STANDBY,
		/**
		 * This mode is similar to `POWER_SAVE`, except the oscillator is kept
		 * running, hence waking up from this mode takes only 6 clock cycles.
		 */
		EXTENDED_STANDBY = SLEEP_MODE_EXT_STANDBY
	};
};

/// @cond notdocumented
// Forward declare interrupt service routines to allow them as friends.
extern "C" {
	void ADC_vect(void) SIGNAL_HANDLER;
	void ANALOG_COMP_vect(void) SIGNAL_HANDLER;
	void INT0_vect(void) SIGNAL_HANDLER;
	void INT1_vect(void) SIGNAL_HANDLER;
	void INT2_vect(void) SIGNAL_HANDLER;
	void PCINT0_vect(void) SIGNAL_HANDLER;
	void PCINT1_vect(void) SIGNAL_HANDLER;
	void PCINT2_vect(void) SIGNAL_HANDLER;
	void PCINT3_vect(void) SIGNAL_HANDLER;
	void SPI_STC_vect(void) SIGNAL_HANDLER;
	void TIMER0_COMPA_vect(void) SIGNAL_HANDLER;
	void TIMER0_COMPB_vect(void) SIGNAL_HANDLER;
	void TIMER0_OVF_vect(void) SIGNAL_HANDLER;
	void TIMER1_COMPA_vect(void) SIGNAL_HANDLER;
	void TIMER1_COMPB_vect(void) SIGNAL_HANDLER;
	void TIMER1_OVF_vect(void) SIGNAL_HANDLER;
	void TIMER1_CAPT_vect(void)  SIGNAL_HANDLER;
	void TIMER2_COMPA_vect(void) SIGNAL_HANDLER;
	void TIMER2_COMPB_vect(void) SIGNAL_HANDLER;
	void TIMER2_OVF_vect(void) SIGNAL_HANDLER;
	void TWI_vect(void) SIGNAL_HANDLER;
	void WDT_vect(void) SIGNAL_HANDLER;
	void USART0_UDRE_vect(void) SIGNAL_HANDLER;
	void USART0_RX_vect(void) SIGNAL_HANDLER;
	void USART0_TX_vect(void) SIGNAL_HANDLER;
	void USART1_UDRE_vect(void) SIGNAL_HANDLER;
	void USART1_RX_vect(void) SIGNAL_HANDLER;
	void USART1_TX_vect(void) SIGNAL_HANDLER;
	void EE_READY_vect(void) SIGNAL_HANDLER;
}
/// @endcond
#endif /* BOARDS_ATMEGA_XX4_HH */
/// @endcond
