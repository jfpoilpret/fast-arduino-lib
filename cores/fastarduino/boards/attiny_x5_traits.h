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

#ifndef BOARDS_ATTINYX5_TRAITS_HH
#define BOARDS_ATTINYX5_TRAITS_HH

#include <avr/io.h>
#include "attiny_x5.h"
#include "common_traits.h"

// For an unknown reason, register WDTCSR for all AVR is named WDTCR for ATtinyX5
// Just define a macro to use the same name everywhere needed
#define WDTCSR WDTCR

namespace board_traits
{
	//====
	// IO
	//====
	template<> struct Port_trait<Port::PORT_B>: Port_trait_impl<R_(PINB), R_(DDRB), R_(PORTB), 0x1F, 0> {};

	/**
	 * Digital pin symbols
	 */
	template<> struct DigitalPin_trait<DigitalPin::NONE>: public DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D0_PB0>: public DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1_PB1>: public DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D2_PB2>: public DigitalPin_trait_impl<Port::PORT_B, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D3_PB3>: public DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D4_PB4>: public DigitalPin_trait_impl<Port::PORT_B, 4> {};

	//==============
	// Analog Input
	//==============
	template<> struct AnalogReference_trait<AnalogReference::AREF>:AnalogReference_trait_impl<_BV(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<0> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_1_1V>:AnalogReference_trait_impl<_BV(REFS1)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_2_56V>:AnalogReference_trait_impl<_BV(REFS2) | _BV(REFS1)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_2_56V_BYPASS_CAP>
		:AnalogReference_trait_impl<_BV(REFS2) | _BV(REFS1) | _BV(REFS0)> {};
	
	template<> struct AnalogSampleType_trait<uint16_t>: AnalogSampleType_trait_impl<uint16_t, 0, 0, R_(ADC)> {};
	template<> struct AnalogSampleType_trait<uint8_t>: AnalogSampleType_trait_impl<uint8_t, 0, _BV(ADLAR), R_(ADCH)> {};

	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_50KHz>: AnalogClock_trait_impl<50000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_100KHz>: AnalogClock_trait_impl<100000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_200KHz>: AnalogClock_trait_impl<200000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_500KHz>: AnalogClock_trait_impl<500000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_1MHz>: AnalogClock_trait_impl<1000000UL> {};

	struct GlobalAnalogPin_trait:GlobalAnalogPin_trait_impl<R_(ADMUX), R_(ADCSRA), R_(ADCSRB)> {};
	
	template<> struct AnalogPin_trait<AnalogPin::A1>: AnalogPin_trait_impl<_BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A2>: AnalogPin_trait_impl<_BV(MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A3>: AnalogPin_trait_impl<_BV(MUX1) | _BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::TEMP>: AnalogPin_trait_impl<_BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::GND>: AnalogPin_trait_impl<_BV(MUX3) | _BV(MUX2) | _BV(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<_BV(MUX3) | _BV(MUX2), 0, 1100> {};

	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D2_PB2_EXT0>: 
		ExternalInterruptPin_trait_impl<0, R_(MCUCR), _BV(ISC00) | _BV(ISC01), R_(GIMSK), _BV(INT0), R_(GIFR), _BV(INTF0)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	//	PCI0 = 0,			// D0-D7, PA0-7
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<Port::PORT_B, 0x1F, _BV(PCIE), _BV(PCIF), R_(GIMSK), R_(GIFR), R_(PCMSK)> {};
	
	//=======
	// USART
	//=======
	// No Hardware USART
	
	//=====
	// SPI
	//=====
	//TODO recheck MOSI/MISO (as this must be DO/DI instead on ATtiny MCU)
	struct SPI_trait: SPI_trait_impl<Port::PORT_B, 0, PB0, PB1, PB2> {};

	//=====
	// I2C
	//=====
	struct TWI_trait: TWI_trait_impl<Port::PORT_B, PB2, PB0> {};

	//========
	// Timers
	//========
	template<> struct Timer_COM_trait<Timer::TIMER0, 0>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D0_PB0_OC0A, R_(OCR0A), 
		_BV(COM0A0) | _BV(COM0A1), 0, _BV(COM0A0), _BV(COM0A1), _BV(COM0A0) | _BV(COM0A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER0, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D1_PB1_OC0B, R_(OCR0B), 
		_BV(COM0B0) | _BV(COM0B1), 0, _BV(COM0B0), _BV(COM0B1), _BV(COM0B0) | _BV(COM0B1)> {};
	template<> struct Timer_trait<Timer::TIMER0>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							_BV(WGM00) | _BV(WGM01), _BV(WGM02), _BV(CS00) | _BV(CS01) | _BV(CS02),
							_BV(WGM00) | _BV(WGM01), 0,
							_BV(WGM00), 0,
							_BV(WGM01), 0,
							R_(TCCR0A), R_(TCCR0B), R_(TCNT0), R_(OCR0A), 
							R_(TIMSK), R_(TIFR)>
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
	
	//TODO FROM HERE
	template<> struct Timer_trait<Timer::TIMER1>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_TO_16384, 
							0,
							0, _BV(CTC1), _BV(CS10) | _BV(CS11) | _BV(CS12) | _BV(CS13),
							0, 0,
							0, 0,
							0, _BV(CTC1),
							//TODO Use 0 instead of R_(TCCR1) for TCCRB_
							NO_REG, R_(TCCR1), R_(TCNT1), R_(OCR1A), 
							R_(TIMSK), R_(TIFR)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return uint8_t(p) + 1;
		}
		static constexpr uint8_t TIMSK_MASK(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? _BV(TOIE1) : 0)
				|	(i & OUTPUT_COMPARE_A ? _BV(OCIE1A) : 0)
				|	(i & OUTPUT_COMPARE_B ? _BV(OCIE1B) : 0);
		}
	};
	
	template<> struct PWMPin_trait<PWMPin::D0_PB0_OC0A>: PWMPin_trait_impl<Timer::TIMER0, 0> {};
	template<> struct PWMPin_trait<PWMPin::D1_PB1_OC0B>: PWMPin_trait_impl<Timer::TIMER0, 1> {};
};

#endif /* BOARDS_ATTINYX5_TRAITS_HH */
