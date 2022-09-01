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

/// @cond uno

/**
 * @file 
 * ATmega328P specific features and pins.
 */

#ifndef BOARDS_UNO_HH
#define BOARDS_UNO_HH

#include "io.h"
#include "../defines.h"
#include <avr/sleep.h>

/// @cond notdocumented
/* This board is based on ATmega328P */
#define BOARD_ATMEGA328P
/// @endcond

/**
 * Defines all types and constants specific to support ATmega328P MCU target.
 */
namespace board
{
	/**
	 * Performs special initialization for ATmega328P, actually nothing at all.
	 * This must be called first in your `main()` function, even before `sei()`.
	 * In general you should ALWAYS call this function at the beginning of your
	 * `main()` even if you know it will not do anything; this will prevent strange
	 * behaviors when you want to port your code to another MCU target for which
	 * `init()` does perform important initialization, e.g. ATmega32u4 (Arduino
	 * LEONARDO).
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
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};
	
	/**
	 * Defines all available digital input/output pins of ATmega328P, with 
	 * reference to Arduino UNO pins.
	 * Each symbol is in the form `Dxx_Pyz`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, C or D) and `z` is the bit number for 
	 * that pin within its port.
	 */
	enum class DigitalPin: uint8_t
	{
		/** Pin PD0 (D0 on Arduino UNO) */
		D0_PD0 = 0,
		/** Pin PD0 (D0 on Arduino UNO) */
		D0 = D0_PD0,
		/** Pin PD1 (D1 on Arduino UNO) */
		D1_PD1,
		/** Pin PD1 (D1 on Arduino UNO) */
		D1 = D1_PD1,
		/** Pin PD2 (D2 on Arduino UNO) */
		D2_PD2,
		/** Pin PD2 (D2 on Arduino UNO) */
		D2 = D2_PD2,
		/** Pin PD3 (D3 on Arduino UNO) */
		D3_PD3,
		/** Pin PD3 (D3 on Arduino UNO) */
		D3 = D3_PD3,
		/** Pin PD4 (D4 on Arduino UNO) */
		D4_PD4,
		/** Pin PD4 (D4 on Arduino UNO) */
		D4 = D4_PD4,
		/** Pin PD5 (D5 on Arduino UNO) */
		D5_PD5,
		/** Pin PD5 (D5 on Arduino UNO) */
		D5 = D5_PD5,
		/** Pin PD6 (D6 on Arduino UNO) */
		D6_PD6,
		/** Pin PD6 (D6 on Arduino UNO) */
		D6 = D6_PD6,
		/** Pin PD7 (D7 on Arduino UNO) */
		D7_PD7,
		/** Pin PD7 (D7 on Arduino UNO) */
		D7 = D7_PD7,
		/** Pin PB0 (D8 on Arduino UNO) */
		D8_PB0,
		/** Pin PB0 (D8 on Arduino UNO) */
		D8 = D8_PB0,
		/** Pin PB1 (D9 on Arduino UNO) */
		D9_PB1,
		/** Pin PB1 (D9 on Arduino UNO) */
		D9 = D9_PB1,
		/** Pin PB2 (D10 on Arduino UNO) */
		D10_PB2,
		/** Pin PB2 (D10 on Arduino UNO) */
		D10 = D10_PB2,
		/** Pin PB3 (D11 on Arduino UNO) */
		D11_PB3,
		/** Pin PB3 (D11 on Arduino UNO) */
		D11 = D11_PB3,
		/** Pin PB4 (D12 on Arduino UNO) */
		D12_PB4,
		/** Pin PB4 (D12 on Arduino UNO) */
		D12 = D12_PB4,
		/** Pin PB5 (D13 on Arduino UNO) */
		D13_PB5,
		/** Pin PB5 (D13 on Arduino UNO) */
		D13 = D13_PB5,
		/** Pin PC0 (A0, D14 on Arduino UNO) */
		D14_PC0,
		/** Pin PC0 (A0, D14 on Arduino UNO) */
		A0 = D14_PC0,
		/** Pin PC0 (A0, D14 on Arduino UNO) */
		A0_PC0 = D14_PC0,
		/** Pin PC1 (A1, D15 on Arduino UNO) */
		D15_PC1,
		/** Pin PC1 (A1, D15 on Arduino UNO) */
		A1 = D15_PC1,
		/** Pin PC1 (A1, D15 on Arduino UNO) */
		A1_PC1 = D15_PC1,
		/** Pin PC2 (A2, D16 on Arduino UNO) */
		D16_PC2,
		/** Pin PC2 (A2, D16 on Arduino UNO) */
		A2 = D16_PC2,
		/** Pin PC2 (A2, D16 on Arduino UNO) */
		A2_PC2 = D16_PC2,
		/** Pin PC3 (A3, D17 on Arduino UNO) */
		D17_PC3,
		/** Pin PC3 (A3, D17 on Arduino UNO) */
		A3 = D17_PC3,
		/** Pin PC3 (A3, D17 on Arduino UNO) */
		A3_PC3 = D17_PC3,
		/** Pin PC4 (A4, D18 on Arduino UNO) */
		D18_PC4,
		/** Pin PC4 (A4, D18 on Arduino UNO) */
		A4 = D18_PC4,
		/** Pin PC4 (A4, D18 on Arduino UNO) */
		A4_PC4 = D18_PC4,
		/** Pin PC5 (A5, D19 on Arduino UNO) */
		D19_PC5,
		/** Pin PC5 (A5, D19 on Arduino UNO) */
		A5 = D19_PC5,
		/** Pin PC5 (A5, D19 on Arduino UNO) */
		A5_PC5 = D19_PC5,
		/** Shortcut for LED pin on Arduino */
		LED = D13_PB5,
		/** Shortcut for LED pin on Arduino */
		LED_PB5 = D13_PB5,
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
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
		/** 
		 * Voltage reference is internal 1.1V reference, generated from the
		 * internal bandgap reference.
		 */
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
		/** Pin ADC0 (A0, D14 on Arduino UNO) */
		A0 = 0,
		/** Pin ADC1 (A1, D15 on Arduino UNO) */
		A1,
		/** Pin ADC2 (A2, D16 on Arduino UNO) */
		A2,
		/** Pin ADC3 (A3, D17 on Arduino UNO) */
		A3,
		/** Pin ADC4 (A4, D18 on Arduino UNO) */
		A4,
		/** Pin ADC5 (A5, D19 on Arduino UNO) */
		A5,
#ifdef HAS_8_ANALOG_INPUTS
		/** Pin ADC6 (A6 on Arduino NANO) */
		A6,
		/** Pin ADC7 (A7 on Arduino NANO) */
		A7,
#endif
		/** Temperature sensor */
		TEMP,
		/** Bandgap reference */
		BANDGAP,
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};
	
