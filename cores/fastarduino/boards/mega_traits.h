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

#ifndef BOARDS_MEGA_TRAITS_HH
#define BOARDS_MEGA_TRAITS_HH

#include "io.h"
#include "mega.h"
#include "common_traits.h"

namespace board_traits
{
	//====
	// IO
	//====
	template<> struct Port_trait<Port::PORT_A>: Port_trait_impl<R_(PINA), R_(DDRA), R_(PORTA), 0xFF> {};
	//	PCI0 = 0,			// PB0-7
	template<> struct Port_trait<Port::PORT_B>: Port_trait_impl<R_(PINB), R_(DDRB), R_(PORTB), 0xFF, 0> {};
	template<> struct Port_trait<Port::PORT_C>: Port_trait_impl<R_(PINC), R_(DDRC), R_(PORTC), 0xFF> {};
	template<> struct Port_trait<Port::PORT_D>: Port_trait_impl<R_(PIND), R_(DDRD), R_(PORTD), 0x8F> {};
	//	PCI1 = 1,			// PE0
	template<> struct Port_trait<Port::PORT_E>: Port_trait_impl<R_(PINE), R_(DDRE), R_(PORTE), 0x3B, 1> {};
	template<> struct Port_trait<Port::PORT_F>: Port_trait_impl<R_(PINF), R_(DDRF), R_(PORTF), 0xFF> {};
	template<> struct Port_trait<Port::PORT_G>: Port_trait_impl<R_(PING), R_(DDRG), R_(PORTG), 0x27> {};
	template<> struct Port_trait<Port::PORT_H>: Port_trait_impl<R_(PINH), R_(DDRH), R_(PORTH), 0x7B> {};
	//	PCI1 = 1,			// PJ0-1
	template<> struct Port_trait<Port::PORT_J>: Port_trait_impl<R_(PINJ), R_(DDRJ), R_(PORTJ), 0x03, 1, 1> {};
	//	PCI2 = 2			// PK0-7
	template<> struct Port_trait<Port::PORT_K>: Port_trait_impl<R_(PINK), R_(DDRK), R_(PORTK), 0xFF, 2> {};
	template<> struct Port_trait<Port::PORT_L>: Port_trait_impl<R_(PINL), R_(DDRL), R_(PORTL), 0xFF> {};
	
	/**
	 * Digital pin symbols
	 */
	template<> struct DigitalPin_trait<DigitalPin::NONE>: public DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D22_PA0>: public DigitalPin_trait_impl<Port::PORT_A, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D23_PA1>: public DigitalPin_trait_impl<Port::PORT_A, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D24_PA2>: public DigitalPin_trait_impl<Port::PORT_A, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D25_PA3>: public DigitalPin_trait_impl<Port::PORT_A, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D26_PA4>: public DigitalPin_trait_impl<Port::PORT_A, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D27_PA5>: public DigitalPin_trait_impl<Port::PORT_A, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D28_PA6>: public DigitalPin_trait_impl<Port::PORT_A, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D29_PA7>: public DigitalPin_trait_impl<Port::PORT_A, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D53_PB0>: public DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D52_PB1>: public DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D51_PB2>: public DigitalPin_trait_impl<Port::PORT_B, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D50_PB3>: public DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D10_PB4>: public DigitalPin_trait_impl<Port::PORT_B, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D11_PB5>: public DigitalPin_trait_impl<Port::PORT_B, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D12_PB6>: public DigitalPin_trait_impl<Port::PORT_B, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D13_PB7>: public DigitalPin_trait_impl<Port::PORT_B, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D37_PC0>: public DigitalPin_trait_impl<Port::PORT_C, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D36_PC1>: public DigitalPin_trait_impl<Port::PORT_C, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D35_PC2>: public DigitalPin_trait_impl<Port::PORT_C, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D34_PC3>: public DigitalPin_trait_impl<Port::PORT_C, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D33_PC4>: public DigitalPin_trait_impl<Port::PORT_C, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D32_PC5>: public DigitalPin_trait_impl<Port::PORT_C, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D31_PC6>: public DigitalPin_trait_impl<Port::PORT_C, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D30_PC7>: public DigitalPin_trait_impl<Port::PORT_C, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D21_PD0>: public DigitalPin_trait_impl<Port::PORT_D, 0, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D20_PD1>: public DigitalPin_trait_impl<Port::PORT_D, 1, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D19_PD2>: public DigitalPin_trait_impl<Port::PORT_D, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D18_PD3>: public DigitalPin_trait_impl<Port::PORT_D, 3, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D38_PD7>: public DigitalPin_trait_impl<Port::PORT_D, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D0_PE0>: public DigitalPin_trait_impl<Port::PORT_E, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1_PE1>: public DigitalPin_trait_impl<Port::PORT_E, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D5_PE3>: public DigitalPin_trait_impl<Port::PORT_E, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D2_PE4>: public DigitalPin_trait_impl<Port::PORT_E, 4, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D3_PE5>: public DigitalPin_trait_impl<Port::PORT_E, 5, true> {};

