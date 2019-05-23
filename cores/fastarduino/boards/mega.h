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

/// @cond mega

/**
 * @file 
 * ATmega2560 (Arduino MEGA) specific features and pins.
 */

#ifndef BOARDS_MEGA_HH
#define BOARDS_MEGA_HH

#include "io.h"
#include <avr/sleep.h>

/// @cond notdocumented
/* This board is based on ATmega2560 (older MEGA boards embedding an ATmega1280 
   are not supported. */
#define BOARD_ATMEGA2560

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif
/// @endcond

/**
 * Defines all types and constants specific to support Arduino MEGA board
 * (ATmega2560 MCU target).
 */
namespace board
{
	/**
	 * Performs special initialization for ATmega2560, actually nothing at all.
	 * This must be called first in your `main()` function, even before `sei()`.
	 * In general you should ALWAYS call this function at the beginning of your
	 * `main()` even if you know it will not do anything; this will prevent strange
	 * behaviors when you want to port your code to another MCU target for which
	 * `init()` does perform important initialization, e.g. ATmega32u4 (Arduino
	 * LEONARDO).
	 */
	inline static void init() {}
	
	/**
	 * Defines all available ports of ATmega2560.
	 */
	enum class Port: uint8_t
	{
		/** Port A (8 IO) */
		PORT_A = 0,
		/** Port B (8 IO) */
		PORT_B,
		/** Port C (8 IO) */
		PORT_C,
		/** Port D (8 IO, only 5 available on Arduino MEGA) */
		PORT_D,
		/** Port E (8 IO, only 5 available on Arduino MEGA) */
		PORT_E,
		/** Port F (8 IO) */
		PORT_F,
		/** Port G (6 IO, only 4 available on Arduino MEGA) */
		PORT_G,
		/** Port H (8 IO, only 6 available on Arduino MEGA) */
		PORT_H,
		/** Port J (8 IO, only 2 available on Arduino MEGA) */
		PORT_J,
		/** Port K (8 IO) */
		PORT_K,
		/** Port L (8 IO) */
		PORT_L,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};
	