	/**
	 * Defines all digital output pins of ATmega328P, capable of PWM output.
	 * Each symbol is in the form `Dxx_Pyz_OCuv`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port, `u` is the number of the timer used by this PWM 
	 * pin and `v` the letter indicating which compare register of the timer this 
	 * PWM pin is mapped to.
	 */
	enum class PWMPin : uint8_t
	{
		D6_PD6_OC0A = 0,
		D5_PD5_OC0B,
		D9_PB1_OC1A,
		D10_PB2_OC1B,
		D11_PB3_OC2A,
		D3_PD3_OC2B,
		D6 = D6_PD6_OC0A,
		D5 = D5_PD5_OC0B,
		D9 = D9_PB1_OC1A,
		D10 = D10_PB2_OC1B,
		D11 = D11_PB3_OC2A,
		D3 = D3_PD3_OC2B,
		// FastArduino internal: DO NOT USE
		NONE = UINT8_MAX
	};
	
	/**
	 * Defines all digital output pins of ATmega328P, usable as direct external 
	 * interrupt pins.
	 * Each symbol is in the form `Dxx_Pyz_EXTu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the interrupt for that
	 * pin.
	 */
	enum class ExternalInterruptPin : uint8_t
	{
		D2_PD2_EXT0 = 0,
		D3_PD3_EXT1,
		D2 = D2_PD2_EXT0,
		D3 = D3_PD3_EXT1
	};

	/**
	 * Defines all digital output pins of ATmega328P, usable as pin change 
	 * interrupt (PCI) pins.
	 * Each symbol is in the form `Dxx_Pyz_PCIu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the PCI vector for that
	 * pin.
	 */
	enum class InterruptPin : uint8_t
	{
		D0_PD0_PCI2 = uint8_t(DigitalPin::D0_PD0),
		D1_PD1_PCI2 = uint8_t(DigitalPin::D1_PD1),
		D2_PD2_PCI2 = uint8_t(DigitalPin::D2_PD2),
		D3_PD3_PCI2 = uint8_t(DigitalPin::D3_PD3),
		D4_PD4_PCI2 = uint8_t(DigitalPin::D4_PD4),
		D5_PD5_PCI2 = uint8_t(DigitalPin::D5_PD5),
		D6_PD6_PCI2 = uint8_t(DigitalPin::D6_PD6),
		D7_PD7_PCI2 = uint8_t(DigitalPin::D7_PD7),
		
