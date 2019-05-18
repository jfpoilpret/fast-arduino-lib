//   Copyright 2016-2019 Jean-Francois Poilpret
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

/// @cond leonardo

/**
 * @file 
 * ATmega32u4 (Arduino LEONARDO) specific features and pins.
 */

#ifndef BOARDS_LEONARDO_HH
#define BOARDS_LEONARDO_HH

#include "io.h"
#include <avr/sleep.h>

/// @cond notdocumented
/* This board is based on ATmega32u4 */
#define BOARD_ATMEGA32U4

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif
/// @endcond

/**
 * Defines all types and constants specific to support Arduino LEONARDO board
 * (ATmega32u4 MCU target).
 * LEONARDO pinout: http://duino4projects.com/wp-content/uploads/2013/04/Ardunio_leonardo_pinout.jpg
 */
namespace board
{
	/**
	 * Performs special initialization for ATmega32u4, actually disabling USB
	 * related interrupts, to avoid strange behavior at reset time.
	 * This must be called first in your `main()` function, even before `sei()`.
	 */
	inline static void init()
	{
		// Disable USB controller by default
		*((volatile uint8_t*) USBCON) = 0;
		*((volatile uint8_t*) UDCON) = 0;
		// Disable all interrupts
		*((volatile uint8_t*) UDINT) = 0;
		*((volatile uint8_t*) UDIEN) = 0;
	}
	
	/**
	 * Defines all available ports of ATmega32u4.
	 */
	enum class Port: uint8_t
	{
		/** Port B (8 IO, only 7 available on LEONARDO) */
		PORT_B = 0,
		/** Port C (2 IO) */
		PORT_C,
		/** Port D (8 IO, only 7 available on LEONARDO) */
		PORT_D,
		/** Port E (2 IO, only 1 available on LEONARDO) */
		PORT_E,
		/** Port F (6 IO) */
		PORT_F,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};
	
	/**
	 * Defines all available digital input/output pins of ATmega32u4, with 
	 * reference to Arduino LEONARDO pins.
	 * Each symbol is in the form `Dxx_Pyz`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, C, D, E or F) and `z` is the bit number for 
	 * that pin within its port.
	 */
	enum class DigitalPin: uint8_t
	{
		/** Pin PD2 (D0 on Arduino LEONARDO) */
		D0_PD2 = 0,
		/** Pin PD2 (D0 on Arduino LEONARDO) */
		D0 = D0_PD2,
		/** Pin PD3 (D1 on Arduino LEONARDO) */
		D1_PD3,
		/** Pin PD3 (D1 on Arduino LEONARDO) */
		D1 = D1_PD3,
		/** Pin PD1 (D2 on Arduino LEONARDO) */
		D2_PD1,
		/** Pin PD1 (D2 on Arduino LEONARDO) */
		D2 = D2_PD1,
		/** Pin PD0 (D3 on Arduino LEONARDO) */
		D3_PD0,
		/** Pin PD0 (D3 on Arduino LEONARDO) */
		D3 = D3_PD0,
		/** Pin PD4 (D4 on Arduino LEONARDO) */
		D4_PD4,
		/** Pin PD4 (D4 on Arduino LEONARDO) */
		D4 = D4_PD4,
		/** Pin PC6 (D5 on Arduino LEONARDO) */
		D5_PC6,
		/** Pin PC6 (D5 on Arduino LEONARDO) */
		D5 = D5_PC6,
		/** Pin PD7 (D6 on Arduino LEONARDO) */
		D6_PD7,
		/** Pin PD7 (D6 on Arduino LEONARDO) */
		D6 = D6_PD7,
		/** Pin PE6 (D7 on Arduino LEONARDO) */
		D7_PE6,
		/** Pin PE6 (D7 on Arduino LEONARDO) */
		D7 = D7_PE6,
		/** Pin PB4 (D8 on Arduino LEONARDO) */
		D8_PB4,
		/** Pin PB4 (D8 on Arduino LEONARDO) */
		D8 = D8_PB4,
		/** Pin PB5 (D9 on Arduino LEONARDO) */
		D9_PB5,
		/** Pin PB5 (D9 on Arduino LEONARDO) */
		D9 = D9_PB5,
		/** Pin PB6 (D10 on Arduino LEONARDO) */
		D10_PB6,
		/** Pin PB6 (D10 on Arduino LEONARDO) */
		D10 = D10_PB6,
		/** Pin PB7 (D11 on Arduino LEONARDO) */
		D11_PB7,
		/** Pin PB7 (D11 on Arduino LEONARDO) */
		D11 = D11_PB7,
		/** Pin PD6 (D12 on Arduino LEONARDO) */
		D12_PD6,
		/** Pin PD6 (D12 on Arduino LEONARDO) */
		D12 = D12_PD6,
		/** Pin PC7 (D13 on Arduino LEONARDO) */
		D13_PC7,
		/** Pin PC7 (D13 on Arduino LEONARDO) */
		D13 = D13_PC7,
		