	template<> struct DigitalPin_trait<DigitalPin::D54_PF0>: public DigitalPin_trait_impl<Port::PORT_F, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D55_PF1>: public DigitalPin_trait_impl<Port::PORT_F, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D56_PF2>: public DigitalPin_trait_impl<Port::PORT_F, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D57_PF3>: public DigitalPin_trait_impl<Port::PORT_F, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D58_PF4>: public DigitalPin_trait_impl<Port::PORT_F, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D59_PF5>: public DigitalPin_trait_impl<Port::PORT_F, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D60_PF6>: public DigitalPin_trait_impl<Port::PORT_F, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D61_PF7>: public DigitalPin_trait_impl<Port::PORT_F, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D41_PG0>: public DigitalPin_trait_impl<Port::PORT_G, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D40_PG1>: public DigitalPin_trait_impl<Port::PORT_G, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D39_PG2>: public DigitalPin_trait_impl<Port::PORT_G, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D4_PG5>: public DigitalPin_trait_impl<Port::PORT_G, 5> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D17_PH0>: public DigitalPin_trait_impl<Port::PORT_H, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D16_PH1>: public DigitalPin_trait_impl<Port::PORT_H, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D6_PH3>: public DigitalPin_trait_impl<Port::PORT_H, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D7_PH4>: public DigitalPin_trait_impl<Port::PORT_H, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D8_PH5>: public DigitalPin_trait_impl<Port::PORT_H, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D9_PH6>: public DigitalPin_trait_impl<Port::PORT_H, 6> {};

	template<> struct DigitalPin_trait<DigitalPin::D15_PJ0>: public DigitalPin_trait_impl<Port::PORT_J, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D14_PJ1>: public DigitalPin_trait_impl<Port::PORT_J, 1> {};

	template<> struct DigitalPin_trait<DigitalPin::D62_PK0>: public DigitalPin_trait_impl<Port::PORT_K, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D63_PK1>: public DigitalPin_trait_impl<Port::PORT_K, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D64_PK2>: public DigitalPin_trait_impl<Port::PORT_K, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D65_PK3>: public DigitalPin_trait_impl<Port::PORT_K, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D66_PK4>: public DigitalPin_trait_impl<Port::PORT_K, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D67_PK5>: public DigitalPin_trait_impl<Port::PORT_K, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D68_PK6>: public DigitalPin_trait_impl<Port::PORT_K, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D69_PK7>: public DigitalPin_trait_impl<Port::PORT_K, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D49_PL0>: public DigitalPin_trait_impl<Port::PORT_L, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D48_PL1>: public DigitalPin_trait_impl<Port::PORT_L, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D47_PL2>: public DigitalPin_trait_impl<Port::PORT_L, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D46_PL3>: public DigitalPin_trait_impl<Port::PORT_L, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D45_PL4>: public DigitalPin_trait_impl<Port::PORT_L, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D44_PL5>: public DigitalPin_trait_impl<Port::PORT_L, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D43_PL6>: public DigitalPin_trait_impl<Port::PORT_L, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D42_PL7>: public DigitalPin_trait_impl<Port::PORT_L, 7> {};

