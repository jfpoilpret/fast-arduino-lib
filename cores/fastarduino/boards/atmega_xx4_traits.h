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

#ifndef BOARDS_ATMEGA_XX4_TRAITS_HH
#define BOARDS_ATMEGA_XX4_TRAITS_HH

#include "io.h"
#include "atmega_xx4.h"
#include "common_traits.h"

namespace board_traits
{
	//====
	// IO
	//====
	//	PCI0 = 0,			// PA0-7
	template<> struct Port_trait<Port::PORT_A>: Port_trait_impl<R_(PINA), R_(DDRA), R_(PORTA), 0xFF, 0> {};
	//	PCI1 = 1,			// PB0-7
	template<> struct Port_trait<Port::PORT_B>: Port_trait_impl<R_(PINB), R_(DDRB), R_(PORTB), 0xFF, 1> {};
	//	PCI2 = 2,			// PC0-7
	template<> struct Port_trait<Port::PORT_C>: Port_trait_impl<R_(PINC), R_(DDRC), R_(PORTC), 0xFF, 2> {};
	//	PCI2 = 2,			// PD0-7
	template<> struct Port_trait<Port::PORT_D>: Port_trait_impl<R_(PIND), R_(DDRD), R_(PORTD), 0xFF, 3> {};
	
	/**
	 * Digital pin symbols
	 */
	template<> struct DigitalPin_trait<DigitalPin::NONE>: public DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D0_PA0>: public DigitalPin_trait_impl<Port::PORT_A, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1_PA1>: public DigitalPin_trait_impl<Port::PORT_A, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D2_PA2>: public DigitalPin_trait_impl<Port::PORT_A, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D3_PA3>: public DigitalPin_trait_impl<Port::PORT_A, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D4_PA4>: public DigitalPin_trait_impl<Port::PORT_A, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D5_PA5>: public DigitalPin_trait_impl<Port::PORT_A, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D6_PA6>: public DigitalPin_trait_impl<Port::PORT_A, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D7_PA7>: public DigitalPin_trait_impl<Port::PORT_A, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D8_PB0>: public DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D9_PB1>: public DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D10_PB2>: public DigitalPin_trait_impl<Port::PORT_B, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D11_PB3>: public DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D12_PB4>: public DigitalPin_trait_impl<Port::PORT_B, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D13_PB5>: public DigitalPin_trait_impl<Port::PORT_B, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D14_PB6>: public DigitalPin_trait_impl<Port::PORT_B, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D15_PB7>: public DigitalPin_trait_impl<Port::PORT_B, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D16_PC0>: public DigitalPin_trait_impl<Port::PORT_C, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D17_PC1>: public DigitalPin_trait_impl<Port::PORT_C, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D18_PC2>: public DigitalPin_trait_impl<Port::PORT_C, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D19_PC3>: public DigitalPin_trait_impl<Port::PORT_C, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D20_PC4>: public DigitalPin_trait_impl<Port::PORT_C, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D21_PC5>: public DigitalPin_trait_impl<Port::PORT_C, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D22_PC6>: public DigitalPin_trait_impl<Port::PORT_C, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D23_PC7>: public DigitalPin_trait_impl<Port::PORT_C, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D24_PD0>: public DigitalPin_trait_impl<Port::PORT_D, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D25_PD1>: public DigitalPin_trait_impl<Port::PORT_D, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D26_PD2>: public DigitalPin_trait_impl<Port::PORT_D, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D27_PD3>: public DigitalPin_trait_impl<Port::PORT_D, 3, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D28_PD4>: public DigitalPin_trait_impl<Port::PORT_D, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D29_PD5>: public DigitalPin_trait_impl<Port::PORT_D, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D30_PD6>: public DigitalPin_trait_impl<Port::PORT_D, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D31_PD7>: public DigitalPin_trait_impl<Port::PORT_D, 7> {};

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

	struct GlobalAnalogPin_trait:GlobalAnalogPin_trait_impl<R_(ADMUX), R_(ADCSRA), R_(ADCSRB), bits::BV8(ACIC)> {};
	