		D8_PB0_PCI0 = uint8_t(DigitalPin::D8_PB0),
		D9_PB1_PCI0 = uint8_t(DigitalPin::D9_PB1),
		D10_PB2_PCI0 = uint8_t(DigitalPin::D10_PB2),
		D11_PB3_PCI0 = uint8_t(DigitalPin::D11_PB3),
		D12_PB4_PCI0 = uint8_t(DigitalPin::D12_PB4),
		D13_PB5_PCI0 = uint8_t(DigitalPin::D13_PB5),
		
		D14_PC0_PCI1 = uint8_t(DigitalPin::D14_PC0),
		D15_PC1_PCI1 = uint8_t(DigitalPin::D15_PC1),
		D16_PC2_PCI1 = uint8_t(DigitalPin::D16_PC2),
		D17_PC3_PCI1 = uint8_t(DigitalPin::D17_PC3),
		D18_PC4_PCI1 = uint8_t(DigitalPin::D18_PC4),
		D19_PC5_PCI1 = uint8_t(DigitalPin::D19_PC5),

		D0 = D0_PD0_PCI2,
		D1 = D1_PD1_PCI2,
		D2 = D2_PD2_PCI2,
		D3 = D3_PD3_PCI2,
		D4 = D4_PD4_PCI2,
		D5 = D5_PD5_PCI2,
		D6 = D6_PD6_PCI2,
		D7 = D7_PD7_PCI2,
		
		D8 = D8_PB0_PCI0,
		D9 = D9_PB1_PCI0,
		D10 = D10_PB2_PCI0,
		D11 = D11_PB3_PCI0,
		D12 = D12_PB4_PCI0,
		D13 = D13_PB5_PCI0,
		
		A0 = D14_PC0_PCI1,
		A1 = D15_PC1_PCI1,
		A2 = D16_PC2_PCI1,
		A3 = D17_PC3_PCI1,
		A4 = D18_PC4_PCI1,
		A5 = D19_PC5_PCI1,
		
		A0_PC0_PCI1 = D14_PC0_PCI1,
		A1_PC1_PCI1 = D15_PC1_PCI1,
		A2_PC2_PCI1 = D16_PC2_PCI1,
		A3_PC3_PCI1 = D17_PC3_PCI1,
		A4_PC4_PCI1 = D18_PC4_PCI1,
		A5_PC5_PCI1 = D19_PC5_PCI1
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
		TIMER0 = 0,
		/** Timer1 (16 bits) */
		TIMER1 = 1,
		/** Timer2 (8 bits) */
		TIMER2 = 2
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
// Forward declare interrupt service routines to allow them as friends
extern "C" {
	void ADC_vect(void) SIGNAL_HANDLER;
	void ANALOG_COMP_vect(void) SIGNAL_HANDLER;
	void INT0_vect(void) SIGNAL_HANDLER;
	void INT1_vect(void) SIGNAL_HANDLER;
	void PCINT0_vect(void) SIGNAL_HANDLER;
	void PCINT1_vect(void) SIGNAL_HANDLER;
	void PCINT2_vect(void) SIGNAL_HANDLER;
	void SPI_STC_vect(void) SIGNAL_HANDLER;
	void TIMER0_COMPA_vect(void) SIGNAL_HANDLER;
	void TIMER0_COMPB_vect(void) SIGNAL_HANDLER;
	void TIMER0_OVF_vect(void) SIGNAL_HANDLER;
	void TIMER1_CAPT_vect(void)  SIGNAL_HANDLER;
	void TIMER1_COMPA_vect(void) SIGNAL_HANDLER;
	void TIMER1_COMPB_vect(void) SIGNAL_HANDLER;
	void TIMER1_OVF_vect(void) SIGNAL_HANDLER;
	void TIMER2_COMPA_vect(void) SIGNAL_HANDLER;
	void TIMER2_COMPB_vect(void) SIGNAL_HANDLER;
	void TIMER2_OVF_vect(void) SIGNAL_HANDLER;
	void TWI_vect(void) SIGNAL_HANDLER;
	void WDT_vect(void) SIGNAL_HANDLER;
	void USART_RX_vect(void) SIGNAL_HANDLER;
	void USART_TX_vect(void) SIGNAL_HANDLER;
	void USART_UDRE_vect(void) SIGNAL_HANDLER;
	void EE_READY_vect(void) SIGNAL_HANDLER;
}

#define USART0_RX_vect USART_RX_vect
#define USART0_TX_vect USART_TX_vect
#define USART0_UDRE_vect USART_UDRE_vect
/// @endcond

#endif /* BOARDS_UNO_HH */
/// @endcond
