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

#ifndef BOARDS_LEONARDO_TRAITS_HH
#define BOARDS_LEONARDO_TRAITS_HH

#include "io.h"
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
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<BV8(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_2_56V>:AnalogReference_trait_impl<BV8(REFS1) | BV8(REFS0)> {};

	template<> struct AnalogSampleType_trait<uint16_t>: AnalogSampleType_trait_impl<uint16_t, 0, 0, R_(ADC)> {};
	template<> struct AnalogSampleType_trait<uint8_t>: AnalogSampleType_trait_impl<uint8_t, BV8(ADLAR), 0, R_(ADCH)> {};

	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_50KHz>: AnalogClock_trait_impl<50000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_100KHz>: AnalogClock_trait_impl<100000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_200KHz>: AnalogClock_trait_impl<200000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_500KHz>: AnalogClock_trait_impl<500000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_1MHz>: AnalogClock_trait_impl<1000000UL> {};

	struct GlobalAnalogPin_trait:GlobalAnalogPin_trait_impl<R_(ADMUX), R_(ADCSRA), R_(ADCSRB)> {};
	
	template<> struct AnalogPin_trait<AnalogPin::A5_ADC0>: AnalogPin_trait_impl<0> {};
	template<> struct AnalogPin_trait<AnalogPin::A4_ADC1>: AnalogPin_trait_impl<BV8(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A3_ADC4>: AnalogPin_trait_impl<BV8(MUX2)> {};
	template<> struct AnalogPin_trait<AnalogPin::A2_ADC5>: AnalogPin_trait_impl<BV8(MUX2) | BV8(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A1_ADC6>: AnalogPin_trait_impl<BV8(MUX2) | BV8(MUX1)> {};
	template<> struct AnalogPin_trait<AnalogPin::A0_ADC7>: AnalogPin_trait_impl<BV8(MUX2) | BV8(MUX1) | BV8(MUX0)> {};
	template<> struct AnalogPin_trait<AnalogPin::A6_D4_ADC8>: AnalogPin_trait_impl<0, BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A11_D12_ADC9>: AnalogPin_trait_impl<BV8(MUX0), BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A7_D6_ADC10>: AnalogPin_trait_impl<BV8(MUX1), BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A8_D8_ADC11>: AnalogPin_trait_impl<BV8(MUX1) | BV8(MUX0), BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A9_D9_ADC12>: AnalogPin_trait_impl<BV8(MUX2), BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::A10_D10_ADC13>: AnalogPin_trait_impl<BV8(MUX2) | BV8(MUX0), BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::TEMP>: AnalogPin_trait_impl<BV8(MUX2) | BV8(MUX1) | BV8(MUX0), BV8(MUX5)> {};
	template<> struct AnalogPin_trait<AnalogPin::BANDGAP>: AnalogPin_trait_impl<BV8(MUX4) | BV8(MUX3) | BV8(MUX2) | BV8(MUX1), 0, 1100> {};
	
	//===============
	// IO interrupts
	//===============
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D3_PD0_EXT0>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D3_PD0, 0, R_(EICRA), BV8(ISC00) | BV8(ISC01), R_(EIMSK), BV8(INT0), R_(EIFR), BV8(INTF0)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D2_PD1_EXT1>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D2_PD1, 1, R_(EICRA), BV8(ISC10) | BV8(ISC11), R_(EIMSK), BV8(INT1), R_(EIFR), BV8(INTF1)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D0_PD2_EXT2>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D0_PD2, 2, R_(EICRA), BV8(ISC20) | BV8(ISC21), R_(EIMSK), BV8(INT2), R_(EIFR), BV8(INTF2)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D1_PD3_EXT3>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D1_PD3, 3, R_(EICRA), BV8(ISC30) | BV8(ISC31), R_(EIMSK), BV8(INT3), R_(EIFR), BV8(INTF3)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::D7_PE6_EXT6>: 
		ExternalInterruptPin_trait_impl<DigitalPin::D7_PE6, 6, R_(EICRB), BV8(ISC60) | BV8(ISC61), R_(EIMSK), BV8(INT6), R_(EIFR), BV8(INTF6)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<> struct PCI_trait<0>: 
		PCI_trait_impl<Port::PORT_B, 0xFF, BV8(PCIE0), BV8(PCIF0), R_(PCICR), R_(PCIFR), R_(PCMSK0)> {};

	//=======
	// USART
	//=======
	template<> struct USART_trait<USART::USART1>: 
		USART_trait_impl<	R_(UCSR1A), R_(UCSR1B), R_(UCSR1C), R_(UDR1), R_(UBRR1), 
							U2X1, TXEN1, RXEN1, UDRIE1, RXCIE1, DOR1, FE1, UPE1> 
	{
		static constexpr uint8_t UCSRC_value(serial::Parity parity, serial::StopBits stopbits)
		{
			return	(	parity == serial::Parity::EVEN ? BV8(UPM10) : 
						parity == serial::Parity::ODD ? BV8(UPM10) | BV8(UPM11) : 0x00)
					|	(stopbits == serial::StopBits::ONE ? 0x00 : BV8(USBS1))
					|	BV8(UCSZ10) | BV8(UCSZ11);
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
		BV8(COM0A0) | BV8(COM0A1), 0, BV8(COM0A0), BV8(COM0A1), BV8(COM0A0) | BV8(COM0A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER0, 1>: Timer_COM_trait_impl<
		uint8_t, PWMPin::D3_PD0_OC0B, R_(OCR0B), 
		BV8(COM0B0) | BV8(COM0B1), 0, BV8(COM0B0), BV8(COM0B1), BV8(COM0B0) | BV8(COM0B1)> {};
	template<> struct Timer_trait<Timer::TIMER0>: 
		Timer_trait_impl<	uint8_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							2,
							BV8(WGM00) | BV8(WGM01), BV8(WGM02), BV8(CS00) | BV8(CS01) | BV8(CS02),
							BV8(WGM00) | BV8(WGM01), 0,
							BV8(WGM00), 0,
							BV8(WGM01), 0,
							R_(TCCR0A), R_(TCCR0B), R_(TCNT0), R_(OCR0A),
							R_(TIMSK0), R_(TIFR0)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? BV8(CS00) :
					p == TIMER_PRESCALER::DIV_8 ? BV8(CS01) :
					p == TIMER_PRESCALER::DIV_64 ? BV8(CS00) | BV8(CS01) :
					p == TIMER_PRESCALER::DIV_256 ? BV8(CS02) :
					BV8(CS02) | BV8(CS00));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? BV8(TOIE0) : 0)
				|	(i & OUTPUT_COMPARE_A ? BV8(OCIE0A) : 0)
				|	(i & OUTPUT_COMPARE_B ? BV8(OCIE0B) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER1, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D9_PB5_OC1A, R_(OCR1A), 
		BV8(COM1A0) | BV8(COM1A1), 0, BV8(COM1A0), BV8(COM1A1), BV8(COM1A0) | BV8(COM1A1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 1>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D10_PB6_OC1B, R_(OCR1B), 
		BV8(COM1B0) | BV8(COM1B1), 0, BV8(COM1B0), BV8(COM1B1), BV8(COM1B0) | BV8(COM1B1)> {};
	template<> struct Timer_COM_trait<Timer::TIMER1, 2>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D11_PB7_OC1C, R_(OCR1C), 
		BV8(COM1C0) | BV8(COM1C1), 0, BV8(COM1C0), BV8(COM1C1), BV8(COM1C0) | BV8(COM1C1)> {};
	template<> struct Timer_trait<Timer::TIMER1>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							3,
							BV8(WGM10) | BV8(WGM11), BV8(WGM12) | BV8(WGM13), BV8(CS10) | BV8(CS11) | BV8(CS12),
							BV8(WGM10) | BV8(WGM11), BV8(WGM12),
							BV8(WGM10) | BV8(WGM11), 0,
							0, BV8(WGM12), 
							R_(TCCR1A), R_(TCCR1B), R_(TCNT1), R_(OCR1A), 
							R_(TIMSK1), R_(TIFR1), 0xFF,
							R_(ICR1),
							0, BV8(WGM12) | BV8(WGM13),
							BV8(WGM11), BV8(WGM12) | BV8(WGM13),
							BV8(WGM11), BV8(WGM13),
							board::DigitalPin::D4_PD4, BV8(ICES1)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? BV8(CS10) :
					p == TIMER_PRESCALER::DIV_8 ? BV8(CS11) :
					p == TIMER_PRESCALER::DIV_64 ? BV8(CS10) | BV8(CS11) :
					p == TIMER_PRESCALER::DIV_256 ? BV8(CS12) :
					BV8(CS12) | BV8(CS10));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? BV8(TOIE1) : 0)
				|	(i & OUTPUT_COMPARE_A ? BV8(OCIE1A) : 0)
				|	(i & OUTPUT_COMPARE_B ? BV8(OCIE1B) : 0)
				|	(i & INPUT_CAPTURE ? BV8(ICIE1) : 0);
		}
	};
	
	template<> struct Timer_COM_trait<Timer::TIMER3, 0>: Timer_COM_trait_impl<
		uint16_t, PWMPin::D5_PC6_OC3A, R_(OCR3A), 
		BV8(COM3A0) | BV8(COM3A1), 0, BV8(COM3A0), BV8(COM3A1), BV8(COM3A0) | BV8(COM3A1)> {};
	template<> struct Timer_trait<Timer::TIMER3>: 
		Timer_trait_impl<	uint16_t, TimerPrescalers::PRESCALERS_1_8_64_256_1024, 
							1,
							BV8(WGM30) | BV8(WGM31), BV8(WGM32) | BV8(WGM33), BV8(CS30) | BV8(CS31) | BV8(CS32),
							BV8(WGM30) | BV8(WGM31), BV8(WGM32),
							BV8(WGM30) | BV8(WGM31), 0,
							0, BV8(WGM32), 
							R_(TCCR3A), R_(TCCR3B), R_(TCNT3), R_(OCR3A), 
							R_(TIMSK3), R_(TIFR3), 0xFF,
							R_(ICR3),
							0, BV8(WGM32) | BV8(WGM33),
							BV8(WGM31), BV8(WGM32) | BV8(WGM33),
							BV8(WGM31), BV8(WGM33),
							board::DigitalPin::D13_PC7, BV8(ICES3)>
	{
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? BV8(CS30) :
					p == TIMER_PRESCALER::DIV_8 ? BV8(CS31) :
					p == TIMER_PRESCALER::DIV_64 ? BV8(CS30) | BV8(CS31) :
					p == TIMER_PRESCALER::DIV_256 ? BV8(CS32) :
					BV8(CS32) | BV8(CS30));
		}
		static constexpr uint8_t TIMSK_int_mask(uint8_t i)
		{
			using namespace board_traits::TimerInterrupt;
			return	(i & OVERFLOW ? BV8(TOIE3) : 0)
				|	(i & OUTPUT_COMPARE_A ? BV8(OCIE3A) : 0)
				|	(i & OUTPUT_COMPARE_B ? BV8(OCIE3B) : 0)
				|	(i & INPUT_CAPTURE ? BV8(ICIE3) : 0);
		}
	};
	
	template<> struct PWMPin_trait<PWMPin::D11_PB7_OC0A> : PWMPin_trait_impl<DigitalPin::D11_PB7, Timer::TIMER0, 0> {};
	template<> struct PWMPin_trait<PWMPin::D3_PD0_OC0B> : PWMPin_trait_impl<DigitalPin::D3_PD0, Timer::TIMER0, 1> {};
	template<> struct PWMPin_trait<PWMPin::D9_PB5_OC1A> : PWMPin_trait_impl<DigitalPin::D9_PB5, Timer::TIMER1, 0> {};
	template<> struct PWMPin_trait<PWMPin::D10_PB6_OC1B> : PWMPin_trait_impl<DigitalPin::D10_PB6, Timer::TIMER1, 1> {};
	template<> struct PWMPin_trait<PWMPin::D11_PB7_OC1C> : PWMPin_trait_impl<DigitalPin::D11_PB7, Timer::TIMER1, 2> {};
	template<> struct PWMPin_trait<PWMPin::D5_PC6_OC3A> : PWMPin_trait_impl<DigitalPin::D5_PC6, Timer::TIMER3, 0> {};
};

