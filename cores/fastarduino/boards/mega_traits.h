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

#ifndef BOARDS_MEGA_TRAITS_HH
#define BOARDS_MEGA_TRAITS_HH

#include <avr/io.h>
#include "mega.h"
#include "common_traits.h"

namespace board_traits
{
	//====
	// IO
	//====
	template<> struct Port_trait<Port::PORT_A>: Port_trait_impl<R_(PINA), R_(DDRA), R_(PORTA), 0xFF, 0> {};
	//	PCI0 = 0,			// PB0-7
	template<> struct Port_trait<Port::PORT_B>: Port_trait_impl<R_(PINB), R_(DDRB), R_(PORTB), 0xFF, 0> {};
	template<> struct Port_trait<Port::PORT_C>: Port_trait_impl<R_(PINC), R_(DDRC), R_(PORTC), 0xFF, 0> {};
	template<> struct Port_trait<Port::PORT_D>: Port_trait_impl<R_(PIND), R_(DDRD), R_(PORTD), 0x8F, 0> {};
	template<> struct Port_trait<Port::PORT_E>: Port_trait_impl<R_(PINE), R_(DDRE), R_(PORTE), 0x3B, 0> {};
	template<> struct Port_trait<Port::PORT_F>: Port_trait_impl<R_(PINF), R_(DDRF), R_(PORTF), 0xFF, 0> {};
	template<> struct Port_trait<Port::PORT_G>: Port_trait_impl<R_(PING), R_(DDRG), R_(PORTG), 0x27, 0> {};
	template<> struct Port_trait<Port::PORT_H>: Port_trait_impl<R_(PINH), R_(DDRH), R_(PORTH), 0x7B, 0> {};
//	PCI1 = 1,			// PJ0-1
	template<> struct Port_trait<Port::PORT_J>: Port_trait_impl<R_(PINJ), R_(DDRJ), R_(PORTJ), 0x03, 1> {};
//	PCI2 = 2			// PK0-7
	template<> struct Port_trait<Port::PORT_K>: Port_trait_impl<R_(PINK), R_(DDRK), R_(PORTK), 0xFF, 2> {};
	template<> struct Port_trait<Port::PORT_L>: Port_trait_impl<R_(PINL), R_(DDRL), R_(PORTL), 0xFF, 0> {};
	
	/**
	 * Digital pin symbols
	 */
	template<> struct DigitalPin_trait<DigitalPin::NONE>: public DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D22>: public DigitalPin_trait_impl<Port::PORT_A, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D23>: public DigitalPin_trait_impl<Port::PORT_A, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D24>: public DigitalPin_trait_impl<Port::PORT_A, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D25>: public DigitalPin_trait_impl<Port::PORT_A, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D26>: public DigitalPin_trait_impl<Port::PORT_A, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D27>: public DigitalPin_trait_impl<Port::PORT_A, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D28>: public DigitalPin_trait_impl<Port::PORT_A, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D29>: public DigitalPin_trait_impl<Port::PORT_A, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D53>: public DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D52>: public DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D51>: public DigitalPin_trait_impl<Port::PORT_B, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D50>: public DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D10>: public DigitalPin_trait_impl<Port::PORT_B, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D11>: public DigitalPin_trait_impl<Port::PORT_B, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D12>: public DigitalPin_trait_impl<Port::PORT_B, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D13>: public DigitalPin_trait_impl<Port::PORT_B, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D37>: public DigitalPin_trait_impl<Port::PORT_C, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D36>: public DigitalPin_trait_impl<Port::PORT_C, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D35>: public DigitalPin_trait_impl<Port::PORT_C, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D34>: public DigitalPin_trait_impl<Port::PORT_C, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D33>: public DigitalPin_trait_impl<Port::PORT_C, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D32>: public DigitalPin_trait_impl<Port::PORT_C, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D31>: public DigitalPin_trait_impl<Port::PORT_C, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D30>: public DigitalPin_trait_impl<Port::PORT_C, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D21>: public DigitalPin_trait_impl<Port::PORT_D, 0, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D20>: public DigitalPin_trait_impl<Port::PORT_D, 1, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D19>: public DigitalPin_trait_impl<Port::PORT_D, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D18>: public DigitalPin_trait_impl<Port::PORT_D, 3, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D38>: public DigitalPin_trait_impl<Port::PORT_D, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D0>: public DigitalPin_trait_impl<Port::PORT_E, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1>: public DigitalPin_trait_impl<Port::PORT_E, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D5>: public DigitalPin_trait_impl<Port::PORT_E, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D2>: public DigitalPin_trait_impl<Port::PORT_E, 4, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D3>: public DigitalPin_trait_impl<Port::PORT_E, 5, true> {};

