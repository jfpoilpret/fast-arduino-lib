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

#ifndef BOARDS_UNO_TRAITS_HH
#define BOARDS_UNO_TRAITS_HH

#include <avr/io.h>
#include "uno.h"
#include "common_traits.h"

namespace board_traits
{
	//====
	// IO
	//====
	template<> struct Port_trait<Port::PORT_B>: Port_trait_impl<R_(PINB), R_(DDRB), R_(PORTB), 0xFF, 0> {};
	template<> struct Port_trait<Port::PORT_C>: Port_trait_impl<R_(PINC), R_(DDRC), R_(PORTC), 0x7F, 1> {};
	template<> struct Port_trait<Port::PORT_D>: Port_trait_impl<R_(PIND), R_(DDRD), R_(PORTD), 0xFF, 2> {};
	
	/**
	 * Digital pin symbols
	 */
	template<> struct DigitalPin_trait<DigitalPin::NONE>: DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D0_PD0>: DigitalPin_trait_impl<Port::PORT_D, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1_PD1>: DigitalPin_trait_impl<Port::PORT_D, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D2_PD2>: DigitalPin_trait_impl<Port::PORT_D, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D3_PD3>: DigitalPin_trait_impl<Port::PORT_D, 3, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D4_PD4>: DigitalPin_trait_impl<Port::PORT_D, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D5_PD5>: DigitalPin_trait_impl<Port::PORT_D, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D6_PD6>: DigitalPin_trait_impl<Port::PORT_D, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D7_PD7>: DigitalPin_trait_impl<Port::PORT_D, 7> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D8_PB0>: DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D9_PB1>: DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D10_PB2>: DigitalPin_trait_impl<Port::PORT_B, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D11_PB3>: DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D12_PB4>: DigitalPin_trait_impl<Port::PORT_B, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D13_PB5>: DigitalPin_trait_impl<Port::PORT_B, 5> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D14_PC0>: DigitalPin_trait_impl<Port::PORT_C, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D15_PC1>: DigitalPin_trait_impl<Port::PORT_C, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D16_PC2>: DigitalPin_trait_impl<Port::PORT_C, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D17_PC3>: DigitalPin_trait_impl<Port::PORT_C, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D18_PC4>: DigitalPin_trait_impl<Port::PORT_C, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D19_PC5>: DigitalPin_trait_impl<Port::PORT_C, 5> {};
	
