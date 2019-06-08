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

#ifndef BOARDS_ATTINYX4_TRAITS_HH
#define BOARDS_ATTINYX4_TRAITS_HH

#include "io.h"
#include "attiny_x4.h"
#include "common_traits.h"

namespace board_traits
{
	//====
	// IO
	//====
	template<> struct Port_trait<Port::PORT_A>: Port_trait_impl<R_(PINA), R_(DDRA), R_(PORTA), 0xFF, 0> {};
	template<> struct Port_trait<Port::PORT_B>: Port_trait_impl<R_(PINB), R_(DDRB), R_(PORTB), 0x07, 1> {};

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

	//==============
	// Analog Input
	//==============
	template<> struct AnalogReference_trait<AnalogReference::AREF>:AnalogReference_trait_impl<_BV(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<0> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_1_1V>:AnalogReference_trait_impl<_BV(REFS1)> {};
	
	template<> struct AnalogSampleType_trait<uint16_t>: AnalogSampleType_trait_impl<uint16_t, 0, 0, R_(ADC)> {};
	template<> struct AnalogSampleType_trait<uint8_t>: AnalogSampleType_trait_impl<uint8_t, 0, _BV(ADLAR), R_(ADCH)> {};

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
	template<> struct AnalogPin_trait<AnalogPin::TEMP>: AnalogPin_trait_impl<_BV(MUX5) | _BV(MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<_BV(MUX5) | _BV(MUX0), 0, 1100> {};

	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D10_PB2_EXT0>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D10_PB2, 0, R_(MCUCR), _BV(ISC00) | _BV(ISC01), R_(GIMSK), _BV(INT0), R_(GIFR), _BV(INTF0)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	//	PCI0 = 0,			// D0-D7, PA0-7
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<Port::PORT_A, 0xFF, _BV(PCIE0), _BV(PCIF0), R_(GIMSK), R_(GIFR), R_(PCMSK0)> {};
	//	PCI1 = 1			// D8-D10, PB0-2 (PB3 used for RESET)
	template<> struct PCI_trait<1>: 
		PCI_trait_impl<Port::PORT_B, 0x07, _BV(PCIE1), _BV(PCIF1), R_(GIMSK), R_(GIFR), R_(PCMSK1)> {};
	
	//=======
	// USART
	//=======
	// No Hardware USART
	
	//=====
	// SPI
	//=====
	struct SPI_trait: SPI_trait_impl<Port::PORT_A, 0, PA5, PA6, PA4> {};

	//=====
	// I2C
	//=====
	struct TWI_trait: TWI_trait_impl<Port::PORT_A, PA4, PA6> {};

	//========
	// Timers
	//========
	template<> struct Timer_COM_trait<Timer::TIMER0, 0>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D10_PB2_OC0A, R_(OCR0A), 
		_BV(COM0A0) | _BV(COM0A1), 0, _BV(COM0A0), _BV(COM0A1), _BV(COM0A0) | _BV(COM0A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER0, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D7_PA7_OC0B, R_(OCR0B), 
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
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? _BV(TOIE0) : 0)
				|	(i & OUTPUT_COMPARE_A ? _BV(OCIE0A) : 0)
				|	(i & OUTPUT_COMPARE_B ? _BV(OCIE0B) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER1, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D6_PA6_OC1A, R_(OCR1A), 
		_BV(COM1A0) | _BV(COM1A1), 0, _BV(COM1A0), _BV(COM1A1), _BV(COM1A0) | _BV(COM1A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D5_PA5_OC1B, R_(OCR1B), 
		_BV(COM1B0) | _BV(COM1B1), 0, _BV(COM1B0), _BV(COM1B1), _BV(COM1B0) | _BV(COM1B1)> {};
	template<> struct Timer_trait<Timer::TIMER1>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							_BV(WGM10) | _BV(WGM11), _BV(WGM12) | _BV(WGM13), _BV(CS10) | _BV(CS11) | _BV(CS12),
							_BV(WGM10) | _BV(WGM11), _BV(WGM12),
							_BV(WGM10) | _BV(WGM11), 0,
							0, _BV(WGM12), 
							R_(TCCR1A), R_(TCCR1B), R_(TCNT1), R_(OCR1A), 
							R_(TIMSK1), R_(TIFR1), 0xFF,
							R_(ICR1),
							0, _BV(WGM12) | _BV(WGM13),
							_BV(WGM11), _BV(WGM12) | _BV(WGM13),
							_BV(WGM11), _BV(WGM13),
							board::DigitalPin::D7_PA7, _BV(ICES1)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS10) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS11) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS10) | _BV(CS11) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS12) :
					_BV(CS12) | _BV(CS10));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? _BV(TOIE1) : 0)
				|	(i & OUTPUT_COMPARE_A ? _BV(OCIE1A) : 0)
				|	(i & OUTPUT_COMPARE_B ? _BV(OCIE1B) : 0)
				|	(i & INPUT_CAPTURE ? _BV(ICIE1) : 0);
		}
	};
	
	template<> struct PWMPin_trait<PWMPin::D10_PB2_OC0A> : PWMPin_trait_impl<DigitalPin::D10_PB2, Timer::TIMER0, 0> {};
	template<> struct PWMPin_trait<PWMPin::D7_PA7_OC0B> : PWMPin_trait_impl<DigitalPin::D7_PA7, Timer::TIMER0, 1> {};
	template<> struct PWMPin_trait<PWMPin::D6_PA6_OC1A> : PWMPin_trait_impl<DigitalPin::D6_PA6, Timer::TIMER1, 0> {};
	template<> struct PWMPin_trait<PWMPin::D5_PA5_OC1B> : PWMPin_trait_impl<DigitalPin::D5_PA5, Timer::TIMER1, 1> {};
};

// Macros to declare some ISR friends
#define DECL_INT_ISR_FRIENDS friend void ::INT0_vect(void);
#define DECL_PCINT_ISR_FRIENDS		\
	friend void ::PCINT0_vect(void);\
	friend void ::PCINT1_vect(void);
#define DECL_TIMER_COMP_FRIENDS				\
	friend void ::TIMER0_COMPA_vect(void);	\
	friend void ::TIMER1_COMPA_vect(void);	\
	friend void ::TIMER0_COMPB_vect(void);	\
	friend void ::TIMER1_COMPB_vect(void);
#define DECL_TIMER_OVF_FRIENDS			\
	friend void ::TIMER0_OVF_vect(void);\
	friend void ::TIMER1_OVF_vect(void);
#define DECL_TIMER_CAPT_FRIENDS friend void ::TIMER1_CAPT_vect(void);
#define DECL_UDRE_ISR_FRIENDS 
#define DECL_RX_ISR_FRIENDS 

#endif /* BOARDS_ATTINYX4_TRAITS_HH */