	template<> struct DigitalPin_trait<DigitalPin::D54>: public DigitalPin_trait_impl<Port::PORT_F, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D55>: public DigitalPin_trait_impl<Port::PORT_F, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D56>: public DigitalPin_trait_impl<Port::PORT_F, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D57>: public DigitalPin_trait_impl<Port::PORT_F, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D58>: public DigitalPin_trait_impl<Port::PORT_F, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D59>: public DigitalPin_trait_impl<Port::PORT_F, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D60>: public DigitalPin_trait_impl<Port::PORT_F, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D61>: public DigitalPin_trait_impl<Port::PORT_F, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D41>: public DigitalPin_trait_impl<Port::PORT_G, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D40>: public DigitalPin_trait_impl<Port::PORT_G, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D39>: public DigitalPin_trait_impl<Port::PORT_G, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D4>: public DigitalPin_trait_impl<Port::PORT_G, 5> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D17>: public DigitalPin_trait_impl<Port::PORT_H, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D16>: public DigitalPin_trait_impl<Port::PORT_H, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D6>: public DigitalPin_trait_impl<Port::PORT_H, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D7>: public DigitalPin_trait_impl<Port::PORT_H, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D8>: public DigitalPin_trait_impl<Port::PORT_H, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D9>: public DigitalPin_trait_impl<Port::PORT_H, 6> {};

	template<> struct DigitalPin_trait<DigitalPin::D15>: public DigitalPin_trait_impl<Port::PORT_J, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D14>: public DigitalPin_trait_impl<Port::PORT_J, 1> {};

	template<> struct DigitalPin_trait<DigitalPin::D62>: public DigitalPin_trait_impl<Port::PORT_K, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D63>: public DigitalPin_trait_impl<Port::PORT_K, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D64>: public DigitalPin_trait_impl<Port::PORT_K, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D65>: public DigitalPin_trait_impl<Port::PORT_K, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D66>: public DigitalPin_trait_impl<Port::PORT_K, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D67>: public DigitalPin_trait_impl<Port::PORT_K, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D68>: public DigitalPin_trait_impl<Port::PORT_K, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D69>: public DigitalPin_trait_impl<Port::PORT_K, 7> {};

	template<> struct DigitalPin_trait<DigitalPin::D49>: public DigitalPin_trait_impl<Port::PORT_L, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D48>: public DigitalPin_trait_impl<Port::PORT_L, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D47>: public DigitalPin_trait_impl<Port::PORT_L, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D46>: public DigitalPin_trait_impl<Port::PORT_L, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D45>: public DigitalPin_trait_impl<Port::PORT_L, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D44>: public DigitalPin_trait_impl<Port::PORT_L, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D43>: public DigitalPin_trait_impl<Port::PORT_L, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D42>: public DigitalPin_trait_impl<Port::PORT_L, 7> {};

