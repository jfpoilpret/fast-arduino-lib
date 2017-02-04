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

//TODO include common board traits utilities used in each XXX_traits header
// - template traits parents

#define R_(REG) ((uint16_t)&REG)

namespace Board
{
	using REG = uint16_t;

	//====
	// IO
	//====
	template<Port P>
	struct Port_trait
	{
		static constexpr const REGISTER PIN{};
		static constexpr const REGISTER DDR{};
		static constexpr const REGISTER PORT{};
		static constexpr const uint8_t DPIN_MASK = 0x00;
		static constexpr const uint8_t PCINT = 0;
	};
	
	template<REG PIN_, REG DDR_, REG PORT_, uint8_t DPIN_MASK_, uint8_t PCINT_>
	struct Port_trait_impl
	{
		static constexpr const REGISTER PIN = PIN_;
		static constexpr const REGISTER DDR = DDR_;
		static constexpr const REGISTER PORT = PORT_;
		static constexpr const uint8_t DPIN_MASK = DPIN_MASK_;
		static constexpr const uint8_t PCINT = PCINT_;
	};

	template<> struct Port_trait<Port::PORT_B>: Port_trait_impl<R_(PINB), R_(DDRB), R_(PORTB), 0xFF, 0> {};
	template<> struct Port_trait<Port::PORT_C>: Port_trait_impl<R_(PINC), R_(DDRC), R_(PORTC), 0x7F, 1> {};
	template<> struct Port_trait<Port::PORT_D>: Port_trait_impl<R_(PIND), R_(DDRD), R_(PORTD), 0xFF, 2> {};
	
	/**
	 * Digital pin symbols
	 */
	template<DigitalPin DPIN>
	struct DigitalPin_trait
	{
		static constexpr const Port PORT = Port::NONE;
		static constexpr const uint8_t BIT = 0;
		static constexpr const bool IS_INT = false;
	};
	template<Port P, uint8_t B, bool INT = false>
	struct DigitalPin_trait_impl
	{
		static constexpr const Port PORT = P;
		static constexpr const uint8_t BIT = B;
		static constexpr const bool IS_INT = INT;
	};

	template<> struct DigitalPin_trait<DigitalPin::NONE>: DigitalPin_trait_impl<Port::NONE, 0> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D0>: DigitalPin_trait_impl<Port::PORT_D, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D1>: DigitalPin_trait_impl<Port::PORT_D, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D2>: DigitalPin_trait_impl<Port::PORT_D, 2, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D3>: DigitalPin_trait_impl<Port::PORT_D, 3, true> {};
	template<> struct DigitalPin_trait<DigitalPin::D4>: DigitalPin_trait_impl<Port::PORT_D, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D5>: DigitalPin_trait_impl<Port::PORT_D, 5> {};
	template<> struct DigitalPin_trait<DigitalPin::D6>: DigitalPin_trait_impl<Port::PORT_D, 6> {};
	template<> struct DigitalPin_trait<DigitalPin::D7>: DigitalPin_trait_impl<Port::PORT_D, 7> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D8>: DigitalPin_trait_impl<Port::PORT_B, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D9>: DigitalPin_trait_impl<Port::PORT_B, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D10>: DigitalPin_trait_impl<Port::PORT_B, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D11>: DigitalPin_trait_impl<Port::PORT_B, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D12>: DigitalPin_trait_impl<Port::PORT_B, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D13>: DigitalPin_trait_impl<Port::PORT_B, 5> {};
	
	template<> struct DigitalPin_trait<DigitalPin::D14>: DigitalPin_trait_impl<Port::PORT_C, 0> {};
	template<> struct DigitalPin_trait<DigitalPin::D15>: DigitalPin_trait_impl<Port::PORT_C, 1> {};
	template<> struct DigitalPin_trait<DigitalPin::D16>: DigitalPin_trait_impl<Port::PORT_C, 2> {};
	template<> struct DigitalPin_trait<DigitalPin::D17>: DigitalPin_trait_impl<Port::PORT_C, 3> {};
	template<> struct DigitalPin_trait<DigitalPin::D18>: DigitalPin_trait_impl<Port::PORT_C, 4> {};
	template<> struct DigitalPin_trait<DigitalPin::D19>: DigitalPin_trait_impl<Port::PORT_C, 5> {};
	