		/** Pin PF7 (A0 on Arduino LEONARDO) */
		A0_PF7,
		/** Pin PF7 (A0 on Arduino LEONARDO) */
		A0 = A0_PF7,
		/** Pin PF6 (A1 on Arduino LEONARDO) */
		A1_PF6,
		/** Pin PF6 (A1 on Arduino LEONARDO) */
		A1 = A1_PF6,
		/** Pin PF5 (A2 on Arduino LEONARDO) */
		A2_PF5,
		/** Pin PF5 (A2 on Arduino LEONARDO) */
		A2 = A2_PF5,
		/** Pin PF4 (A3 on Arduino LEONARDO) */
		A3_PF4,
		/** Pin PF4 (A3 on Arduino LEONARDO) */
		A3 = A3_PF4,
		/** Pin PF1 (A4 on Arduino LEONARDO) */
		A4_PF1,
		/** Pin PF1 (A4 on Arduino LEONARDO) */
		A4 = A4_PF1,
		/** Pin PF0 (A5 on Arduino LEONARDO) */
		A5_PF0,
		/** Pin PF0 (A5 on Arduino LEONARDO) */
		A5 = A5_PF0,
		
		/** Pin PB1 (SCK (SPI) on Arduino LEONARDO) */
		SCK_PB1,
		/** Pin PB2 (MOSI (SPI) on Arduino LEONARDO) */
		MOSI_PB2,
		/** Pin PB3 (MISO (SPI) on Arduino LEONARDO) */
		MISO_PB3,
		
		/** Pin PB0 (RXLED on Arduino LEONARDO, no pin) */
		RXLED_PB0,
		/** Pin PD5 (TXLED on Arduino LEONARDO, no pin) */
		TXLED_PD5,
		
		/** Shortcut for LED pin on Arduino */
		LED = D13_PC7,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};

	/**
	 * Defines available clocks of ATmega32u4, used for analog input.
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
	 * Defines available voltage references of ATmega32u4, used for analog input.
	 */
	enum class AnalogReference: uint8_t
	{
		/** Voltage reference is given by the `AREF` pin. */
		AREF = 0,
		/** Voltage reference is given by the `AVcc` pin. */
		AVCC,
		/** Voltage reference is internal 2.56V reference, generated from the
		 * internal bandgap reference. */
		INTERNAL_2_56V
	};
	