	//==============
	// Analog Input
	//==============
	template<> struct AnalogReference_trait<AnalogReference::AREF>:AnalogReference_trait_impl<0> {};
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<bits::BV8(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_1_1V>:AnalogReference_trait_impl<bits::BV8(REFS1)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_2_56V>:AnalogReference_trait_impl<bits::BV8(REFS1, REFS0)> {};
	
	template<> struct AnalogSampleType_trait<uint16_t>: AnalogSampleType_trait_impl<uint16_t, 0, 0, R_(ADC)> {};
	template<> struct AnalogSampleType_trait<uint8_t>: AnalogSampleType_trait_impl<uint8_t, bits::BV8(ADLAR), 0, R_(ADCH)> {};

	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_50KHz>: AnalogClock_trait_impl<50000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_100KHz>: AnalogClock_trait_impl<100000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_200KHz>: AnalogClock_trait_impl<200000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_500KHz>: AnalogClock_trait_impl<500000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_1MHz>: AnalogClock_trait_impl<1000000UL> {};

	struct GlobalAnalogPin_trait:GlobalAnalogPin_trait_impl<R_(ADMUX), R_(ADCSRA), R_(ADCSRB)> {};
	
	template<> struct AnalogPin_trait<AnalogPin::A0>: AnalogPin_trait_impl<0> {};
	template<> struct AnalogPin_trait<AnalogPin::A1>: AnalogPin_trait_impl<bits::BV8(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A2>: AnalogPin_trait_impl<bits::BV8(MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A3>: AnalogPin_trait_impl<bits::BV8(MUX1, MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A4>: AnalogPin_trait_impl<bits::BV8(MUX2)> {};
	template<> struct AnalogPin_trait<AnalogPin::A5>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A6>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A7>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX1, MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<bits::BV8(MUX4, MUX3, MUX2, MUX1), 0, false, 1100> {};
	template<> struct AnalogPin_trait<AnalogPin::A8>: AnalogPin_trait_impl<0, bits::BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A9>: AnalogPin_trait_impl<bits::BV8(MUX0), bits::BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A10>: AnalogPin_trait_impl<bits::BV8(MUX1), bits::BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A11>: AnalogPin_trait_impl<bits::BV8(MUX1, MUX0), bits::BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A12>: AnalogPin_trait_impl<bits::BV8(MUX2), bits::BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A13>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX0), bits::BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A14>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX1), bits::BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A15>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX1, MUX0), bits::BV8(MUX5)> {};

	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D21_PD0_EXT0>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D21_PD0, 0, R_(EICRA), bits::BV8(ISC00, ISC01), R_(EIMSK), bits::BV8(INT0), R_(EIFR), bits::BV8(INTF0)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D20_PD1_EXT1>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D20_PD1, 1, R_(EICRA), bits::BV8(ISC10, ISC11), R_(EIMSK), bits::BV8(INT1), R_(EIFR), bits::BV8(INTF1)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D19_PD2_EXT2>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D19_PD2, 2, R_(EICRA), bits::BV8(ISC20, ISC21), R_(EIMSK), bits::BV8(INT2), R_(EIFR), bits::BV8(INTF2)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D18_PD3_EXT3>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D18_PD3, 3, R_(EICRA), bits::BV8(ISC30, ISC31), R_(EIMSK), bits::BV8(INT3), R_(EIFR), bits::BV8(INTF3)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D2_PE4_EXT4>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D2_PE4, 4, R_(EICRA), bits::BV8(ISC40, ISC41), R_(EIMSK), bits::BV8(INT4), R_(EIFR), bits::BV8(INTF4)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D3_PE5_EXT5>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D3_PE5, 5, R_(EICRA), bits::BV8(ISC50, ISC51), R_(EIMSK), bits::BV8(INT5), R_(EIFR), bits::BV8(INTF5)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<0xFF, bits::BV8(PCIE0), bits::BV8(PCIF0), R_(PCICR), R_(PCIFR), R_(PCMSK0)> {};
	template<> struct PCI_trait<1>: 
		PCI_trait_impl<0x07, bits::BV8(PCIE1), bits::BV8(PCIF1), R_(PCICR), R_(PCIFR), R_(PCMSK1)> {};
	template<> struct PCI_trait<2>: 
		PCI_trait_impl<0xFF, bits::BV8(PCIE2), bits::BV8(PCIF2), R_(PCICR), R_(PCIFR), R_(PCMSK2)> {};

	//=======
	// USART
	//=======
	template<> struct USART_trait<USART::USART0>: 
		USART_trait_impl<	R_(UCSR0A), R_(UCSR0B), R_(UCSR0C), R_(UDR0), R_(UBRR0), 
							U2X0, TXEN0, RXEN0, UDRIE0, RXCIE0, DOR0, FE0, UPE0> 
	{
		static constexpr uint8_t UCSRC_value(serial::Parity parity, serial::StopBits stopbits)
		{
			return	(	parity == serial::Parity::EVEN ? bits::BV8(UPM00) : 
						parity == serial::Parity::ODD ? bits::BV8(UPM00, UPM01) : 0x00)
					|	(stopbits == serial::StopBits::ONE ? 0x00 : bits::BV8(USBS0))
					|	bits::BV8(UCSZ00, UCSZ01);
		}
	};
	template<> struct USART_trait<USART::USART1>: 
		USART_trait_impl<	R_(UCSR1A), R_(UCSR1B), R_(UCSR1C), R_(UDR1), R_(UBRR1), 
							U2X1, TXEN1, RXEN1, UDRIE1, RXCIE1, DOR1, FE1, UPE1> 
	{
		static constexpr uint8_t UCSRC_value(serial::Parity parity, serial::StopBits stopbits)
		{
			return	(	parity == serial::Parity::EVEN ? bits::BV8(UPM10) : 
						parity == serial::Parity::ODD ? bits::BV8(UPM10, UPM11) : 0x00)
					|	(stopbits == serial::StopBits::ONE ? 0x00 : bits::BV8(USBS1))
					|	bits::BV8(UCSZ10, UCSZ11);
		}
	};
	template<> struct USART_trait<USART::USART2>: 
		USART_trait_impl<	R_(UCSR2A), R_(UCSR2B), R_(UCSR2C), R_(UDR2), R_(UBRR2), 
							U2X2, TXEN2, RXEN2, UDRIE2, RXCIE2, DOR2, FE2, UPE2> 
	{
		static constexpr uint8_t UCSRC_value(serial::Parity parity, serial::StopBits stopbits)
		{
			return	(	parity == serial::Parity::EVEN ? bits::BV8(UPM20) : 
						parity == serial::Parity::ODD ? bits::BV8(UPM20, UPM21) : 0x00)
					|	(stopbits == serial::StopBits::ONE ? 0x00 : bits::BV8(USBS2))
					|	bits::BV8(UCSZ20, UCSZ21);
		}
	};
	template<> struct USART_trait<USART::USART3>: 
		USART_trait_impl<	R_(UCSR3A), R_(UCSR3B), R_(UCSR3C), R_(UDR3), R_(UBRR3), 
							U2X3, TXEN3, RXEN3, UDRIE3, RXCIE3, DOR3, FE3, UPE3> 
	{
		static constexpr uint8_t UCSRC_value(serial::Parity parity, serial::StopBits stopbits)
		{
			return	(	parity == serial::Parity::EVEN ? bits::BV8(UPM30) : 
						parity == serial::Parity::ODD ? bits::BV8(UPM30, UPM31) : 0x00)
					|	(stopbits == serial::StopBits::ONE ? 0x00 : bits::BV8(USBS3))
					|	bits::BV8(UCSZ30, UCSZ31);
		}
	};

	//=====
	// SPI
	//=====
	struct SPI_trait: SPI_trait_impl<Port::PORT_B, PB0, PB2, PB3, PB1> {};
	
	//=====
	// I2C
	//=====
	struct TWI_trait: TWI_trait_impl<Port::PORT_D, PD0, PD1> {};

	//========
	// Timers
	//========
	template<> struct Timer_COM_trait<Timer::TIMER0, 0>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D13_PB7_OC0A, R_(OCR0A), 
		bits::BV8(COM0A0, COM0A1), 0, bits::BV8(COM0A0), bits::BV8(COM0A1), bits::BV8(COM0A0, COM0A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER0, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D4_PG5_OC0B, R_(OCR0B), 
		bits::BV8(COM0B0, COM0B1), 0, bits::BV8(COM0B0), bits::BV8(COM0B1), bits::BV8(COM0B0, COM0B1)> {};
	template<> struct Timer_trait<Timer::TIMER0>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							bits::BV8(WGM00, WGM01), bits::BV8(WGM02), bits::BV8(CS00, CS01, CS02),
							bits::BV8(WGM00, WGM01), 0,
							bits::BV8(WGM00), 0,
							bits::BV8(WGM01), 0,
							R_(TCCR0A), R_(TCCR0B), R_(TCNT0), R_(OCR0A), 
							R_(TIMSK0), R_(TIFR0)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? bits::BV8(CS00) :
					p == TIMER_PRESCALER::DIV_8 ? bits::BV8(CS01) :
					p == TIMER_PRESCALER::DIV_64 ? bits::BV8(CS00, CS01) :
					p == TIMER_PRESCALER::DIV_256 ? bits::BV8(CS02) :
					bits::BV8(CS02, CS00));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? bits::BV8(TOIE0) : 0)
				|	(i & OUTPUT_COMPARE_A ? bits::BV8(OCIE0A) : 0)
				|	(i & OUTPUT_COMPARE_B ? bits::BV8(OCIE0B) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER2, 0>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D10_PB4_OC2A, R_(OCR2A), 
		bits::BV8(COM2A0, COM2A1), 0, bits::BV8(COM2A0), bits::BV8(COM2A1), bits::BV8(COM2A0, COM2A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER2, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D9_PH6_OC2B, R_(OCR2B), 
		bits::BV8(COM2B0, COM2B1), 0, bits::BV8(COM2B0), bits::BV8(COM2B1), bits::BV8(COM2B0, COM2B1)> {};
	template<> struct Timer_trait<Timer::TIMER2>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_32_64_128_256_1024, 
							2,
							bits::BV8(WGM20, WGM21), bits::BV8(WGM22), bits::BV8(CS20, CS21, CS22),
							bits::BV8(WGM20, WGM21), 0,
							bits::BV8(WGM20), 0,
							bits::BV8(WGM21), 0,
							R_(TCCR2A), R_(TCCR2B), R_(TCNT2), R_(OCR2A), 
							R_(TIMSK2), R_(TIFR2)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? bits::BV8(CS20) :
					p == TIMER_PRESCALER::DIV_8 ? bits::BV8(CS21) :
					p == TIMER_PRESCALER::DIV_32 ? bits::BV8(CS21, CS20) :
					p == TIMER_PRESCALER::DIV_64 ? bits::BV8(CS22) :
					p == TIMER_PRESCALER::DIV_128 ? bits::BV8(CS22, CS20) :
					p == TIMER_PRESCALER::DIV_256 ? bits::BV8(CS22, CS21) :
					bits::BV8(CS22, CS21, CS20));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? bits::BV8(TOIE2) : 0)
				|	(i & OUTPUT_COMPARE_A ? bits::BV8(OCIE2A) : 0)
				|	(i & OUTPUT_COMPARE_B ? bits::BV8(OCIE2B) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER1, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D11_PB5_OC1A, R_(OCR1A), 
		bits::BV8(COM1A0, COM1A1), 0, bits::BV8(COM1A0), bits::BV8(COM1A1), bits::BV8(COM1A0, COM1A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D12_PB6_OC1B, R_(OCR1B), 
		bits::BV8(COM1B0, COM1B1), 0, bits::BV8(COM1B0), bits::BV8(COM1B1), bits::BV8(COM1B0, COM1B1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 2>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D13_PB7_OC1C, R_(OCR1C), 
		bits::BV8(COM1C0, COM1C1), 0, bits::BV8(COM1C0), bits::BV8(COM1C1), bits::BV8(COM1C0, COM1C1)> {};
	template<> struct Timer_trait<Timer::TIMER1>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							3,
							bits::BV8(WGM10, WGM11), bits::BV8(WGM12, WGM13), bits::BV8(CS10, CS11, CS12),
							bits::BV8(WGM10, WGM11), bits::BV8(WGM12),
							bits::BV8(WGM10, WGM11), 0,
							0, bits::BV8(WGM12), 
							R_(TCCR1A), R_(TCCR1B), R_(TCNT1), R_(OCR1A), 
							R_(TIMSK1), R_(TIFR1), 0xFF,
							R_(ICR1),
							0, bits::BV8(WGM12, WGM13),
							bits::BV8(WGM11), bits::BV8(WGM12, WGM13),
							bits::BV8(WGM11), bits::BV8(WGM13),
							DigitalPin::NONE, bits::BV8(ICES1), bits::BV8(ICNC1)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? bits::BV8(CS10) :
					p == TIMER_PRESCALER::DIV_8 ? bits::BV8(CS11) :
					p == TIMER_PRESCALER::DIV_64 ? bits::BV8(CS10, CS11) :
					p == TIMER_PRESCALER::DIV_256 ? bits::BV8(CS12) :
					bits::BV8(CS12, CS10));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? bits::BV8(TOIE1) : 0)
				|	(i & OUTPUT_COMPARE_A ? bits::BV8(OCIE1A) : 0)
				|	(i & OUTPUT_COMPARE_B ? bits::BV8(OCIE1B) : 0)
				|	(i & INPUT_CAPTURE ? bits::BV8(ICIE1) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER3, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D5_PE3_OC3A, R_(OCR3A), 
		bits::BV8(COM3A0, COM3A1), 0, bits::BV8(COM3A0), bits::BV8(COM3A1), bits::BV8(COM3A0, COM3A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER3, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D2_PE4_OC3B, R_(OCR3B), 
		bits::BV8(COM3B0, COM3B1), 0, bits::BV8(COM3B0), bits::BV8(COM3B1), bits::BV8(COM3B0, COM3B1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER3, 2>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D3_PE5_OC3C, R_(OCR3C), 
		bits::BV8(COM3C0, COM3C1), 0, bits::BV8(COM3C0), bits::BV8(COM3C1), bits::BV8(COM3C0, COM3C1)> {};
	template<> struct Timer_trait<Timer::TIMER3>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							3,
							bits::BV8(WGM30, WGM31), bits::BV8(WGM32, WGM33), bits::BV8(CS10, CS11, CS12),
							bits::BV8(WGM30, WGM31), bits::BV8(WGM32),
							bits::BV8(WGM30, WGM31), 0,
							0, bits::BV8(WGM32), 
							R_(TCCR3A), R_(TCCR3B), R_(TCNT3), R_(OCR3A), 
							R_(TIMSK3), R_(TIFR3), 0xFF,
							R_(ICR3),
							0, bits::BV8(WGM32, WGM33),
							bits::BV8(WGM31), bits::BV8(WGM32, WGM33),
							bits::BV8(WGM31), bits::BV8(WGM33),
							DigitalPin::NONE, bits::BV8(ICES3), bits::BV8(ICNC3)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? bits::BV8(CS30) :
					p == TIMER_PRESCALER::DIV_8 ? bits::BV8(CS31) :
					p == TIMER_PRESCALER::DIV_64 ? bits::BV8(CS30, CS31) :
					p == TIMER_PRESCALER::DIV_256 ? bits::BV8(CS32) :
					bits::BV8(CS32, CS30));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? bits::BV8(TOIE3) : 0)
				|	(i & OUTPUT_COMPARE_A ? bits::BV8(OCIE3A) : 0)
				|	(i & OUTPUT_COMPARE_B ? bits::BV8(OCIE3B) : 0)
				|	(i & INPUT_CAPTURE ? bits::BV8(ICIE3) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER4, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D6_PH3_OC4A, R_(OCR4A), 
		bits::BV8(COM4A0, COM4A1), 0, bits::BV8(COM4A0), bits::BV8(COM4A1), bits::BV8(COM4A0, COM4A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER4, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D7_PH4_OC4B, R_(OCR4B), 
		bits::BV8(COM4B0, COM4B1), 0, bits::BV8(COM4B0), bits::BV8(COM4B1), bits::BV8(COM4B0, COM4B1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER4, 2>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D8_PH5_OC4C, R_(OCR4C), 
		bits::BV8(COM4C0, COM4C1), 0, bits::BV8(COM4C0), bits::BV8(COM4C1), bits::BV8(COM4C0, COM4C1)> {};
	template<> struct Timer_trait<Timer::TIMER4>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							3,
							bits::BV8(WGM40, WGM41), bits::BV8(WGM42, WGM43), bits::BV8(CS10, CS11, CS12),
							bits::BV8(WGM40, WGM41), bits::BV8(WGM42),
							bits::BV8(WGM40, WGM41), 0,
							0, bits::BV8(WGM42), 
							R_(TCCR4A), R_(TCCR4B), R_(TCNT4), R_(OCR4A), 
							R_(TIMSK4), R_(TIFR4), 0xFF,
							R_(ICR4),
							0, bits::BV8(WGM42, WGM43),
							bits::BV8(WGM41), bits::BV8(WGM42, WGM43),
							bits::BV8(WGM41), bits::BV8(WGM43),
							DigitalPin::D49_PL0, bits::BV8(ICES4), bits::BV8(ICNC4)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? bits::BV8(CS40) :
					p == TIMER_PRESCALER::DIV_8 ? bits::BV8(CS41) :
					p == TIMER_PRESCALER::DIV_64 ? bits::BV8(CS40, CS41) :
					p == TIMER_PRESCALER::DIV_256 ? bits::BV8(CS42) :
					bits::BV8(CS42, CS40));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? bits::BV8(TOIE4) : 0)
				|	(i & OUTPUT_COMPARE_A ? bits::BV8(OCIE4A) : 0)
				|	(i & OUTPUT_COMPARE_B ? bits::BV8(OCIE4B) : 0)
				|	(i & INPUT_CAPTURE ? bits::BV8(ICIE4) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER5, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D46_PL3_OC5A, R_(OCR5A), 
		bits::BV8(COM5A0, COM5A1), 0, bits::BV8(COM5A0), bits::BV8(COM5A1), bits::BV8(COM5A0, COM5A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER5, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D45_PL4_OC5B, R_(OCR5B), 
		bits::BV8(COM5B0, COM5B1), 0, bits::BV8(COM5B0), bits::BV8(COM5B1), bits::BV8(COM5B0, COM5B1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER5, 2>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D44_PL5_OC5C, R_(OCR5C), 
		bits::BV8(COM5C0, COM5C1), 0, bits::BV8(COM5C0), bits::BV8(COM5C1), bits::BV8(COM5C0, COM5C1)> {};
	template<> struct Timer_trait<Timer::TIMER5>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							3,
							bits::BV8(WGM50, WGM51), bits::BV8(WGM52, WGM53), bits::BV8(CS10, CS11, CS12),
							bits::BV8(WGM50, WGM51), bits::BV8(WGM52),
							bits::BV8(WGM50, WGM51), 0,
							0, bits::BV8(WGM52), 
							R_(TCCR5A), R_(TCCR5B), R_(TCNT5), R_(OCR5A), 
							R_(TIMSK5), R_(TIFR5), 0xFF,
							R_(ICR5),
							0, bits::BV8(WGM52, WGM53),
							bits::BV8(WGM51), bits::BV8(WGM52, WGM53),
							bits::BV8(WGM51), bits::BV8(WGM53),
							DigitalPin::D48_PL1, bits::BV8(ICES5), bits::BV8(ICNC5)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? bits::BV8(CS50) :
					p == TIMER_PRESCALER::DIV_8 ? bits::BV8(CS51) :
					p == TIMER_PRESCALER::DIV_64 ? bits::BV8(CS50, CS51) :
					p == TIMER_PRESCALER::DIV_256 ? bits::BV8(CS52) :
					bits::BV8(CS52, CS50));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? bits::BV8(TOIE5) : 0)
				|	(i & OUTPUT_COMPARE_A ? bits::BV8(OCIE5A) : 0)
				|	(i & OUTPUT_COMPARE_B ? bits::BV8(OCIE5B) : 0)
				|	(i & INPUT_CAPTURE ? bits::BV8(ICIE5) : 0);
		}
	};
	
	template<> struct PWMPin_trait<PWMPin::D13_PB7_OC0A> : PWMPin_trait_impl<DigitalPin::D13_PB7, Timer::TIMER0, 0> {};
	template<> struct PWMPin_trait<PWMPin::D4_PG5_OC0B> : PWMPin_trait_impl<DigitalPin::D4_PG5, Timer::TIMER0, 1> {};
	template<> struct PWMPin_trait<PWMPin::D11_PB5_OC1A> : PWMPin_trait_impl<DigitalPin::D11_PB5, Timer::TIMER1, 0> {};
	template<> struct PWMPin_trait<PWMPin::D12_PB6_OC1B> : PWMPin_trait_impl<DigitalPin::D12_PB6, Timer::TIMER1, 1> {};
	template<> struct PWMPin_trait<PWMPin::D13_PB7_OC1C> : PWMPin_trait_impl<DigitalPin::D13_PB7, Timer::TIMER1, 2> {};
	template<> struct PWMPin_trait<PWMPin::D10_PB4_OC2A> : PWMPin_trait_impl<DigitalPin::D10_PB4, Timer::TIMER2, 0> {};
	template<> struct PWMPin_trait<PWMPin::D9_PH6_OC2B> : PWMPin_trait_impl<DigitalPin::D9_PH6, Timer::TIMER2, 1> {};
	template<> struct PWMPin_trait<PWMPin::D5_PE3_OC3A> : PWMPin_trait_impl<DigitalPin::D5_PE3, Timer::TIMER3, 0> {};
	template<> struct PWMPin_trait<PWMPin::D2_PE4_OC3B> : PWMPin_trait_impl<DigitalPin::D2_PE4, Timer::TIMER3, 1> {};
	template<> struct PWMPin_trait<PWMPin::D3_PE5_OC3C> : PWMPin_trait_impl<DigitalPin::D3_PE5, Timer::TIMER3, 2> {};
	template<> struct PWMPin_trait<PWMPin::D6_PH3_OC4A> : PWMPin_trait_impl<DigitalPin::D6_PH3, Timer::TIMER4, 0> {};
	template<> struct PWMPin_trait<PWMPin::D7_PH4_OC4B> : PWMPin_trait_impl<DigitalPin::D7_PH4, Timer::TIMER4, 1> {};
	template<> struct PWMPin_trait<PWMPin::D8_PH5_OC4C> : PWMPin_trait_impl<DigitalPin::D8_PH5, Timer::TIMER4, 2> {};
	template<> struct PWMPin_trait<PWMPin::D46_PL3_OC5A> : PWMPin_trait_impl<DigitalPin::D46_PL3, Timer::TIMER5, 0> {};
	template<> struct PWMPin_trait<PWMPin::D45_PL4_OC5B> : PWMPin_trait_impl<DigitalPin::D45_PL4, Timer::TIMER5, 1> {};
	template<> struct PWMPin_trait<PWMPin::D44_PL5_OC5C> : PWMPin_trait_impl<DigitalPin::D44_PL5, Timer::TIMER5, 2> {};
};

// Macros to declare some ISR friends
#define DECL_INT_ISR_FRIENDS 		\
	friend void ::INT0_vect(void);	\
	friend void ::INT1_vect(void);	\
	friend void ::INT2_vect(void);	\
	friend void ::INT3_vect(void);	\
	friend void ::INT4_vect(void);	\
	friend void ::INT5_vect(void);	\
	friend void ::INT6_vect(void);	\
	friend void ::INT7_vect(void);
#define DECL_PCINT_ISR_FRIENDS		\
	friend void ::PCINT0_vect(void);\
	friend void ::PCINT1_vect(void);\
	friend void ::PCINT2_vect(void);
#define DECL_TIMER_COMP_FRIENDS				\
	friend void ::TIMER0_COMPA_vect(void);	\
	friend void ::TIMER1_COMPA_vect(void);	\
	friend void ::TIMER2_COMPA_vect(void);	\
	friend void ::TIMER3_COMPA_vect(void);	\
	friend void ::TIMER4_COMPA_vect(void);	\
	friend void ::TIMER5_COMPA_vect(void);	\
	friend void ::TIMER0_COMPB_vect(void);	\
	friend void ::TIMER1_COMPB_vect(void);	\
	friend void ::TIMER2_COMPB_vect(void);	\
	friend void ::TIMER3_COMPB_vect(void);	\
	friend void ::TIMER4_COMPB_vect(void);	\
	friend void ::TIMER5_COMPB_vect(void);	\
	friend void ::TIMER1_COMPC_vect(void);	\
	friend void ::TIMER3_COMPC_vect(void);	\
	friend void ::TIMER4_COMPC_vect(void);	\
	friend void ::TIMER5_COMPC_vect(void);
#define DECL_TIMER_OVF_FRIENDS			\
	friend void ::TIMER0_OVF_vect(void);\
	friend void ::TIMER1_OVF_vect(void);\
	friend void ::TIMER2_OVF_vect(void);\
	friend void ::TIMER3_OVF_vect(void);\
	friend void ::TIMER4_OVF_vect(void);\
	friend void ::TIMER5_OVF_vect(void);
#define DECL_TIMER_CAPT_FRIENDS				\
	friend void ::TIMER1_CAPT_vect(void);	\
	friend void ::TIMER3_CAPT_vect(void);	\
	friend void ::TIMER4_CAPT_vect(void);	\
	friend void ::TIMER5_CAPT_vect(void);
#define DECL_UDRE_ISR_FRIENDS				\
	friend void ::USART0_UDRE_vect(void);	\
	friend void ::USART1_UDRE_vect(void);	\
	friend void ::USART2_UDRE_vect(void);	\
	friend void ::USART3_UDRE_vect(void);
#define DECL_RX_ISR_FRIENDS				\
	friend void ::USART0_RX_vect(void);	\
	friend void ::USART1_RX_vect(void);	\
	friend void ::USART2_RX_vect(void);	\
	friend void ::USART3_RX_vect(void);

#endif /* BOARDS_MEGA_TRAITS_HH */
