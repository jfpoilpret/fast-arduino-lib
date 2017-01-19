#ifndef BOARDS_UNO_TRAITS_HH
#define BOARDS_UNO_TRAITS_HH

#include <avr/io.h>
#include "Uno.hh"

namespace Board
{
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
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_{};
	};
	
	template<>
	struct Port_trait<Port::PORT_B>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINB);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRB);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTB);
		static constexpr const uint8_t DPIN_MASK = 0xFF;
		
		static constexpr const uint8_t PCINT = 0;
		static constexpr const uint8_t PCI_MASK = 0x3F;
		static constexpr const uint8_t PCICR_MASK = _BV(PCIE0); 
		static constexpr const uint8_t PCIFR_MASK = _BV(PCIF0);
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_ = _SELECT_REG(PCMSK0);
	};
	
	template<>
	struct Port_trait<Port::PORT_C>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PINC);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRC);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTC);
		static constexpr const uint8_t DPIN_MASK = 0x7F;

		static constexpr const uint8_t PCINT = 1;
		static constexpr const uint8_t PCI_MASK = 0x3F;
		static constexpr const uint8_t PCICR_MASK = _BV(PCIE1); 
		static constexpr const uint8_t PCIFR_MASK = _BV(PCIF1);
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_ = _SELECT_REG(PCMSK1);
	};
	
	template<>
	struct Port_trait<Port::PORT_D>
	{
		static constexpr const REGISTER PIN = _SELECT_REG(PIND);
		static constexpr const REGISTER DDR = _SELECT_REG(DDRD);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTD);
		static constexpr const uint8_t DPIN_MASK = 0xFF;

		static constexpr const uint8_t PCINT = 2;
		static constexpr const uint8_t PCI_MASK = 0xFF;
		static constexpr const uint8_t PCICR_MASK = _BV(PCIE2); 
		static constexpr const uint8_t PCIFR_MASK = _BV(PCIF2);
		static constexpr const REGISTER PCICR_ = _SELECT_REG(PCICR);
		static constexpr const REGISTER PCIFR_ = _SELECT_REG(PCIFR);
		static constexpr const REGISTER PCMSK_ = _SELECT_REG(PCMSK2);
	};
	
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
	
	template<> struct AnalogReference_trait<AnalogReference::AREF>
	{
		static constexpr const uint8_t MASK = 0;
	};
	template<> struct AnalogReference_trait<AnalogReference::AVCC>
	{
		static constexpr const uint8_t MASK = _BV(REFS0);
	};
	template<> struct AnalogReference_trait<AnalogReference::INTERNAL_1_1V>
	{
		static constexpr const uint8_t MASK = _BV(REFS1) | _BV(REFS0);
	};

	template<typename SAMPLE_TYPE>
	struct AnalogSampleType_trait
	{
		static constexpr const uint8_t ADLAR1 = 0;
		static constexpr const uint8_t ADLAR2 = 0;
		static constexpr const REGISTER _ADC{};
	};
	
	template<>
	struct AnalogSampleType_trait<uint16_t>
	{
		static constexpr const uint8_t ADLAR1 = 0;
		static constexpr const uint8_t ADLAR2 = 0;
		static constexpr const REGISTER _ADC = _SELECT_REG(ADC);
	};
	template<>
	struct AnalogSampleType_trait<uint8_t>
	{
		static constexpr const uint8_t ADLAR1 = _BV(ADLAR);
		static constexpr const uint8_t ADLAR2 = 0;
		static constexpr const REGISTER _ADC = _SELECT_REG(ADCH);
	};

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
	
	struct GlobalAnalogPin_trait
	{
		static constexpr const REGISTER ADMUX_ = _SELECT_REG(ADMUX);
		static constexpr const REGISTER ADCSRA_ = _SELECT_REG(ADCSRA);
		static constexpr const REGISTER ADCSRB_ = _SELECT_REG(ADCSRB);
	};
	
	//TODO Check if and when we need DIDR0 stuff
	template<AnalogPin APIN>
	struct AnalogPin_trait
	{
//		static constexpr const REGISTER DIDR_ = _SELECT_REG(DIDR0);
//		static constexpr const uint8_t DIDR_MASK = 0;
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

	template<>
	struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT0>
	{
		static constexpr const uint8_t INT = 0;
		static constexpr const REGISTER EICR_ = _SELECT_REG(EICRA);
		static constexpr const uint8_t EICR_MASK = _BV(ISC00) | _BV(ISC01);
		static constexpr const REGISTER EIMSK_ = _SELECT_REG(EIMSK);
		static constexpr const uint8_t EIMSK_MASK = _BV(INT0);
		static constexpr const REGISTER EIFR_ = _SELECT_REG(EIFR);
		static constexpr const uint8_t EIFR_MASK = _BV(INTF0);
	};

	template<>
	struct ExternalInterruptPin_trait<ExternalInterruptPin::EXT1>
	{
		static constexpr const uint8_t INT = 1;
		static constexpr const REGISTER EICR_ = _SELECT_REG(EICRA);
		static constexpr const uint8_t EICR_MASK = _BV(ISC10) | _BV(ISC11);
		static constexpr const REGISTER EIMSK_ = _SELECT_REG(EIMSK);
		static constexpr const uint8_t EIMSK_MASK = _BV(INT1);
		static constexpr const REGISTER EIFR_ = _SELECT_REG(EIFR);
		static constexpr const uint8_t EIFR_MASK = _BV(INTF1);
	};

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	template<uint8_t PCINT>
	struct PCI_trait
	{
		static constexpr const Port PORT = Port::NONE;
	};
	template<>
	struct PCI_trait<0>
	{
		static constexpr const Port PORT = Port::PORT_B;
	};
	template<>
	struct PCI_trait<1>
	{
		static constexpr const Port PORT = Port::PORT_C;
	};
	template<>
	struct PCI_trait<2>
	{
		static constexpr const Port PORT = Port::PORT_D;
	};

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
	
	template<>
	struct USART_trait<USART::USART0>
	{
		static constexpr const REGISTER UCSRA = _SELECT_REG(UCSR0A);
		static constexpr const REGISTER UCSRB = _SELECT_REG(UCSR0B);
		static constexpr const REGISTER UCSRC = _SELECT_REG(UCSR0C);
		static constexpr const REGISTER UDR = _SELECT_REG(UDR0);
		static constexpr const REGISTER UBRR = _SELECT_REG(UBRR0);
	};
	
	//=====
	// SPI
	//=====

	struct SPI_trait
	{
		static constexpr const REGISTER DDR = _SELECT_REG(DDRB);
		static constexpr const REGISTER PORT = _SELECT_REG(PORTB);
		static constexpr const uint8_t SS = PB2;
		static constexpr const uint8_t MOSI = PB3;
		static constexpr const uint8_t MISO = PB4;
		static constexpr const uint8_t SCK = PB5;
	};

	//========
	// Timers
	//========
	
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
	
	//TODO IMPROVE NEW STUFF: TimerPrescaler should be put in only one header (same for all boards)
	// Also define two constexpr methods for both kinds of timers prescaling values list, then attach point to there here
	// Also define default prescaler options for MS timers
	//TODO how to prevent using forbidden Prescaler values???
	template<>
	struct Timer_trait<Timer::TIMER0>
	{
		using TYPE = uint8_t;
		static constexpr const uint32_t MAX_COUNTER = 256;

		static constexpr const TimerPrescalers PRESCALERS = TimerPrescalers::PRESCALERS_1_8_64_256_1024;
		using PRESCALERS_TRAIT = TimerPrescalers_trait<PRESCALERS>;
		using TIMER_PRESCALER = PRESCALERS_TRAIT::TYPE;
		
		static constexpr const uint8_t CTC_TCCRA  = _BV(WGM01);
		static constexpr const uint8_t CTC_TCCRB  = 0;
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR0A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR0B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT0);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR0A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR0B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK0);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR0);
		
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return (p == TIMER_PRESCALER::NO_PRESCALING ? _BV(CS00) :
					p == TIMER_PRESCALER::DIV_8 ? _BV(CS01) :
					p == TIMER_PRESCALER::DIV_64 ? _BV(CS00) | _BV(CS01) :
					p == TIMER_PRESCALER::DIV_256 ? _BV(CS02) :
					p == TIMER_PRESCALER::DIV_1024 ? _BV(CS02) | _BV(CS00) :
					0);
		}
	};
	
	template<>
	struct Timer_trait<Timer::TIMER2>
	{
		using TYPE = uint8_t;
		static constexpr const uint32_t MAX_COUNTER = 256;

		static constexpr const TimerPrescalers PRESCALERS = TimerPrescalers::PRESCALERS_1_8_32_64_128_256_1024;
		using PRESCALERS_TRAIT = TimerPrescalers_trait<PRESCALERS>;
		using TIMER_PRESCALER = PRESCALERS_TRAIT::TYPE;
		
		static constexpr const uint8_t CTC_TCCRA  = _BV(WGM21);
		static constexpr const uint8_t CTC_TCCRB  = 0;
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR2A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR2B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT2);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR2A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR2B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK2);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR2);

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
	
	template<>
	struct Timer_trait<Timer::TIMER1>
	{
		using TYPE = uint16_t;
		static constexpr const uint32_t MAX_COUNTER = 65536;

		static constexpr const TimerPrescalers PRESCALERS = TimerPrescalers::PRESCALERS_1_8_64_256_1024;
		using PRESCALERS_TRAIT = TimerPrescalers_trait<PRESCALERS>;
		using TIMER_PRESCALER = PRESCALERS_TRAIT::TYPE;

		static constexpr const uint8_t CTC_TCCRA  = 0;
		static constexpr const uint8_t CTC_TCCRB  = _BV(WGM12);
		static constexpr const REGISTER TCCRA = _SELECT_REG(TCCR1A);
		static constexpr const REGISTER TCCRB = _SELECT_REG(TCCR1B);
		static constexpr const REGISTER TCNT = _SELECT_REG(TCNT1);
		static constexpr const REGISTER OCRA = _SELECT_REG(OCR1A);
		static constexpr const REGISTER OCRB = _SELECT_REG(OCR1B);
		static constexpr const REGISTER TIMSK = _SELECT_REG(TIMSK1);
		static constexpr const REGISTER TIFR = _SELECT_REG(TIFR1);

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
