//   Copyright 2016-2018 Jean-Francois Poilpret
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

#ifndef BOARDS_LEONARDO_TRAITS_HH
#define BOARDS_LEONARDO_TRAITS_HH

#include <avr/io.h>
#include "leonardo.h"
#include "common_traits.h"

namespace board_traits
{
	//====
	// IO
	//====
	template<> struct Port_trait<Port::PORT_B>: Port_trait_impl<R_(PINB), R_(DDRB), R_(PORTB), 0xFF, 0> {};
	template<> struct Port_trait<Port::PORT_C>: Port_trait_impl<R_(PINC), R_(DDRC), R_(PORTC), 0xC0> {};
	template<> struct Port_trait<Port::PORT_D>: Port_trait_impl<R_(PIND), R_(DDRD), R_(PORTD), 0xFF> {};
	template<> struct Port_trait<Port::PORT_E>: Port_trait_impl<R_(PINE), R_(DDRE), R_(PORTE), 0x40> {};
	template<> struct Port_trait<Port::PORT_F>: Port_trait_impl<R_(PINF), R_(DDRF), R_(PORTF), 0xF3> {};
	
	/**
	 * Digital pin symbols
	 */
	template<> struct DigitalPin_trait<DigitalPin::NONE>: DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::RXLED_PB0>: DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::SCK_PB1>: DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::MOSI_PB2>: DigitalPin_trait_impl<Port::PORT_B, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::MISO_PB3>: DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D8_PB4>: DigitalPin_trait_impl<Port::PORT_B, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D9_PB5>: DigitalPin_trait_impl<Port::PORT_B, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D10_PB6>: DigitalPin_trait_impl<Port::PORT_B, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D11_PB7>: DigitalPin_trait_impl<Port::PORT_B, 7> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D5_PC6>: DigitalPin_trait_impl<Port::PORT_C, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D13_PC7>: DigitalPin_trait_impl<Port::PORT_C, 7> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D3_PD0>: DigitalPin_trait_impl<Port::PORT_D, 0, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D2_PD1>: DigitalPin_trait_impl<Port::PORT_D, 1, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D0_PD2>: DigitalPin_trait_impl<Port::PORT_D, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D1_PD3>: DigitalPin_trait_impl<Port::PORT_D, 3, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D4_PD4>: DigitalPin_trait_impl<Port::PORT_D, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::TXLED_PD5>: DigitalPin_trait_impl<Port::PORT_D, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D12_PD6>: DigitalPin_trait_impl<Port::PORT_D, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D6_PD7>: DigitalPin_trait_impl<Port::PORT_D, 7> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D7_PE6>: DigitalPin_trait_impl<Port::PORT_E, 6, true> {};
	
	template<> struct DigitalPin_trait<DigitalPin::A5_PF0>: DigitalPin_trait_impl<Port::PORT_F, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::A4_PF1>: DigitalPin_trait_impl<Port::PORT_F, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::A3_PF4>: DigitalPin_trait_impl<Port::PORT_F, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::A2_PF5>: DigitalPin_trait_impl<Port::PORT_F, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::A1_PF6>: DigitalPin_trait_impl<Port::PORT_F, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::A0_PF7>: DigitalPin_trait_impl<Port::PORT_F, 7> {};
	