	template<> struct AnalogPin_trait<AnalogPin::A0>: AnalogPin_trait_impl<0> {};
	template<> struct AnalogPin_trait<AnalogPin::A1>: AnalogPin_trait_impl<bits::BV8(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A2>: AnalogPin_trait_impl<bits::BV8(MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A3>: AnalogPin_trait_impl<bits::BV8(MUX1, MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A4>: AnalogPin_trait_impl<bits::BV8(MUX2)> {};
	template<> struct AnalogPin_trait<AnalogPin::A5>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A6>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A7>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX1, MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<bits::BV8(MUX4, MUX3, MUX2, MUX1), 0, false, 1100> {};

	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D26_PD2_EXT0>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D26_PD2, 0, R_(EICRA), bits::BV8(ISC00, ISC01), R_(EIMSK), bits::BV8(INT0), R_(EIFR), bits::BV8(INTF0)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D27_PD3_EXT1>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D27_PD3, 1, R_(EICRA), bits::BV8(ISC10, ISC11), R_(EIMSK), bits::BV8(INT1), R_(EIFR), bits::BV8(INTF1)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D10_PB2_EXT2>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D10_PB2, 2, R_(EICRA), bits::BV8(ISC20, ISC21), R_(EIMSK), bits::BV8(INT2), R_(EIFR), bits::BV8(INTF2)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<0xFF, bits::BV8(PCIE0), bits::BV8(PCIF0), R_(PCICR), R_(PCIFR), R_(PCMSK0)> {};
	template<> struct PCI_trait<1>: 
		PCI_trait_impl<0xFF, bits::BV8(PCIE1), bits::BV8(PCIF1), R_(PCICR), R_(PCIFR), R_(PCMSK1)> {};
	template<> struct PCI_trait<2>: 
		PCI_trait_impl<0xFF, bits::BV8(PCIE2), bits::BV8(PCIF2), R_(PCICR), R_(PCIFR), R_(PCMSK2)> {};
	template<> struct PCI_trait<3>: 
		PCI_trait_impl<0xFF, bits::BV8(PCIE3), bits::BV8(PCIF3), R_(PCICR), R_(PCIFR), R_(PCMSK3)> {};

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

	//=====
	// SPI
	//=====
	struct SPI_trait: SPI_trait_impl<Port::PORT_B, PB4, PB5, PB6, PB7> {};
	
	//=====
	// I2C
	//=====
	struct TWI_trait: TWI_trait_impl<Port::PORT_C, PC0, PC1> {};

	//========
	// Timers
	//========
	template<> struct Timer_COM_trait<Timer::TIMER0, 0>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D11_PB3_OC0A, R_(OCR0A), 
		bits::BV8(COM0A0, COM0A1), 0, bits::BV8(COM0A0), bits::BV8(COM0A1), bits::BV8(COM0A0, COM0A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER0, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D12_PB4_OC0B, R_(OCR0B), 
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
		uint8_t, PWMPin::D31_PD7_OC2A, R_(OCR2A), 
		bits::BV8(COM2A0, COM2A1), 0, bits::BV8(COM2A0), bits::BV8(COM2A1), bits::BV8(COM2A0, COM2A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER2, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D30_PD6_OC2B, R_(OCR2B), 
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
		uint16_t, PWMPin::D29_PD5_OC1A, R_(OCR1A), 
		bits::BV8(COM1A0, COM1A1), 0, bits::BV8(COM1A0), bits::BV8(COM1A1), bits::BV8(COM1A0, COM1A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D28_PD4_OC1B, R_(OCR1B), 
		bits::BV8(COM1B0, COM1B1), 0, bits::BV8(COM1B0), bits::BV8(COM1B1), bits::BV8(COM1B0, COM1B1)> {};
	template<> struct Timer_trait<Timer::TIMER1>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
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
							DigitalPin::D30_PD6, bits::BV8(ICES1), bits::BV8(ICNC1)>
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

#ifdef __AVR_ATmega1284P__
	template<> struct Timer_COM_trait<Timer::TIMER3, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D14_PB6_OC3A, R_(OCR3A), 
		bits::BV8(COM3A0, COM3A1), 0, bits::BV8(COM3A0), bits::BV8(COM3A1), bits::BV8(COM3A0, COM3A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER3, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D15_PB7_OC3B, R_(OCR3B), 
		bits::BV8(COM3B0, COM3B1), 0, bits::BV8(COM3B0), bits::BV8(COM3B1), bits::BV8(COM3B0, COM3B1)> {};
	template<> struct Timer_trait<Timer::TIMER3>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							bits::BV8(WGM30, WGM31), bits::BV8(WGM32, WGM33), bits::BV8(CS30, CS31, CS32),
							bits::BV8(WGM30, WGM31), bits::BV8(WGM32),
							bits::BV8(WGM30, WGM31), 0,
							0, bits::BV8(WGM32), 
							R_(TCCR3A), R_(TCCR3B), R_(TCNT3), R_(OCR3A), 
							R_(TIMSK3), R_(TIFR3), 0xFF,
							R_(ICR3),
							0, bits::BV8(WGM32, WGM33),
							bits::BV8(WGM31), bits::BV8(WGM32, WGM33),
							bits::BV8(WGM31), bits::BV8(WGM33),
							DigitalPin::D13_PB5, bits::BV8(ICES3), bits::BV8(ICNC3)>
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
#endif

	template<> struct PWMPin_trait<PWMPin::D11_PB3_OC0A> : PWMPin_trait_impl<DigitalPin::D11_PB3, Timer::TIMER0, 0> {};
	template<> struct PWMPin_trait<PWMPin::D12_PB4_OC0B> : PWMPin_trait_impl<DigitalPin::D12_PB4, Timer::TIMER0, 1> {};
	template<> struct PWMPin_trait<PWMPin::D29_PD5_OC1A> : PWMPin_trait_impl<DigitalPin::D29_PD5, Timer::TIMER1, 0> {};
	template<> struct PWMPin_trait<PWMPin::D28_PD4_OC1B> : PWMPin_trait_impl<DigitalPin::D28_PD4, Timer::TIMER1, 1> {};
	template<> struct PWMPin_trait<PWMPin::D31_PD7_OC2A> : PWMPin_trait_impl<DigitalPin::D31_PD7, Timer::TIMER2, 0> {};
	template<> struct PWMPin_trait<PWMPin::D30_PD6_OC2B> : PWMPin_trait_impl<DigitalPin::D30_PD6, Timer::TIMER2, 1> {};
#ifdef __AVR_ATmega1284P__
	template<> struct PWMPin_trait<PWMPin::D14_PB6_OC3A> : PWMPin_trait_impl<DigitalPin::D14_PB6, Timer::TIMER3, 0> {};
	template<> struct PWMPin_trait<PWMPin::D15_PB7_OC3B> : PWMPin_trait_impl<DigitalPin::D15_PB7, Timer::TIMER3, 1> {};
#endif
};

// Macros to declare some ISR friends
#define DECL_INT_ISR_FRIENDS 		\
	friend void ::INT0_vect(void);	\
	friend void ::INT1_vect(void);	\
	friend void ::INT2_vect(void);
#define DECL_PCINT_ISR_FRIENDS		\
	friend void ::PCINT0_vect(void);\
	friend void ::PCINT1_vect(void);\
	friend void ::PCINT2_vect(void);\
	friend void ::PCINT3_vect(void);
#define DECL_TIMER_COMP_FRIENDS				\
	friend void ::TIMER0_COMPA_vect(void);	\
	friend void ::TIMER1_COMPA_vect(void);	\
	friend void ::TIMER2_COMPA_vect(void);	\
	friend void ::TIMER0_COMPB_vect(void);	\
	friend void ::TIMER1_COMPB_vect(void);	\
	friend void ::TIMER2_COMPB_vect(void);
#define DECL_TIMER_OVF_FRIENDS			\
	friend void ::TIMER0_OVF_vect(void);\
	friend void ::TIMER1_OVF_vect(void);\
	friend void ::TIMER2_OVF_vect(void);
#define DECL_TIMER_CAPT_FRIENDS				\
	friend void ::TIMER1_CAPT_vect(void);
#define DECL_UDRE_ISR_FRIENDS				\
	friend void ::USART0_UDRE_vect(void);	\
	friend void ::USART1_UDRE_vect(void);
#define DECL_RX_ISR_FRIENDS				\
	friend void ::USART0_RX_vect(void);	\
	friend void ::USART1_RX_vect(void);
#define DECL_TWI_FRIENDS friend void ::TWI_vect(void);

#endif /* BOARDS_ATMEGA_XX4_TRAITS_HH */