	template<DigitalPin PIN>
	constexpr uint8_t BIT()
	{
		return DigitalPin_trait<PIN>::BIT;
	}
	
	//==============
	// Analog Input
	//==============
	template<AnalogReference AREF>
	struct AnalogReference_trait
	{
		static constexpr const uint8_t MASK = 0;
	};
	template<uint8_t MASK_>
	struct AnalogReference_trait_impl
	{
		static constexpr const uint8_t MASK = MASK_;
	};
	
	template<> struct AnalogReference_trait<AnalogReference::AREF>:AnalogReference_trait_impl<0> {};
	template<> struct AnalogReference_trait<AnalogReference::AVCC>:AnalogReference_trait_impl<_BV(REFS0)> {};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_1_1V>:AnalogReference_trait_impl<_BV(REFS1) | _BV(REFS0)> {};

	template<typename SAMPLE_TYPE>
	struct AnalogSampleType_trait
	{
		static constexpr const uint8_t ADLAR1 = 0;
		static constexpr const uint8_t ADLAR2 = 0;
		static constexpr const REGISTER _ADC{};
	};
	template<uint8_t ADLAR1_, uint8_t ADLAR2_, REG ADC_>
	struct AnalogSampleType_trait_impl
	{
		static constexpr const uint8_t ADLAR1 = ADLAR1_;
		static constexpr const uint8_t ADLAR2 = ADLAR2_;
		static constexpr const REGISTER _ADC = ADC_;
	};
	
	template<> struct AnalogSampleType_trait<uint16_t>: AnalogSampleType_trait_impl<0, 0, R_(ADC)> {};
	template<> struct AnalogSampleType_trait<uint8_t>: AnalogSampleType_trait_impl<_BV(ADLAR), 0, R_(ADCH)> {};

	template<AnalogClock MAXFREQ>
	struct AnalogClock_trait
	{
		static constexpr const uint8_t PRESCALER = 0;
		static constexpr const uint8_t PRESCALER_MASK = 0;
		
	};

	template<uint32_t MAXFREQ>
	struct AnalogClock_trait_impl
	{
		static constexpr uint8_t round_prescaler(uint16_t rate)
		{
			return (rate > 64 ? 128 :
					rate > 32 ? 64 :
					rate > 16 ? 32 :
					rate > 8 ? 16 :
					rate > 4 ? 8 :
					rate > 2 ? 4 :
					2);
		}
		static constexpr uint8_t prescaler_mask(uint8_t prescaler)
		{
			return (prescaler == 128 ? _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) :
					prescaler == 64 ? _BV(ADPS2) | _BV(ADPS1) :
					prescaler == 32 ? _BV(ADPS2) | _BV(ADPS0) :
					prescaler == 16 ? _BV(ADPS2) :
					prescaler == 8 ? _BV(ADPS1) | _BV(ADPS0) :
					prescaler == 4 ? _BV(ADPS1) :
					_BV(ADPS0));
		}