	/**
	 * Defines all available digital input/output pins of ATmega2560, with 
	 * reference to Arduino MEGA pins. Only Arduino MEGA pins are listed here.
	 * Each symbol is in the form `Dxx_Pyz`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter and `z` is the bit number for 
	 * that pin within its port.
	 */
	enum class DigitalPin: uint8_t
	{
		/** Pin PE0 (D0/RX0 on Arduino MEGA) */
		D0_PE0 = 0,
		/** Pin PE0 (D0/RX0 on Arduino MEGA) */
		D0 = D0_PE0,
		/** Pin PE1 (D1/TX0 on Arduino MEGA) */
		D1_PE1,
		/** Pin PE1 (D1/TX0 on Arduino MEGA) */
		D1 = D1_PE1,
		/** Pin PE4 (D3 on Arduino MEGA) */
		D2_PE4,
		/** Pin PE4 (D3 on Arduino MEGA) */
		D2 = D2_PE4,
		/** Pin PE5 (D3 on Arduino MEGA) */
		D3_PE5,
		/** Pin PE5 (D3 on Arduino MEGA) */
		D3 = D3_PE5,
		/** Pin PG5 (D4 on Arduino MEGA) */
		D4_PG5,
		/** Pin PG5 (D4 on Arduino MEGA) */
		D4 = D4_PG5,
		/** Pin PE3 (D5 on Arduino MEGA) */
		D5_PE3,
		/** Pin PE3 (D5 on Arduino MEGA) */
		D5 = D5_PE3,
		/** Pin PH3 (D6 on Arduino MEGA) */
		D6_PH3,
		/** Pin PH3 (D6 on Arduino MEGA) */
		D6 = D6_PH3,
		/** Pin PH4 (D7 on Arduino MEGA) */
		D7_PH4,
		/** Pin PH4 (D7 on Arduino MEGA) */
		D7 = D7_PH4,
		/** Pin PH5 (D8 on Arduino MEGA) */
		D8_PH5,
		/** Pin PH5 (D8 on Arduino MEGA) */
		D8 = D8_PH5,
		/** Pin PH6 (D9 on Arduino MEGA) */
		D9_PH6,
		/** Pin PH6 (D9 on Arduino MEGA) */
		D9 = D9_PH6,
		/** Pin PB4 (D10 on Arduino MEGA) */
		D10_PB4,
		/** Pin PB4 (D10 on Arduino MEGA) */
		D10 = D10_PB4,
		/** Pin PB5 (D11 on Arduino MEGA) */
		D11_PB5,
		/** Pin PB5 (D11 on Arduino MEGA) */
		D11 = D11_PB5,
		/** Pin PB6 (D12 on Arduino MEGA) */
		D12_PB6,
		/** Pin PB6 (D12 on Arduino MEGA) */
		D12 = D12_PB6,
		/** Pin PB7 (D13 on Arduino MEGA) */
		D13_PB7,
		/** Pin PB7 (D13 on Arduino MEGA) */
		D13 = D13_PB7,
		/** Pin PJ1 (D14/TX3 on Arduino MEGA) */
		D14_PJ1,
		/** Pin PJ1 (D14/TX3 on Arduino MEGA) */
		D14 = D14_PJ1,
		/** Pin PJ0 (D15/RX3 on Arduino MEGA) */
		D15_PJ0,
		/** Pin PJ0 (D15/RX3 on Arduino MEGA) */
		D15 = D15_PJ0,
		/** Pin PH1 (D16 /TX2 on Arduino MEGA) */
		D16_PH1,
		/** Pin PH1 (D16 /TX2 on Arduino MEGA) */
		D16 = D16_PH1,
		/** Pin PH0 (D17 /RX2 on Arduino MEGA) */
		D17_PH0,
		/** Pin PH0 (D17 /RX2 on Arduino MEGA) */
		D17 = D17_PH0,
		/** Pin PD3 (D18 /TX1 on Arduino MEGA) */
		D18_PD3,
		/** Pin PD3 (D18 /TX1 on Arduino MEGA) */
		D18 = D18_PD3,
		/** Pin PD2 (D19 /RX1 on Arduino MEGA) */
		D19_PD2,
		/** Pin PD2 (D19 /RX1 on Arduino MEGA) */
		D19 = D19_PD2,
		/** Pin PD1 (D20 /SDA on Arduino MEGA) */
		D20_PD1,
		/** Pin PD1 (D20 /SDA on Arduino MEGA) */
		D20 = D20_PD1,
		/** Pin PD0 (D21 /SCL on Arduino MEGA) */
		D21_PD0,
		/** Pin PD0 (D21 /SCL on Arduino MEGA) */
		D21 = D21_PD0,
		/** Pin PA0 (D22 on Arduino MEGA) */
		D22_PA0,
		/** Pin PA0 (D22 on Arduino MEGA) */
		D22 = D22_PA0,
		/** Pin PA1 (D23 on Arduino MEGA) */
		D23_PA1,
		/** Pin PA1 (D23 on Arduino MEGA) */
		D23 = D23_PA1,
		/** Pin PA2 (D24 on Arduino MEGA) */
		D24_PA2,
		/** Pin PA2 (D24 on Arduino MEGA) */
		D24 = D24_PA2,
		/** Pin PA3 (D25 on Arduino MEGA) */
		D25_PA3,
		/** Pin PA3 (D25 on Arduino MEGA) */
		D25 = D25_PA3,
		/** Pin PA4 (D26 on Arduino MEGA) */
		D26_PA4,
		/** Pin PA4 (D26 on Arduino MEGA) */
		D26 = D26_PA4,
		/** Pin PA5 (D27 on Arduino MEGA) */
		D27_PA5,
		/** Pin PA5 (D27 on Arduino MEGA) */
		D27 = D27_PA5,
		/** Pin PA6 (D28 on Arduino MEGA) */
		D28_PA6,
		/** Pin PA6 (D28 on Arduino MEGA) */
		D28 = D28_PA6,
		/** Pin PA7 (D29 on Arduino MEGA) */
		D29_PA7,
		/** Pin PA7 (D29 on Arduino MEGA) */
		D29 = D29_PA7,
		/** Pin PC7 (D30 on Arduino MEGA) */
		D30_PC7,
		/** Pin PC7 (D30 on Arduino MEGA) */
		D30 = D30_PC7,
		/** Pin PC6 (D31 on Arduino MEGA) */
		D31_PC6,
		/** Pin PC6 (D31 on Arduino MEGA) */
		D31 = D31_PC6,
		/** Pin PC5 (D32 on Arduino MEGA) */
		D32_PC5,
		/** Pin PC5 (D32 on Arduino MEGA) */
		D32 = D32_PC5,
		/** Pin PC4 (D33 on Arduino MEGA) */
		D33_PC4,
		/** Pin PC4 (D33 on Arduino MEGA) */
		D33 = D33_PC4,
		/** Pin PC3 (D34 on Arduino MEGA) */
		D34_PC3,
		/** Pin PC3 (D34 on Arduino MEGA) */
		D34 = D34_PC3,
		/** Pin PC2 (D35 on Arduino MEGA) */
		D35_PC2,
		/** Pin PC2 (D35 on Arduino MEGA) */
		D35 = D35_PC2,
		/** Pin PC1 (D36 on Arduino MEGA) */
		D36_PC1,
		/** Pin PC1 (D36 on Arduino MEGA) */
		D36 = D36_PC1,
		/** Pin PC0 (D37 on Arduino MEGA) */
		D37_PC0,
		/** Pin PC0 (D37 on Arduino MEGA) */
		D37 = D37_PC0,
		/** Pin PD7 (D38 on Arduino MEGA) */
		D38_PD7,
		/** Pin PD7 (D38 on Arduino MEGA) */
		D38 = D38_PD7,
		/** Pin PG2 (D39 on Arduino MEGA) */
		D39_PG2,
		/** Pin PG2 (D39 on Arduino MEGA) */
		D39 = D39_PG2,
		/** Pin PG1 (D40 on Arduino MEGA) */
		D40_PG1,
		/** Pin PG1 (D40 on Arduino MEGA) */
		D40 = D40_PG1,
		/** Pin PG0 (D41 on Arduino MEGA) */
		D41_PG0,
		/** Pin PG0 (D41 on Arduino MEGA) */
		D41 = D41_PG0,
		/** Pin PL7 (D42 on Arduino MEGA) */
		D42_PL7,
		/** Pin PL7 (D42 on Arduino MEGA) */
		D42 = D42_PL7,
		/** Pin PL6 (D43 on Arduino MEGA) */
		D43_PL6,
		/** Pin PL6 (D43 on Arduino MEGA) */
		D43 = D43_PL6,
		/** Pin PL5 (D44 on Arduino MEGA) */
		D44_PL5,
		/** Pin PL5 (D44 on Arduino MEGA) */
		D44 = D44_PL5,
		/** Pin PL4 (D45 on Arduino MEGA) */
		D45_PL4,
		/** Pin PL4 (D45 on Arduino MEGA) */
		D45 = D45_PL4,
		/** Pin PL3 (D46 on Arduino MEGA) */
		D46_PL3,
		/** Pin PL3 (D46 on Arduino MEGA) */
		D46 = D46_PL3,
		/** Pin PL2 (D47 on Arduino MEGA) */
		D47_PL2,
		/** Pin PL2 (D47 on Arduino MEGA) */
		D47 = D47_PL2,
		/** Pin PL1 (D48 on Arduino MEGA) */
		D48_PL1,
		/** Pin PL1 (D48 on Arduino MEGA) */
		D48 = D48_PL1,
		/** Pin PL0 (D49 on Arduino MEGA) */
		D49_PL0,
		/** Pin PL0 (D49 on Arduino MEGA) */
		D49 = D49_PL0,
		/** Pin PB3 (D50/MISO on Arduino MEGA) */
		D50_PB3,
		/** Pin PB3 (D50/MISO on Arduino MEGA) */
		D50 = D50_PB3,
		/** Pin PB2 (D51/MOSI on Arduino MEGA) */
		D51_PB2,
		/** Pin PB2 (D51/MOSI on Arduino MEGA) */
		D51 = D51_PB2,
		/** Pin PB1 (D52/SCK on Arduino MEGA) */
		D52_PB1,
		/** Pin PB1 (D52/SCK on Arduino MEGA) */
		D52 = D52_PB1,
		/** Pin PB0 (D53/SS on Arduino MEGA) */
		D53_PB0,
		/** Pin PB0 (D53/SS on Arduino MEGA) */
		D53 = D53_PB0,
		/** Pin PF0 (D54/A0 on Arduino MEGA) */
		D54_PF0,
		/** Pin PF0 (D54/A0 on Arduino MEGA) */
		A0 = D54_PF0,
		/** Pin PF1 (D55/A1 on Arduino MEGA) */
		D55_PF1,
		/** Pin PF1 (D55/A1 on Arduino MEGA) */
		A1 = D55_PF1,
		/** Pin PF2 (D56/A2 on Arduino MEGA) */
		D56_PF2,
		/** Pin PF2 (D56/A2 on Arduino MEGA) */
		A2 = D56_PF2,
		/** Pin PF3 (D57/A3 on Arduino MEGA) */
		D57_PF3,
		/** Pin PF3 (D57/A3 on Arduino MEGA) */
		A3 = D57_PF3,
		/** Pin PF4 (D58/A4 on Arduino MEGA) */
		D58_PF4,
		/** Pin PF4 (D58/A4 on Arduino MEGA) */
		A4 = D58_PF4,
		/** Pin PF5 (D59/A5 on Arduino MEGA) */
		D59_PF5,
		/** Pin PF5 (D59/A5 on Arduino MEGA) */
		A5 = D59_PF5,
		/** Pin PF6 (D60/A6 on Arduino MEGA) */
		D60_PF6,
		/** Pin PF6 (D60/A6 on Arduino MEGA) */
		A6 = D60_PF6,
		/** Pin PF7 (D61/A7 on Arduino MEGA) */
		D61_PF7,
		/** Pin PF7 (D61/A7 on Arduino MEGA) */
		A7 = D61_PF7,
		/** Pin PK0 (D62/A8 on Arduino MEGA) */
		D62_PK0,
		/** Pin PK0 (D62/A8 on Arduino MEGA) */
		A8 = D62_PK0,
		/** Pin PK1 (D63/A9 on Arduino MEGA) */
		D63_PK1,
		/** Pin PK1 (D63/A9 on Arduino MEGA) */
		A9 = D63_PK1,
		/** Pin PK2 (D64/A10 on Arduino MEGA) */
		D64_PK2,
		/** Pin PK2 (D64/A10 on Arduino MEGA) */
		A10 = D64_PK2,
		/** Pin PK3 (D65/A11 on Arduino MEGA) */
		D65_PK3,
		/** Pin PK3 (D65/A11 on Arduino MEGA) */
		A11 = D65_PK3,
		/** Pin PK4 (D66/A12 on Arduino MEGA) */
		D66_PK4,
		/** Pin PK4 (D66/A12 on Arduino MEGA) */
		A12 = D66_PK4,
		/** Pin PK5 (D67/A13 on Arduino MEGA) */
		D67_PK5,
		/** Pin PK5 (D67/A13 on Arduino MEGA) */
		A13 = D67_PK5,
		/** Pin PK6 (D68/A14 on Arduino MEGA) */
		D68_PK6,
		/** Pin PK6 (D68/A14 on Arduino MEGA) */
		A14 = D68_PK6,
		/** Pin PK7 (D69/A15 on Arduino MEGA) */
		D69_PK7,
		/** Pin PK7 (D69/A15 on Arduino MEGA) */
		A15 = D69_PK7,
		/** Shortcut for LED pin on Arduino */
		LED = D13_PB7,
		// FastArduino internal: DO NOT USE
		NONE = 0XFF
	};