// Macros to declare some ISR friends
#define DECL_INT_ISR_FRIENDS 		\
	friend void ::INT0_vect(void);	\
	friend void ::INT1_vect(void);	\
	friend void ::INT2_vect(void);	\
	friend void ::INT3_vect(void);	\
	friend void ::INT6_vect(void);
#define DECL_PCINT_ISR_FRIENDS friend void ::PCINT0_vect(void);
#define DECL_TIMER_COMP_FRIENDS				\
	friend void ::TIMER0_COMPA_vect(void);	\
	friend void ::TIMER1_COMPA_vect(void);	\
	friend void ::TIMER3_COMPA_vect(void);	\
	friend void ::TIMER4_COMPA_vect(void);	\
	friend void ::TIMER0_COMPB_vect(void);	\
	friend void ::TIMER1_COMPB_vect(void);	\
	friend void ::TIMER3_COMPB_vect(void);	\
	friend void ::TIMER4_COMPB_vect(void);	\
	friend void ::TIMER1_COMPC_vect(void);	\
	friend void ::TIMER3_COMPC_vect(void);	\
	friend void ::TIMER4_COMPD_vect(void);
#define DECL_TIMER_OVF_FRIENDS			\
	friend void ::TIMER0_OVF_vect(void);\
	friend void ::TIMER1_OVF_vect(void);\
	friend void ::TIMER3_OVF_vect(void);\
	friend void ::TIMER4_OVF_vect(void);
#define DECL_TIMER_CAPT_FRIENDS				\
	friend void ::TIMER1_CAPT_vect(void);	\
	friend void ::TIMER3_CAPT_vect(void);
#define DECL_UDRE_ISR_FRIENDS friend void ::USART1_UDRE_vect(void);
#define DECL_RX_ISR_FRIENDS friend void ::USART1_RX_vect(void);

#endif /* BOARDS_LEONARDO_TRAITS_HH */