	//==============
	// Analog Input
	//==============
	template<> struct AnalogReference_trait<AnalogReference::AREF>:AnalogReference_trait_impl<0> {};
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<_BV(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_1_1V>:AnalogReference_trait_impl<_BV(REFS1)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_2_56V>:AnalogReference_trait_impl<_BV(REFS1) | _BV(REFS0)> {};
	
	template<> struct AnalogSampleType_trait<uint16_t>: AnalogSampleType_trait_impl<0, 0, R_(ADC)> {};
	template<> struct AnalogSampleType_trait<uint8_t>: AnalogSampleType_trait_impl<_BV(ADLAR), 0, R_(ADCH)> {};

	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_50KHz>: AnalogClock_trait_impl<50000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_100KHz>: AnalogClock_trait_impl<100000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_200KHz>: AnalogClock_trait_impl<200000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_500KHz>: AnalogClock_trait_impl<500000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_1MHz>: AnalogClock_trait_impl<1000000UL> {};

	struct GlobalAnalogPin_trait:GlobalAnalogPin_trait_impl<R_(ADMUX), R_(ADCSRA), R_(ADCSRB)> {};
	
	template<> struct AnalogPin_trait<AnalogPin::A0>: AnalogPin_trait_impl<0> {};
	template<> struct AnalogPin_trait<AnalogPin::A1>: AnalogPin_trait_impl<_BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A2>: AnalogPin_trait_impl<_BV(MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A3>: AnalogPin_trait_impl<_BV(MUX1) | _BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A4>: AnalogPin_trait_impl<_BV(MUX2)> {};
	template<> struct AnalogPin_trait<AnalogPin::A5>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A6>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A7>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX1) | _BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<_BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1), 0, 1100> {};
	template<> struct AnalogPin_trait<AnalogPin::A8>: AnalogPin_trait_impl<0, _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A9>: AnalogPin_trait_impl<_BV(MUX0), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A10>: AnalogPin_trait_impl<_BV(MUX1), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A11>: AnalogPin_trait_impl<_BV(MUX1) | _BV(MUX0), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A12>: AnalogPin_trait_impl<_BV(MUX2), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A13>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX0), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A14>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX1), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A15>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX1) | _BV(MUX0), _BV(MUX5)> {};

	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D21_EXT0>: 
		ExternalInterruptPin_trait_impl<0, R_(EICRA), _BV(ISC00) | _BV(ISC01), R_(EIMSK), _BV(INT0), R_(EIFR), _BV(INTF0)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D20_EXT1>: 
		ExternalInterruptPin_trait_impl<1, R_(EICRA), _BV(ISC10) | _BV(ISC11), R_(EIMSK), _BV(INT1), R_(EIFR), _BV(INTF1)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D19_EXT2>: 
		ExternalInterruptPin_trait_impl<2, R_(EICRA), _BV(ISC20) | _BV(ISC21), R_(EIMSK), _BV(INT2), R_(EIFR), _BV(INTF2)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D18_EXT3>: 
		ExternalInterruptPin_trait_impl<3, R_(EICRA), _BV(ISC30) | _BV(ISC31), R_(EIMSK), _BV(INT3), R_(EIFR), _BV(INTF3)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D2_EXT4>: 
		ExternalInterruptPin_trait_impl<4, R_(EICRA), _BV(ISC40) | _BV(ISC41), R_(EIMSK), _BV(INT4), R_(EIFR), _BV(INTF4)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D3_EXT5>: 
		ExternalInterruptPin_trait_impl<5, R_(EICRA), _BV(ISC50) | _BV(ISC51), R_(EIMSK), _BV(INT5), R_(EIFR), _BV(INTF5)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<Port::PORT_B, 0xFF, _BV(PCIE0), _BV(PCIF0), R_(PCICR), R_(PCIFR), R_(PCMSK0)> {};
	template<> struct PCI_trait<1>: 
		PCI_trait_impl<Port::PORT_J, 0x03, _BV(PCIE1), _BV(PCIF1), R_(PCICR), R_(PCIFR), R_(PCMSK1)> {};
	template<> struct PCI_trait<2>: 
		PCI_trait_impl<Port::PORT_K, 0xFF, _BV(PCIE2), _BV(PCIF2), R_(PCICR), R_(PCIFR), R_(PCMSK2)> {};

	//=======
	// USART
	//=======
	template<> struct USART_trait<USART::USART0>: USART_trait_impl<R_(UCSR0A), R_(UCSR0B), R_(UCSR0C), R_(UDR0), R_(UBRR0)> {};
	template<> struct USART_trait<USART::USART1>: USART_trait_impl<R_(UCSR1A), R_(UCSR1B), R_(UCSR1C), R_(UDR1), R_(UBRR1)> {};
	template<> struct USART_trait<USART::USART2>: USART_trait_impl<R_(UCSR2A), R_(UCSR2B), R_(UCSR2C), R_(UDR2), R_(UBRR2)> {};
	template<> struct USART_trait<USART::USART3>: USART_trait_impl<R_(UCSR3A), R_(UCSR3B), R_(UCSR3C), R_(UDR3), R_(UBRR3)> {};

	//=====
	// SPI
	//=====
	struct SPI_trait: SPI_trait_impl<Port::PORT_B, PB0, PB2, PB3, PB1> {};
	
	//========
	// Timers
	//========
	template<> struct Timer_trait<Timer::TIMER0>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							_BV(WGM01), 0, R_(TCCR0A), R_(TCCR0B), R_(TCNT0), 
							R_(OCR0A), R_(OCR0B), R_(TIMSK0), R_(TIFR0)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS00) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS01) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS00) | _BV(CS01) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS02) :
					_BV(CS02) | _BV(CS00));
		}
	};
	template<> struct Timer_trait<Timer::TIMER2>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_32_64_128_256_1024, 
							_BV(WGM21), 0, R_(TCCR2A), R_(TCCR2B), R_(TCNT2), 
							R_(OCR2A), R_(OCR2B), R_(TIMSK2), R_(TIFR2)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS20) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS21) :
					p == TIMER_PRESCALER::DIV_32 ? _BV(CS21) | _BV(CS20) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS22) :
					p == TIMER_PRESCALER::DIV_128 ? _BV(CS22) | _BV(CS20) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS22) | _BV(CS21) :
					_BV(CS22) | _BV(CS21) | _BV(CS20));
		}
	};
	template<> struct Timer_trait<Timer::TIMER1>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							0, _BV(WGM12), R_(TCCR1A), R_(TCCR1B), R_(TCNT1), 
							R_(OCR1A), R_(OCR1B), R_(TIMSK1), R_(TIFR1)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS10) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS11) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS10) | _BV(CS11) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS12) :
					_BV(CS12) | _BV(CS10));
		}
	};
	template<> struct Timer_trait<Timer::TIMER3>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							0, _BV(WGM32), R_(TCCR3A), R_(TCCR3B), R_(TCNT3), 
							R_(OCR3A), R_(OCR3B), R_(TIMSK3), R_(TIFR3)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS30) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS31) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS30) | _BV(CS31) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS32) :
					_BV(CS32) | _BV(CS30));
		}
	};
	template<> struct Timer_trait<Timer::TIMER4>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							0, _BV(WGM42), R_(TCCR4A), R_(TCCR4B), R_(TCNT4), 
							R_(OCR4A), R_(OCR4B), R_(TIMSK4), R_(TIFR4)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS40) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS41) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS40) | _BV(CS41) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS42) :
					_BV(CS42) | _BV(CS40));
		}
	};
	template<> struct Timer_trait<Timer::TIMER5>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							0, _BV(WGM52), R_(TCCR5A), R_(TCCR5B), R_(TCNT5), 
							R_(OCR5A), R_(OCR5B), R_(TIMSK5), R_(TIFR5)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS50) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS51) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS50) | _BV(CS51) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS52) :
					_BV(CS52) | _BV(CS50));
		}
	};
};

#endif /* BOARDS_MEGA_TRAITS_HH */