	/**
	 * Defines all available analog input pins of ATmega32u4, with 
	 * reference to Arduino LEONARDO pins.
	 * Note that this includes also other sources than pin, e.g. the internal
	 * bandgap reference or the temperature sensor.
	 */
	enum class AnalogPin: uint8_t
	{
		/** Pin ADC7 (A0 on Arduino LEONARDO) */
		A0_ADC7 = 0,
		/** Alias to pin ADC7 (A0 on Arduino LEONARDO) */
		A0 = A0_ADC7,
		/** Pin ADC6 (A1 on Arduino LEONARDO) */
		A1_ADC6,
		/** Alias to pin ADC7 (A0 on Arduino LEONARDO) */
		A1 = A1_ADC6,
		/** Pin ADC5 (A2 on Arduino LEONARDO) */
		A2_ADC5,
		/** Alias to pin ADC5 (A2 on Arduino LEONARDO) */
		A2 = A2_ADC5,
		/** Pin ADC4 (A3 on Arduino LEONARDO) */
		A3_ADC4,
		/** Alias to pin ADC4 (A3 on Arduino LEONARDO) */
		A3 = A3_ADC4,
		/** Pin ADC1 (A4 on Arduino LEONARDO) */
		A4_ADC1,
		/** Alias to pin ADC1 (A4 on Arduino LEONARDO) */
		A4 = A4_ADC1,
		/** Pin ADC0 (A5 on Arduino LEONARDO) */
		A5_ADC0,
		/** Alias to pin ADC0 (A5 on Arduino LEONARDO) */
		A5 = A5_ADC0,

		/** Pin ADC8 (D4 on Arduino LEONARDO) */
		A6_D4_ADC8,
		/** Pin ADC8 (D4 on Arduino LEONARDO) */
		D4 = A6_D4_ADC8,
		/** Pin ADC10 (D6 on Arduino LEONARDO) */
		A7_D6_ADC10,
		/** Pin ADC10 (D6 on Arduino LEONARDO) */
		D6 = A7_D6_ADC10,
		/** Pin ADC11 (D8 on Arduino LEONARDO) */
		A8_D8_ADC11,
		/** Pin ADC11 (D8 on Arduino LEONARDO) */
		D8 = A8_D8_ADC11,
		/** Pin ADC12 (D9 on Arduino LEONARDO) */
		A9_D9_ADC12,
		/** Pin ADC12 (D9 on Arduino LEONARDO) */
		D9 = A9_D9_ADC12,
		/** Pin ADC13 (D10 on Arduino LEONARDO) */
		A10_D10_ADC13,
		/** Pin ADC13 (D10 on Arduino LEONARDO) */
		D10 = A10_D10_ADC13,
		/** Pin ADC9 (D12 on Arduino LEONARDO) */
		A11_D12_ADC9,
		/** Pin ADC9 (D12 on Arduino LEONARDO) */
		D12 = A11_D12_ADC9,
		
		/** Temperature sensor */
		TEMP,
		/** Bandgap reference */
		BANDGAP,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};
	
	/**
	 * Defines all digital output pins of ATmega32u4, capable of PWM output.
	 * Each symbol is in the form `Dxx_Pyz_OCuv`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (B, C or D), `z` is the bit number for 
	 * that pin within its port, `u` is the number of the timer used by this PWM 
	 * pin and `v` the letter indicating which compare register of the timer this 
	 * PWM pin is mapped to.
	 * 
	 * Note that Timer4, which is specific (high speed) is not present here, as
	 * it is not supported (yet) by FastArduino.
	 * 
	 * Also note that ATmega32u4 has pins that can be linked to several timers (e.g.
	 * pin PB7 is connected to OC0A and OC1C) but FastArduino is currently limited
	 * to pins linked to only one timer, hence for each pin, a choice had to be
	 * made: the main principles for this choice were to take timer comparison
	 * channels A and B first. In the future, support for pins with multiple
	 * timers linked may be added.
	 */
	enum class PWMPin : uint8_t
	{
		D11_PB7_OC0A = 0,
		D3_PD0_OC0B,
		D9_PB5_OC1A,
		D10_PB6_OC1B,
		D11_PB7_OC1C,
		D5_PC6_OC3A,
		//TODO High-speed timer (Timer4) related PWM pins are not listed here because Timer4 is not supported yer

		D3 = D3_PD0_OC0B,
		D9 = D9_PB5_OC1A,
		D10 = D10_PB6_OC1B,
		D5 = D5_PC6_OC3A,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};
	