	/**
	 * Defines available clocks of ATmega2560, used for analog input.
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
	 * Defines available voltage references of ATmega2560, used for analog input.
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
	 * Defines all available analog input pins of ATmega2560, with 
	 * reference to Arduino MEGA pins.
	 * Note that this includes also other sources than pin, e.g. the internal
	 * bandgap reference.
	 */
	enum class AnalogPin: uint8_t
	{
		/** Pin ADC0 (A0 on Arduino MEGA) */
		A0 = 0,
		/** Pin ADC1 (A1 on Arduino MEGA) */
		A1,
		/** Pin ADC2 (A2 on Arduino MEGA) */
		A2,
		/** Pin ADC3 (A3 on Arduino MEGA) */
		A3,
		/** Pin ADC4 (A4 on Arduino MEGA) */
		A4,
		/** Pin ADC5 (A5 on Arduino MEGA) */
		A5,
		/** Pin ADC6 (A6 on Arduino MEGA) */
		A6,
		/** Pin ADC7 (A7 on Arduino MEGA) */
		A7,
		/** Pin ADC8 (A8 on Arduino MEGA) */
		A8,
		/** Pin ADC9 (A9 on Arduino MEGA) */
		A9,
		/** Pin ADC10 (A10 on Arduino MEGA) */
		A10,
		/** Pin ADC11 (A11 on Arduino MEGA) */
		A11,
		/** Pin ADC12 (A12 on Arduino MEGA) */
		A12,
		/** Pin ADC13 (A13 on Arduino MEGA) */
		A13,
		/** Pin ADC14 (A14 on Arduino MEGA) */
		A14,
		/** Pin ADC15 (A15 on Arduino MEGA) */
		A15,
		/** Bandgap reference */
		BANDGAP,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};
	
