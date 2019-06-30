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

#ifndef BOARDS_UNO_TRAITS_HH
#define BOARDS_UNO_TRAITS_HH

#include "io.h"
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
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<bits::BV8(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_1_1V>:AnalogReference_trait_impl<bits::BV8(REFS1, REFS0)> {};

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
#ifdef HAS_8_ANALOG_INPUTS
	template<> struct AnalogPin_trait<AnalogPin::A6>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A7>: AnalogPin_trait_impl<bits::BV8(MUX2, MUX1, MUX0)> {};
#endif	
	template<> struct AnalogPin_trait<AnalogPin::TEMP>: AnalogPin_trait_impl<bits::BV8(MUX3)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<bits::BV8(MUX3, MUX2, MUX1), 0, 1100> {};
	
	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D2_PD2_EXT0>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D2_PD2, 0, R_(EICRA), bits::BV8(ISC00, ISC01), R_(EIMSK), bits::BV8(INT0), R_(EIFR), bits::BV8(INTF0)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D3_PD3_EXT1>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D3_PD3, 1, R_(EICRA), bits::BV8(ISC10, ISC11), R_(EIMSK), bits::BV8(INT1), R_(EIFR), bits::BV8(INTF1)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<Port::PORT_B, 0x3F, bits::BV8(PCIE0), bits::BV8(PCIF0), R_(PCICR), R_(PCIFR), R_(PCMSK0)> {};
	template<> struct PCI_trait<1>: 
		PCI_trait_impl<Port::PORT_C, 0x3F, bits::BV8(PCIE1), bits::BV8(PCIF1), R_(PCICR), R_(PCIFR), R_(PCMSK1)> {};
	template<> struct PCI_trait<2>: 
		PCI_trait_impl<Port::PORT_D, 0xFF, bits::BV8(PCIE2), bits::BV8(PCIF2), R_(PCICR), R_(PCIFR), R_(PCMSK2)> {};

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
	
	//=====
	// SPI
	//=====
	struct SPI_trait: SPI_trait_impl<Port::PORT_B, PB2, PB3, PB4, PB5> {};

	//=====
	// I2C
	//=====
	struct TWI_trait: TWI_trait_impl<Port::PORT_C, PC5, PC4> {};

	//========
	// Timers
	//========
	template<> struct Timer_COM_trait<Timer::TIMER0, 0>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D6_PD6_OC0A, R_(OCR0A), 
		bits::BV8(COM0A0, COM0A1), 0, bits::BV8(COM0A0), bits::BV8(COM0A1), bits::BV8(COM0A0, COM0A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER0, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D5_PD5_OC0B, R_(OCR0B), 
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
		uint8_t, PWMPin::D11_PB3_OC2A, R_(OCR2A), 
		bits::BV8(COM2A0, COM2A1), 0, bits::BV8(COM2A0), bits::BV8(COM2A1), bits::BV8(COM2A0, COM2A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER2, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D3_PD3_OC2B, R_(OCR2B), 
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
		uint16_t, PWMPin::D9_PB1_OC1A, R_(OCR1A), 
		bits::BV8(COM1A0, COM1A1), 0, bits::BV8(COM1A0), bits::BV8(COM1A1), bits::BV8(COM1A0, COM1A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D10_PB2_OC1B, R_(OCR1B), 
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
							board::DigitalPin::D8_PB0, bits::BV8(ICES1)>
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
	
	template<> struct PWMPin_trait<PWMPin::D6_PD6_OC0A> : PWMPin_trait_impl<DigitalPin::D6_PD6, Timer::TIMER0, 0> {};
	template<> struct PWMPin_trait<PWMPin::D5_PD5_OC0B> : PWMPin_trait_impl<DigitalPin::D5_PD5, Timer::TIMER0, 1> {};
	template<> struct PWMPin_trait<PWMPin::D9_PB1_OC1A> : PWMPin_trait_impl<DigitalPin::D9_PB1, Timer::TIMER1, 0> {};
	template<> struct PWMPin_trait<PWMPin::D10_PB2_OC1B> : PWMPin_trait_impl<DigitalPin::D10_PB2, Timer::TIMER1, 1> {};
	template<> struct PWMPin_trait<PWMPin::D11_PB3_OC2A> : PWMPin_trait_impl<DigitalPin::D11_PB3, Timer::TIMER2, 0> {};
	template<> struct PWMPin_trait<PWMPin::D3_PD3_OC2B> : PWMPin_trait_impl<DigitalPin::D3_PD3, Timer::TIMER2, 1> {};
};

// Macros to declare some ISR friends
#define DECL_INT_ISR_FRIENDS 		\
	friend void ::INT0_vect(void);	\
	friend void ::INT1_vect(void);
#define DECL_PCINT_ISR_FRIENDS		\
	friend void ::PCINT0_vect(void);\
	friend void ::PCINT1_vect(void);\
	friend void ::PCINT2_vect(void);
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
#define DECL_TIMER_CAPT_FRIENDS friend void ::TIMER1_CAPT_vect(void);
#define DECL_UDRE_ISR_FRIENDS friend void ::USART0_UDRE_vect(void);
#define DECL_RX_ISR_FRIENDS friend void ::USART0_RX_vect(void);

#endif /* BOARDS_UNO_TRAITS_HH */