	/**
	 * Defines all digital output pins of ATmega32u4, usable as direct external 
	 * interrupt pins.
	 * Each symbol is in the form `Dxx_Pyz_EXTu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter (D or E), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the interrupt for that
	 * pin.
	 * This namespace exists for the sole purpose of quickly finding an external
	 * interrupt pin among all digital IO pins.
	 */
	namespace ExternalInterruptPin
	{
		constexpr const DigitalPin D3_PD0_EXT0 = DigitalPin::D3_PD0;
		constexpr const DigitalPin D2_PD1_EXT1 = DigitalPin::D2_PD1;
		constexpr const DigitalPin D0_PD2_EXT2 = DigitalPin::D0_PD2;
		constexpr const DigitalPin D1_PD3_EXT3 = DigitalPin::D1_PD3;
		constexpr const DigitalPin D7_PE6_EXT6 = DigitalPin::D7_PE6;

		constexpr const DigitalPin D3 = DigitalPin::D3_PD0;
		constexpr const DigitalPin D2 = DigitalPin::D2_PD1;
		constexpr const DigitalPin D0 = DigitalPin::D0_PD2;
		constexpr const DigitalPin D1 = DigitalPin::D1_PD3;
		constexpr const DigitalPin D7 = DigitalPin::D7_PE6;
	};

	/**
	 * Defines all digital output pins of ATmega32u4, usable as pin change 
	 * interrupt (PCI) pins.
	 * Each symbol is in the form `Dxx_Pyz_PCIu` or `FUNC_Pyz_PCIu`, where 
	 * `FUNC` is the specific function of the pin or `xx` the pin number on 
	 * Arduino, `y` is the port letter (always B), `z` is the bit number for 
	 * that pin within its port and `u` is the number of the PCI vector for that
	 * pin.
	 * This namespace exists for the sole purpose of quickly finding an pin change
	 * interrupt pin among all digital IO pins.
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

		constexpr const DigitalPin D8 = DigitalPin::D8_PB4;
		constexpr const DigitalPin D9 = DigitalPin::D9_PB5;
		constexpr const DigitalPin D10 = DigitalPin::D10_PB6;
		constexpr const DigitalPin D11 = DigitalPin::D11_PB7;
	};

	/**
	 * Defines all USART modules of ATmega32u4, actually only one.
	 * Note that USB device is not represented here as it is not supported yet
	 * by FastArduino.
	 */
	enum class USART: uint8_t
	{
		USART1 = 1
	};
	
	/**
	 * Defines all "standard" timers available for ATmega32u4. This excludes
	 * specific "high-speed" Timer4, not supported by FastArduino currently.
	 */
	enum class Timer: uint8_t
	{
		/** Timer0 (8 bits) */
		TIMER0 = 0,
		/** Timer1 (16 bits) */
		TIMER1 = 1,
		/** Timer3 (16 bits) */
		TIMER3 = 3
	};
	
	//TODO Add specific enumeration for High-speed timer? (or specific traits?)
	
	/**
	 * Defines all available sleep modes for ATmega32u4.
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
		 * interrupts, I2C slave (if enabled), USB interrupts and Watchdog Timer 
		 * (if enabled).
		 * This is lowest current consumption mode, typically a few uA, depending
		 * on other factors (voltage, watchdog enabled or not).
		 * Waking up from this mode may take significant time until internal 
		 * oscillator is restarted and stabilized; refer to datasheet for more
		 * detailed data (look for `Start-up times`).
		 */
		POWER_DOWN = SLEEP_MODE_PWR_DOWN,
		/**
		 * This mode is exactly the same as `POWER_DOWN`.
		 */
		POWER_SAVE = SLEEP_MODE_PWR_SAVE,
		/**
		 * This mode is similar to `POWER_DOWN`, except the oscillator is kept
		 * running, hence waking up from this mode takes only 6 clock cycles.
		 */
		STANDBY = SLEEP_MODE_STANDBY,
		/**
		 * This mode is exactly the same as `STANDBY`.
		 */
		EXTENDED_STANDBY = SLEEP_MODE_EXT_STANDBY
	};
};

/// @cond notdocumented
// Forward declare interrupt service routines to allow them as friends.
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
/// @endcond

#endif /* BOARDS_LEONARDO_HH */
/// @endcond