	/**
	 * Defines all digital output pins of ATmega2560, capable of PWM output.
	 * Each symbol is in the form `Dxx_Pyz_OCuv`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter, `z` is the bit number for 
	 * that pin within its port, `u` is the number of the timer used by this PWM 
	 * pin and `v` the letter indicating which compare register of the timer this 
	 * PWM pin is mapped to.
	 * 
	 * Note that ATmega2560 has pins that can be linked to several timers (e.g.
	 * pin PB7 is connected to OC0A and OC1C) but FastArduino is currently limited
	 * to pins linked to only one timer, hence for each pin, a choice had to be
	 * made: the main principles for this choice were to take timer comparison
	 * channels A and B first. In the future, support for pins with multiple
	 * timers linked may be added.
	 */
	enum class PWMPin : uint8_t
	{
		// This pin is also OC1C! We can handle only one timer for one pin!
		D13_PB7_OC0A = 0,
		D4_PG5_OC0B,
		D11_PB5_OC1A,
		D12_PB6_OC1B,
		D13_PB7_OC1C,
		D10_PB4_OC2A,
		D9_PH6_OC2B,
		D5_PE3_OC3A,
		D2_PE4_OC3B,
		D3_PE5_OC3C,
		D6_PH3_OC4A,
		D7_PH4_OC4B,
		D8_PH5_OC4C,
		D46_PL3_OC5A,
		D45_PL4_OC5B,
		D44_PL5_OC5C,