		static constexpr const uint8_t PRESCALER = round_prescaler(uint16_t(F_CPU / MAXFREQ));
		static constexpr const uint8_t PRESCALER_MASK = prescaler_mask(PRESCALER);
	};
	
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_50KHz>: AnalogClock_trait_impl<50000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_100KHz>: AnalogClock_trait_impl<100000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_200KHz>: AnalogClock_trait_impl<200000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_500KHz>: AnalogClock_trait_impl<500000UL> {};
	template<> struct AnalogClock_trait<AnalogClock::MAX_FREQ_1MHz>: AnalogClock_trait_impl<1000000UL> {};

	template<REG ADMUX__, REG ADCSRA__, REG ADCSRB__>
	struct GlobalAnalogPin_trait_impl
	{
		static constexpr const REGISTER ADMUX_ = ADMUX__;
		static constexpr const REGISTER ADCSRA_ = ADCSRA__;
		static constexpr const REGISTER ADCSRB_ = ADCSRB__;
	};
	struct GlobalAnalogPin_trait:GlobalAnalogPin_trait_impl<R_(ADMUX), R_(ADCSRA), R_(ADCSRB)> {};
	
	template<AnalogPin APIN>
	struct AnalogPin_trait
	{
		static constexpr const uint8_t MUX_MASK1 = 0;
		static constexpr const uint8_t MUX_MASK2 = 0;
		static constexpr const bool IS_BANDGAP = false;
		static constexpr const uint16_t BANDGAP_VOLTAGE_MV = 0xFFFF;
	};
	
	template<uint8_t MUXM1, uint8_t MUXM2 = 0, uint16_t VOLTAGE = 0xFFFF>
	struct AnalogPin_trait_impl
	{
		static constexpr const uint8_t MUX_MASK1 = MUXM1;
		static constexpr const uint8_t MUX_MASK2 = MUXM2;
		static constexpr const bool IS_BANDGAP = (VOLTAGE != 0xFFFF);
		static constexpr const uint16_t BANDGAP_VOLTAGE_MV = VOLTAGE;
	};
	
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

	template<DigitalPin DPIN>
	struct ExternalInterruptPin_trait
	{
		static constexpr const uint8_t INT = 0;
		static constexpr const REGISTER EICR_{};
		static constexpr const uint8_t EICR_MASK = 0x00;
		static constexpr const REGISTER EIMSK_{};
		static constexpr const uint8_t EIMSK_MASK = 0x00;
		static constexpr const REGISTER EIFR_{};
		static constexpr const uint8_t EIFR_MASK = 0x00;
	};

	template<uint8_t INT_, REG EICR__, uint8_t EICR_MASK_, REG EIMSK__, uint8_t EIMSK_MASK_, REG EIFR__, uint8_t EIFR_MASK_>
	struct ExternalInterruptPin_trait_impl
	{
		static constexpr const uint8_t INT = INT_;
		static constexpr const REGISTER EICR_ = EICR__;
		static constexpr const uint8_t EICR_MASK = EICR_MASK_;
		static constexpr const REGISTER EIMSK_ = EIMSK__;
		static constexpr const uint8_t EIMSK_MASK = EIMSK_MASK_;
		static constexpr const REGISTER EIFR_ = EIFR__;
		static constexpr const uint8_t EIFR_MASK = EIFR_MASK_;
	};

	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT0>: 
		ExternalInterruptPin_trait_impl<0, R_(EICRA), _BV(ISC00) | _BV(ISC01), R_(EIMSK), _BV(INT0), R_(EIFR), _BV(INTF0)> {};
	template<> struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT1>: 
		ExternalInterruptPin_trait_impl<1, R_(EICRA), _BV(ISC10) | _BV(ISC11), R_(EIMSK), _BV(INT1), R_(EIFR), _BV(INTF1)> {};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<uint8_t PCINT>
	struct PCI_trait
	{
		static constexpr const Port PORT = Port::NONE;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	template<Port PORT_, uint8_t PCI_MASK_, uint8_t PCICR_MASK_, uint8_t PCIFR_MASK_, REG PCICR__, REG PCIFR__, REG PCMSK__>
	struct PCI_trait_impl
	{
		static constexpr const Port PORT = PORT_;
		static constexpr const uint8_t PCI_MASK = PCI_MASK_;
		static constexpr const uint8_t PCICR_MASK = PCICR_MASK_; 
		static constexpr const uint8_t PCIFR_MASK = PCIFR_MASK_;
		static constexpr const REGISTER PCICR_ = PCICR__;
		static constexpr const REGISTER PCIFR_ = PCIFR__;
		static constexpr const REGISTER PCMSK_ = PCMSK__;
	};

	template<> struct PCI_trait<0>: 
		PCI_trait_impl<Port::PORT_B, 0x3F, _BV(PCIE0), _BV(PCIF0), R_(PCICR), R_(PCIFR), R_(PCMSK0)> {};
	template<> struct PCI_trait<1>: 
		PCI_trait_impl<Port::PORT_C, 0x3F, _BV(PCIE1), _BV(PCIF1), R_(PCICR), R_(PCIFR), R_(PCMSK1)> {};
	template<> struct PCI_trait<2>: 
		PCI_trait_impl<Port::PORT_D, 0xFF, _BV(PCIE2), _BV(PCIF2), R_(PCICR), R_(PCIFR), R_(PCMSK2)> {};

	//=======
	// USART
	//=======
	
	template<USART USART>
	struct USART_trait
	{
		static constexpr const REGISTER UCSRA{};
		static constexpr const REGISTER UCSRB{};
		static constexpr const REGISTER UCSRC{};
		static constexpr const REGISTER UDR{};
		static constexpr const REGISTER UBRR{};
	};
	template<REG UCSRA_, REG UCSRB_, REG UCSRC_, REG UDR_, REG UBRR_>
	struct USART_trait_impl
	{
		static constexpr const REGISTER UCSRA = UCSRA_;
		static constexpr const REGISTER UCSRB = UCSRB_;
		static constexpr const REGISTER UCSRC = UCSRC_;
		static constexpr const REGISTER UDR = UDR_;
		static constexpr const REGISTER UBRR = UBRR_;
	};
	
	template<> struct USART_trait<USART::USART0>: USART_trait_impl<R_(UCSR0A), R_(UCSR0B), R_(UCSR0C), R_(UDR0), R_(UBRR0)> {};
	
	//=====
	// SPI
	//=====

	template<Port PORT_, uint8_t SS_, uint8_t MOSI_, uint8_t MISO_, uint8_t SCK_>
	struct SPI_trait_impl
	{
		//TODO ultimately replace 3 lines with just reference to Port_trait
		using PORT_TRAIT = Port_trait<PORT_>;
		static constexpr const REGISTER DDR = PORT_TRAIT::DDR;
		static constexpr const REGISTER PORT = PORT_TRAIT::PORT;
		
		static constexpr const uint8_t SS = PB2;
		static constexpr const uint8_t MOSI = PB3;
		static constexpr const uint8_t MISO = PB4;
		static constexpr const uint8_t SCK = PB5;
	};
	struct SPI_trait: SPI_trait_impl<Port::PORT_B, PB2, PB3, PB4, PB5> {};

	//========
	// Timers
	//========

	//TODO externalize BEGIN
	enum TimerPrescalers: uint8_t
	{
		PRESCALERS_1_8_64_256_1024,
		PRESCALERS_1_8_32_64_128_256_1024,
		PRESCALERS_NONE
	};
	
	template<TimerPrescalers PRESCALERS>
	struct TimerPrescalers_trait
	{
		enum class TimerPrescaler: uint8_t {};
		using TYPE = TimerPrescaler;
		static constexpr const TimerPrescaler ALL_PRESCALERS[] = {};
	};
	
	template<>
	struct TimerPrescalers_trait<TimerPrescalers::PRESCALERS_1_8_64_256_1024>
	{
		enum class TimerPrescaler: uint8_t
		{
			NO_PRESCALING	= 0,
			DIV_8			= 3,
			DIV_64			= 6,
			DIV_256			= 8,
			DIV_1024		= 10
		};
		using TYPE = TimerPrescaler;
		static constexpr const TimerPrescaler ALL_PRESCALERS[] = 
		{
			TimerPrescaler::NO_PRESCALING,
			TimerPrescaler::DIV_8,
			TimerPrescaler::DIV_64,
			TimerPrescaler::DIV_256,
			TimerPrescaler::DIV_1024
		};
	};
	
	template<>
	struct TimerPrescalers_trait<TimerPrescalers::PRESCALERS_1_8_32_64_128_256_1024>
	{
		enum class TimerPrescaler: uint8_t
		{
			NO_PRESCALING	= 0,
			DIV_8			= 3,
			DIV_32			= 5,
			DIV_64			= 6,
			DIV_128			= 7,
			DIV_256			= 8,
			DIV_1024		= 10
		};
		using TYPE = TimerPrescaler;
		static constexpr const TimerPrescaler ALL_PRESCALERS[] = 
		{
			TimerPrescaler::NO_PRESCALING,
			TimerPrescaler::DIV_8,
			TimerPrescaler::DIV_32,
			TimerPrescaler::DIV_64,
			TimerPrescaler::DIV_128,
			TimerPrescaler::DIV_256,
			TimerPrescaler::DIV_1024
		};
	};
	
	template<typename TYPE_> struct Timer_type_trait
	{
		static constexpr const uint32_t MAX_COUNTER = 1UL << (8 * sizeof(TYPE_));
	};
	//TODO externalize END
	
	template<Timer TIMER>
	struct Timer_trait
	{
		using TYPE = uint8_t;
		static constexpr const uint32_t MAX_COUNTER = 0;

		static constexpr const TimerPrescalers PRESCALERS = TimerPrescalers::PRESCALERS_NONE;
		using PRESCALERS_TRAIT = TimerPrescalers_trait<PRESCALERS>;
		using TIMER_PRESCALER = PRESCALERS_TRAIT::TYPE;
		
		static constexpr const uint8_t CTC_TCCRA  = 0;
		static constexpr const uint8_t CTC_TCCRB  = 0;
		static constexpr const REGISTER TCCRA{};
		static constexpr const REGISTER TCCRB{};
		static constexpr const REGISTER TCNT{};
		static constexpr const REGISTER OCRA{};
		static constexpr const REGISTER OCRB{};
		static constexpr const REGISTER TIMSK{};
		static constexpr const REGISTER TIFR{};
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return 0;
		}
	};

	template<typename TYPE_, TimerPrescalers PRESCALERS_, uint8_t CTC_TCCRA_, uint8_t CTC_TCCRB_, REG TCCRA_, REG TCCRB_, REG TCNT_, REG OCRA_, REG OCRB_, REG TIMSK_, REG TIFR_>
	struct Timer_trait_impl
	{
		using TYPE = TYPE_;
		static constexpr const uint32_t MAX_COUNTER = Timer_type_trait<TYPE>::MAX_COUNTER;

		static constexpr const TimerPrescalers PRESCALERS = PRESCALERS_;
		using PRESCALERS_TRAIT = TimerPrescalers_trait<PRESCALERS>;
		using TIMER_PRESCALER = typename PRESCALERS_TRAIT::TYPE;
		
		static constexpr const uint8_t CTC_TCCRA  = CTC_TCCRA_;
		static constexpr const uint8_t CTC_TCCRB  = CTC_TCCRB_;
		static constexpr const REGISTER TCCRA = TCCRA_;
		static constexpr const REGISTER TCCRB = TCCRB_;
		static constexpr const REGISTER TCNT = TCNT_;
		static constexpr const REGISTER OCRA = OCRA_;
		static constexpr const REGISTER OCRB = OCRB_;
		static constexpr const REGISTER TIMSK = TIMSK_;
		static constexpr const REGISTER TIFR = TIFR_;
	};
	
	//TODO IMPROVE NEW STUFF: TimerPrescaler should be put in only one header (same for all boards)
	// Also define two constexpr methods for both kinds of timers prescaling values list, then attach point to there here
	// Also define default prescaler options for MS timers
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
};

#endif /* BOARDS_UNO_TRAITS_HH */