	//==============
	// Analog Input
	//==============
	template<> struct AnalogReference_trait<AnalogReference::AREF>:AnalogReference_trait_impl<0> {};
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<_BV(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_1_1V>:AnalogReference_trait_impl<_BV(REFS1) | _BV(REFS0)> {};

	template<> struct AnalogSampleType_trait<uint16_t>: AnalogSampleType_trait_impl<uint16_t, 0, 0, R_(ADC)> {};
	template<> struct AnalogSampleType_trait<uint8_t>: AnalogSampleType_trait_impl<uint8_t, _BV(ADLAR), 0, R_(ADCH)> {};

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
	template<> struct AnalogPin_trait<AnalogPin::TEMP>: AnalogPin_trait_impl<_BV(MUX3)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<_BV(MUX3) | _BV(MUX2) | _BV(MUX1), 0, 1100> {};
	
	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D2_PD2_EXT0>: 
		ExternalInterruptPin_trait_impl<0, R_(EICRA), _BV(ISC00) | _BV(ISC01), R_(EIMSK), _BV(INT0), R_(EIFR), _BV(INTF0)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D3_PD3_EXT1>: 
		ExternalInterruptPin_trait_impl<1, R_(EICRA), _BV(ISC10) | _BV(ISC11), R_(EIMSK), _BV(INT1), R_(EIFR), _BV(INTF1)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<Port::PORT_B, 0x3F, _BV(PCIE0), _BV(PCIF0), R_(PCICR), R_(PCIFR), R_(PCMSK0)> {};
	template<> struct PCI_trait<1>: 
		PCI_trait_impl<Port::PORT_C, 0x3F, _BV(PCIE1), _BV(PCIF1), R_(PCICR), R_(PCIFR), R_(PCMSK1)> {};
	template<> struct PCI_trait<2>: 
		PCI_trait_impl<Port::PORT_D, 0xFF, _BV(PCIE2), _BV(PCIF2), R_(PCICR), R_(PCIFR), R_(PCMSK2)> {};

	//=======
	// USART
	//=======
	template<> struct USART_trait<USART::USART0>: 
		USART_trait_impl<R_(UCSR0A), R_(UCSR0B), R_(UCSR0C), R_(UDR0), R_(UBRR0), U2X0, TXEN0, RXEN0, UDRIE0, RXCIE0> 
	{
		static constexpr uint8_t UCSRC_value(serial::Parity parity, serial::StopBits stopbits)
		{
			return	(	parity == serial::Parity::EVEN ? _BV(UPM00) : 
						parity == serial::Parity::ODD ? _BV(UPM00) | _BV(UPM01) : 0x00)
					|	(stopbits == serial::StopBits::ONE ? 0x00 : _BV(USBS0))
					|	_BV(UCSZ00) | _BV(UCSZ01);
		}
	};
	
	//=====
	// SPI
	//=====
	struct SPI_trait: SPI_trait_impl<Port::PORT_B, PB2, PB3, PB4, PB5> {};

	//========
	// Timers
	//========
	template<> struct Timer_COM_trait<Timer::TIMER0, 0>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D6_PD6_OC0A, R_(OCR0A), 
		_BV(COM0A0) | _BV(COM0A1), 0, _BV(COM0A0), _BV(COM0A1), _BV(COM0A0) | _BV(COM0A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER0, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D5_PD5_OC0B, R_(OCR0B), 
		_BV(COM0B0) | _BV(COM0B1), 0, _BV(COM0B0), _BV(COM0B1), _BV(COM0B0) | _BV(COM0B1)> {};
	template<> struct Timer_trait<Timer::TIMER0>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							_BV(WGM00) | _BV(WGM01), 0,
							_BV(WGM00), 0,
							_BV(WGM01), 0,
							R_(TCCR0A), R_(TCCR0B), R_(TCNT0), R_(OCR0A),
							R_(TIMSK0), R_(TIFR0)>
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
	
	template<> struct Timer_COM_trait<Timer::TIMER2, 0>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D11_PB3_OC2A, R_(OCR2A), 
		_BV(COM2A0) | _BV(COM2A1), 0, _BV(COM2A0), _BV(COM2A1), _BV(COM2A0) | _BV(COM2A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER2, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D3_PD3_OC2B, R_(OCR2B), 
		_BV(COM2B0) | _BV(COM2B1), 0, _BV(COM2B0), _BV(COM2B1), _BV(COM2B0) | _BV(COM2B1)> {};
	template<> struct Timer_trait<Timer::TIMER2>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_32_64_128_256_1024, 
							2,
							_BV(WGM20) | _BV(WGM21), 0,
							_BV(WGM20), 0,
							_BV(WGM21), 0,
							R_(TCCR2A), R_(TCCR2B), R_(TCNT2), R_(OCR2A),
							R_(TIMSK2), R_(TIFR2)>
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
	
	template<> struct Timer_COM_trait<Timer::TIMER1, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D9_PB1_OC1A, R_(OCR1A), 
		_BV(COM1A0) | _BV(COM1A1), 0, _BV(COM1A0), _BV(COM1A1), _BV(COM1A0) | _BV(COM1A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D10_PB2_OC1B, R_(OCR1B), 
		_BV(COM1B0) | _BV(COM1B1), 0, _BV(COM1B0), _BV(COM1B1), _BV(COM1B0) | _BV(COM1B1)> {};
	template<> struct Timer_trait<Timer::TIMER1>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							_BV(WGM10) | _BV(WGM11), _BV(WGM12),
							_BV(WGM10) | _BV(WGM11), 0,
							0, _BV(WGM12), 
							R_(TCCR1A), R_(TCCR1B), R_(TCNT1), R_(OCR1A), 
							R_(TIMSK1), R_(TIFR1)>
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
	
	template<> struct PWMPin_trait<PWMPin::D6_PD6_OC0A>: PWMPin_trait_impl<Timer::TIMER0, 0> {};
	template<> struct PWMPin_trait<PWMPin::D5_PD5_OC0B>: PWMPin_trait_impl<Timer::TIMER0, 1> {};
	template<> struct PWMPin_trait<PWMPin::D9_PB1_OC1A>: PWMPin_trait_impl<Timer::TIMER1, 0> {};
	template<> struct PWMPin_trait<PWMPin::D10_PB2_OC1B>: PWMPin_trait_impl<Timer::TIMER1, 1> {};
	template<> struct PWMPin_trait<PWMPin::D11_PB3_OC2A>: PWMPin_trait_impl<Timer::TIMER2, 0> {};
	template<> struct PWMPin_trait<PWMPin::D3_PD3_OC2B>: PWMPin_trait_impl<Timer::TIMER2, 1> {};
};

#endif /* BOARDS_UNO_TRAITS_HH */