		D2 = D2_PE4_OC3B,
		D3 = D3_PE5_OC3C,
		D4 = D4_PG5_OC0B,
		D5 = D5_PE3_OC3A,
		D6 = D6_PH3_OC4A,
		D7 = D7_PH4_OC4B,
		D8 = D8_PH5_OC4C,
		D9 = D9_PH6_OC2B,
		D10 = D10_PB4_OC2A,
		D11 = D11_PB5_OC1A,
		D12 = D12_PB6_OC1B,
		D44 = D44_PL5_OC5C,
		D45 = D45_PL4_OC5B,
		D46 = D46_PL3_OC5A,
		// FastArduino internal: DO NOT USE
		NONE = 0xFF
	};
	
	/**
	 * Defines all digital output pins of ATmega2560, usable as direct external 
	 * interrupt pins.
	 * Each symbol is in the form `Dxx_Pyz_EXTu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter, `z` is the bit number for 
	 * that pin within its port and `u` is the number of the interrupt for that
	 * pin.
	 */
	enum class ExternalInterruptPin : uint8_t
	{
		D21_PD0_EXT0 = 0,
		D20_PD1_EXT1,
		D19_PD2_EXT2,
		D18_PD3_EXT3,
		D2_PE4_EXT4,
		D3_PE5_EXT5,

		D21 = D21_PD0_EXT0,
		D20 = D20_PD1_EXT1,
		D19 = D19_PD2_EXT2,
		D18 = D18_PD3_EXT3,
		D2 = D2_PE4_EXT4,
		D3 = D3_PE5_EXT5
	};