	//==============
	// Analog Input
	//==============
	template<> struct AnalogReference_trait<AnalogReference::AREF>:AnalogReference_trait_impl<0> {};
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<_BV(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_2_56V>:AnalogReference_trait_impl<_BV(REFS1) | _BV(REFS0)> {};

	template<> struct AnalogSampleType_trait<uint16_t>: AnalogSampleType_trait_impl<uint16_t, 0, 0, R_(ADC)> {};
	template<> struct AnalogSampleType_trait<uint8_t>: AnalogSampleType_trait_impl<uint8_t, _BV(ADLAR), 0, R_(ADCH)> {};

	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_50KHz>: AnalogClock_trait_impl<50000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_100KHz>: AnalogClock_trait_impl<100000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_200KHz>: AnalogClock_trait_impl<200000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_500KHz>: AnalogClock_trait_impl<500000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_1MHz>: AnalogClock_trait_impl<1000000UL> {};

	struct GlobalAnalogPin_trait:GlobalAnalogPin_trait_impl<R_(ADMUX), R_(ADCSRA), R_(ADCSRB)> {};
	
	template<> struct AnalogPin_trait<AnalogPin::A5_ADC0>: AnalogPin_trait_impl<0> {};
	template<> struct AnalogPin_trait<AnalogPin::A4_ADC1>: AnalogPin_trait_impl<_BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A3_ADC4>: AnalogPin_trait_impl<_BV(MUX2)> {};
	template<> struct AnalogPin_trait<AnalogPin::A2_ADC5>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A1_ADC6>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A0_ADC7>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX1) | _BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A6_D4_ADC8>: AnalogPin_trait_impl<0, _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A11_D12_ADC9>: AnalogPin_trait_impl<_BV(MUX0), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A7_D6_ADC10>: AnalogPin_trait_impl<_BV(MUX1), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A8_D8_ADC11>: AnalogPin_trait_impl<_BV(MUX1) | _BV(MUX0), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A9_D9_ADC12>: AnalogPin_trait_impl<_BV(MUX2), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A10_D10_ADC13>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX0), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::TEMP>: AnalogPin_trait_impl<_BV(MUX2) | _BV(MUX1) | _BV(MUX0), _BV(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<_BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1), 0, 1100> {};
	
	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D3_PD0_EXT0>: 
		ExternalInterruptPin_trait_impl<0, R_(EICRA), _BV(ISC00) | _BV(ISC01), R_(EIMSK), _BV(INT0), R_(EIFR), _BV(INTF0)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D2_PD1_EXT1>: 
		ExternalInterruptPin_trait_impl<1, R_(EICRA), _BV(ISC10) | _BV(ISC11), R_(EIMSK), _BV(INT1), R_(EIFR), _BV(INTF1)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D0_PD2_EXT2>: 
		ExternalInterruptPin_trait_impl<2, R_(EICRA), _BV(ISC20) | _BV(ISC21), R_(EIMSK), _BV(INT2), R_(EIFR), _BV(INTF2)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D1_PD3_EXT3>: 
		ExternalInterruptPin_trait_impl<3, R_(EICRA), _BV(ISC30) | _BV(ISC31), R_(EIMSK), _BV(INT3), R_(EIFR), _BV(INTF3)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D7_PE6_EXT6>: 
		ExternalInterruptPin_trait_impl<6, R_(EICRB), _BV(ISC60) | _BV(ISC61), R_(EIMSK), _BV(INT6), R_(EIFR), _BV(INTF6)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<Port::PORT_B, 0xFF, _BV(PCIE0), _BV(PCIF0), R_(PCICR), R_(PCIFR), R_(PCMSK0)> {};

	//=======
	// USART
	//=======
	template<> struct USART_trait<USART::USART1>: 
		USART_trait_impl<	R_(UCSR1A), R_(UCSR1B), R_(UCSR1C), R_(UDR1), R_(UBRR1), 
							U2X1, TXEN1, RXEN1, UDRIE1, RXCIE1, DOR1, FE1, UPE1> 
	{
		static constexpr uint8_t UCSRC_value(serial::Parity parity, serial::StopBits stopbits)
		{
			return	(	parity == serial::Parity::EVEN ? _BV(UPM10) : 
						parity == serial::Parity::ODD ? _BV(UPM10) | _BV(UPM11) : 0x00)
					|	(stopbits == serial::StopBits::ONE ? 0x00 : _BV(USBS1))
					|	_BV(UCSZ10) | _BV(UCSZ11);
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
		uint8_t, PWMPin::D11_PB7_OC0A, R_(OCR0A), 
		_BV(COM0A0) | _BV(COM0A1), 0, _BV(COM0A0), _BV(COM0A1), _BV(COM0A0) | _BV(COM0A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER0, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D3_PD0_OC0B, R_(OCR0B), 
		_BV(COM0B0) | _BV(COM0B1), 0, _BV(COM0B0), _BV(COM0B1), _BV(COM0B0) | _BV(COM0B1)> {};
	template<> struct Timer_trait<Timer::TIMER0>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							_BV(WGM00) | _BV(WGM01), _BV(WGM02), _BV(CS00) | _BV(CS01) | _BV(CS02),
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
		static constexpr uint8_t TIMSK_MASK(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? _BV(TOIE0) : 0)
				|	(i & OUTPUT_COMPARE_A ? _BV(OCIE0A) : 0)
				|	(i & OUTPUT_COMPARE_B ? _BV(OCIE0B) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER1, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D9_PB5_OC1A, R_(OCR1A), 
		_BV(COM1A0) | _BV(COM1A1), 0, _BV(COM1A0), _BV(COM1A1), _BV(COM1A0) | _BV(COM1A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D10_PB6_OC1B, R_(OCR1B), 
		_BV(COM1B0) | _BV(COM1B1), 0, _BV(COM1B0), _BV(COM1B1), _BV(COM1B0) | _BV(COM1B1)> {};
	template<> struct Timer_trait<Timer::TIMER1>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							_BV(WGM10) | _BV(WGM11), _BV(WGM12) | _BV(WGM13), _BV(CS10) | _BV(CS11) | _BV(CS12),
							_BV(WGM10) | _BV(WGM11), _BV(WGM12),
							_BV(WGM10) | _BV(WGM11), 0,
							0, _BV(WGM12), 
							R_(TCCR1A), R_(TCCR1B), R_(TCNT1), R_(OCR1A), 
							R_(TIMSK1), R_(TIFR1),
							R_(ICR1),
							0, _BV(WGM12) | _BV(WGM13),
							_BV(WGM11), _BV(WGM12) | _BV(WGM13),
							_BV(WGM11), _BV(WGM13),
							board::DigitalPin::D4_PD4, _BV(ICES1)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS10) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS11) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS10) | _BV(CS11) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS12) :
					_BV(CS12) | _BV(CS10));
		}
		static constexpr uint8_t TIMSK_MASK(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? _BV(TOIE1) : 0)
				|	(i & OUTPUT_COMPARE_A ? _BV(OCIE1A) : 0)
				|	(i & OUTPUT_COMPARE_B ? _BV(OCIE1B) : 0)
				|	(i & INPUT_CAPTURE ? _BV(ICIE1) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER3, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D5_PC6_OC3A, R_(OCR3A), 
		_BV(COM3A0) | _BV(COM3A1), 0, _BV(COM3A0), _BV(COM3A1), _BV(COM3A0) | _BV(COM3A1)> {};
	template<> struct Timer_trait<Timer::TIMER3>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							1,
							_BV(WGM30) | _BV(WGM31), _BV(WGM32) | _BV(WGM33), _BV(CS30) | _BV(CS31) | _BV(CS32),
							_BV(WGM30) | _BV(WGM31), _BV(WGM32),
							_BV(WGM30) | _BV(WGM31), 0,
							0, _BV(WGM32), 
							R_(TCCR3A), R_(TCCR3B), R_(TCNT3), R_(OCR3A), 
							R_(TIMSK3), R_(TIFR3),
							R_(ICR3),
							0, _BV(WGM32) | _BV(WGM33),
							_BV(WGM31), _BV(WGM32) | _BV(WGM33),
							_BV(WGM31), _BV(WGM33),
							board::DigitalPin::D13_PC7, _BV(ICES3)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS30) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS31) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS30) | _BV(CS31) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS32) :
					_BV(CS32) | _BV(CS30));
		}
		static constexpr uint8_t TIMSK_MASK(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? _BV(TOIE3) : 0)
				|	(i & OUTPUT_COMPARE_A ? _BV(OCIE3A) : 0)
				|	(i & OUTPUT_COMPARE_B ? _BV(OCIE3B) : 0)
				|	(i & INPUT_CAPTURE ? _BV(ICIE3) : 0);
		}
	};
	
	template<> struct PWMPin_trait<PWMPin::D11_PB7_OC0A>: PWMPin_trait_impl<Timer::TIMER0, 0> {};
	template<> struct PWMPin_trait<PWMPin::D3_PD0_OC0B>: PWMPin_trait_impl<Timer::TIMER0, 1> {};
	template<> struct PWMPin_trait<PWMPin::D9_PB5_OC1A>: PWMPin_trait_impl<Timer::TIMER1, 0> {};
	template<> struct PWMPin_trait<PWMPin::D10_PB6_OC1B>: PWMPin_trait_impl<Timer::TIMER1, 1> {};
	template<> struct PWMPin_trait<PWMPin::D5_PC6_OC3A>: PWMPin_trait_impl<Timer::TIMER3, 0> {};
};

#endif /* BOARDS_LEONARDO_TRAITS_HH */