	/**
	 * Defines all digital output pins of ATmega2560, usable as pin change 
	 * interrupt (PCI) pins.
	 * Each symbol is in the form `Dxx_Pyz_PCIu`, where `xx` is the pin number on 
	 * Arduino, `y` is the port letter, `z` is the bit number for 
	 * that pin within its port and `u` is the number of the PCI vector for that
	 * pin.
	 */
	enum class InterruptPin : uint8_t
	{
		// PB0-7
		D53_PB0_PCI0 = uint8_t(DigitalPin::D53_PB0),
		D52_PB1_PCI0 = uint8_t(DigitalPin::D52_PB1),
		D51_PB2_PCI0 = uint8_t(DigitalPin::D51_PB2),
		D50_PB3_PCI0 = uint8_t(DigitalPin::D50_PB3),
		D10_PB4_PCI0 = uint8_t(DigitalPin::D10_PB4),
		D11_PB5_PCI0 = uint8_t(DigitalPin::D11_PB5),
		D12_PB6_PCI0 = uint8_t(DigitalPin::D12_PB6),
		D13_PB7_PCI0 = uint8_t(DigitalPin::D13_PB7),

		// PJ0-1
		D15_PJ0_PCI1 = uint8_t(DigitalPin::D15_PJ0),
		D14_PJ1_PCI1 = uint8_t(DigitalPin::D14_PJ1),
		
		// PK0-7
		D62_PK0_PCI2 = uint8_t(DigitalPin::D62_PK0),
		D63_PK1_PCI2 = uint8_t(DigitalPin::D63_PK1),
		D64_PK2_PCI2 = uint8_t(DigitalPin::D64_PK2),
		D65_PK3_PCI2 = uint8_t(DigitalPin::D65_PK3),
		D66_PK4_PCI2 = uint8_t(DigitalPin::D66_PK4),
		D67_PK5_PCI2 = uint8_t(DigitalPin::D67_PK5),
		D68_PK6_PCI2 = uint8_t(DigitalPin::D68_PK6),
		D69_PK7_PCI2 = uint8_t(DigitalPin::D69_PK7),

		// PB0-7
		D53 = uint8_t(DigitalPin::D53_PB0),
		D52 = uint8_t(DigitalPin::D52_PB1),
		D51 = uint8_t(DigitalPin::D51_PB2),
		D50 = uint8_t(DigitalPin::D50_PB3),
		D10 = uint8_t(DigitalPin::D10_PB4),
		D11 = uint8_t(DigitalPin::D11_PB5),
		D12 = uint8_t(DigitalPin::D12_PB6),
		D13 = uint8_t(DigitalPin::D13_PB7),

		// PJ0-1
		D15 = uint8_t(DigitalPin::D15_PJ0),
		D14 = uint8_t(DigitalPin::D14_PJ1),
		
		// PK0-7
		A8 = uint8_t(DigitalPin::D62_PK0),
		A9 = uint8_t(DigitalPin::D63_PK1),
		A10 = uint8_t(DigitalPin::D64_PK2),
		A11 = uint8_t(DigitalPin::D65_PK3),
		A12 = uint8_t(DigitalPin::D66_PK4),
		A13 = uint8_t(DigitalPin::D67_PK5),
		A14 = uint8_t(DigitalPin::D68_PK6),
		A15 = uint8_t(DigitalPin::D69_PK7)
	};

	/**
	 * Defines all USART modules of ATmega2560.
	 */
	enum class USART: uint8_t
	{
		/**
		 * On Arduino MEGA, `USART0` is connected to USB, ans is also
		 * connected to pins D1_PE1 (TX0) and D0_PE0 (RX0).
		 */
		USART0 = 0,
		/** USART1 is connected to pins D18_PD3 (TX1) and D19_PD2 (RX1). */
		USART1 = 1,
		/** USART2 is connected to pins D16_PH1 (TX2) and D17_PH0 (RX2). */
		USART2 = 2,
		/** USART3 is connected to pins D14_PJ1 (TX3) and D15_PJ0 (RX3). */
		USART3 = 3
	};

	/**
	 * Defines all timers available for ATmega2560.
	 */
	enum class Timer: uint8_t
	{
		/** Timer0 (8 bits) */
		TIMER0 = 0,
		/** Timer1 (16 bits) */
		TIMER1 = 1,
		/** Timer2 (8 bits) */
		TIMER2 = 2,
		/** Timer3 (16 bits) */
		TIMER3 = 3,
		/** Timer4 (16 bits) */
		TIMER4 = 4,
		/** Timer5 (16 bits) */
		TIMER5 = 5
	};
	
	/**
	 * Defines all available sleep modes for ATmega2560.
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
/// @endcond
#endif /* BOARDS_MEGA_HH */
/// @endcond
