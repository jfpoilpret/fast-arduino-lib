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

#ifndef BOARDS_COMMON_TRAITS_HH
#define BOARDS_COMMON_TRAITS_HH

#include <avr/io.h>

#include "../utilities.h"
#include "../board.h"

// This internal macro is used by individual boards headers
#define R_(REG) ((uint16_t)&REG)

namespace board_traits
{
	template<typename T>
	class REGISTER
	{
	public:
		constexpr REGISTER():ADDR(0) {}
		constexpr REGISTER(uint16_t ADDR) INLINE:ADDR(ADDR) {}

		void operator =(int value) const INLINE 
		{
			*((volatile T*) ADDR) = (T) value;
		}
		void operator |=(int value) const INLINE 
		{
			*((volatile T*) ADDR) |= (T) value;
		}
		void operator &=(int value) const INLINE 
		{
			*((volatile T*) ADDR) &= (T) value;
		}
		void operator ^=(int value) const INLINE 
		{
			*((volatile T*) ADDR) ^= (T) value;
		}
		T operator ~() const INLINE 
		{
			return ~(*((volatile T*) ADDR));
		}
		void loop_until_bit_set(uint8_t bit) const INLINE
		{
			while (!(*((volatile T*) ADDR) & _BV(bit))) ;
		}
		void loop_until_bit_clear(uint8_t bit) const INLINE
		{
			while (*((volatile T*) ADDR) | _BV(bit)) ;
		}
		bool operator ==(T value) const INLINE
		{
			return *((volatile T*) ADDR) == value;
		}
		bool operator !=(T value) const INLINE
		{
			return *((volatile T*) ADDR) != value;
		}
		bool operator >(T value) const INLINE
		{
			return *((volatile T*) ADDR) > value;
		}
		bool operator >=(T value) const INLINE
		{
			return *((volatile T*) ADDR) >= value;
		}
		bool operator <(T value) const INLINE
		{
			return *((volatile T*) ADDR) < value;
		}
		bool operator <=(T value) const INLINE
		{
			return *((volatile T*) ADDR) <= value;
		}
		operator volatile T&() const INLINE 
		{
			return *((volatile T*) ADDR);
		}

	private:	
		const uint16_t ADDR;
	};

	using REG8 = REGISTER<uint8_t>;
	using REG16 = REGISTER<uint16_t>;

	using namespace ::Board;
	using REG = uint16_t;
	
	template<Port P>
	struct Port_trait
	{
		static constexpr const REG8 PIN{};
		static constexpr const REG8 DDR{};
		static constexpr const REG8 PORT{};
		static constexpr const uint8_t DPIN_MASK = 0x00;
		static constexpr const uint8_t PCINT = 0;
	};
	template<REG PIN_, REG DDR_, REG PORT_, uint8_t DPIN_MASK_, uint8_t PCINT_>
	struct Port_trait_impl
	{
		static constexpr const REG8 PIN{PIN_};
		static constexpr const REG8 DDR{DDR_};
		static constexpr const REG8 PORT{PORT_};
		static constexpr const uint8_t DPIN_MASK = DPIN_MASK_;
		static constexpr const uint8_t PCINT = PCINT_;
	};
	
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
	
	template<typename SAMPLE_TYPE> 
	struct AnalogSampleType_trait
	{
		static constexpr const uint8_t ADLAR1 = 0;
		static constexpr const uint8_t ADLAR2 = 0;
		static constexpr const REGISTER<SAMPLE_TYPE> _ADC{};
	};
	template<typename SAMPLE_TYPE, uint8_t ADLAR1_, uint8_t ADLAR2_, REG ADC_>
	struct AnalogSampleType_trait_impl
	{
		static constexpr const uint8_t ADLAR1 = ADLAR1_;
		static constexpr const uint8_t ADLAR2 = ADLAR2_;
		static constexpr const REGISTER<SAMPLE_TYPE> _ADC = ADC_;
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
	
	template<REG ADMUX__, REG ADCSRA__, REG ADCSRB__>
	struct GlobalAnalogPin_trait_impl
	{
		static constexpr const REG8 ADMUX_ = ADMUX__;
		static constexpr const REG8 ADCSRA_ = ADCSRA__;
		static constexpr const REG8 ADCSRB_ = ADCSRB__;
	};
	
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
	
	template<DigitalPin DPIN> 
	struct ExternalInterruptPin_trait
	{
		static constexpr const uint8_t INT = 0;
		static constexpr const REG8 EICR_{};
		static constexpr const uint8_t EICR_MASK = 0x00;
		static constexpr const REG8 EIMSK_{};
		static constexpr const uint8_t EIMSK_MASK = 0x00;
		static constexpr const REG8 EIFR_{};
		static constexpr const uint8_t EIFR_MASK = 0x00;
	};
	template<uint8_t INT_, REG EICR__, uint8_t EICR_MASK_, REG EIMSK__, uint8_t EIMSK_MASK_, REG EIFR__, uint8_t EIFR_MASK_>
	struct ExternalInterruptPin_trait_impl
	{
		static constexpr const uint8_t INT = INT_;
		static constexpr const REG8 EICR_ = EICR__;
		static constexpr const uint8_t EICR_MASK = EICR_MASK_;
		static constexpr const REG8 EIMSK_ = EIMSK__;
		static constexpr const uint8_t EIMSK_MASK = EIMSK_MASK_;
		static constexpr const REG8 EIFR_ = EIFR__;
		static constexpr const uint8_t EIFR_MASK = EIFR_MASK_;
	};

	template<uint8_t PCINT> 
	struct PCI_trait
	{
		static constexpr const Port PORT = Port::NONE;
		static constexpr const uint8_t PCI_MASK = 0x00;
		static constexpr const uint8_t PCICR_MASK = 0x00; 
		static constexpr const uint8_t PCIFR_MASK = 0x00;
		static constexpr const REG8 PCICR_{};
		static constexpr const REG8 PCIFR_{};
		static constexpr const REG8 PCMSK_{};
	};
	template<Port PORT_, uint8_t PCI_MASK_, uint8_t PCICR_MASK_, uint8_t PCIFR_MASK_, REG PCICR__, REG PCIFR__, REG PCMSK__>
	struct PCI_trait_impl
	{
		static constexpr const Port PORT = PORT_;
		static constexpr const uint8_t PCI_MASK = PCI_MASK_;
		static constexpr const uint8_t PCICR_MASK = PCICR_MASK_; 
		static constexpr const uint8_t PCIFR_MASK = PCIFR_MASK_;
		static constexpr const REG8 PCICR_ = PCICR__;
		static constexpr const REG8 PCIFR_ = PCIFR__;
		static constexpr const REG8 PCMSK_ = PCMSK__;
	};

	template<USART USART> 
	struct USART_trait
	{
		static constexpr const REG8 UCSRA{};
		static constexpr const REG8 UCSRB{};
		static constexpr const REG8 UCSRC{};
		static constexpr const REG8 UDR{};
		static constexpr const REG16 UBRR{};
	};
	template<REG UCSRA_, REG UCSRB_, REG UCSRC_, REG UDR_, REG UBRR_>
	struct USART_trait_impl
	{
		static constexpr const REG8 UCSRA = UCSRA_;
		static constexpr const REG8 UCSRB = UCSRB_;
		static constexpr const REG8 UCSRC = UCSRC_;
		static constexpr const REG8 UDR = UDR_;
		static constexpr const REG16 UBRR = UBRR_;
	};
	
	template<Port PORT_, uint8_t SS_, uint8_t MOSI_, uint8_t MISO_, uint8_t SCK_>
	struct SPI_trait_impl
	{
		//TODO ultimately replace 3 lines with just reference to Port_trait
		using PORT_TRAIT = Port_trait<PORT_>;
		static constexpr const REG8 DDR = PORT_TRAIT::DDR;
		static constexpr const REG8 PORT = PORT_TRAIT::PORT;
		
		static constexpr const uint8_t SS = SS_;
		static constexpr const uint8_t MOSI = MOSI_;
		static constexpr const uint8_t MISO = MISO_;
		static constexpr const uint8_t SCK = SCK_;
	};

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
	template<> struct TimerPrescalers_trait<TimerPrescalers::PRESCALERS_1_8_64_256_1024>
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
	template<> struct TimerPrescalers_trait<TimerPrescalers::PRESCALERS_1_8_32_64_128_256_1024>
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
	
	template<typename TYPE>
	struct Timer_type_trait
	{
		static constexpr const uint32_t MAX_COUNTER = 1UL << (8 * sizeof(TYPE));
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
		static constexpr const REG8 TCCRA{};
		static constexpr const REG8 TCCRB{};
		static constexpr const REGISTER<TYPE> TCNT{};
		static constexpr const REGISTER<TYPE> OCRA{};
		static constexpr const REGISTER<TYPE> OCRB{};
		static constexpr const REG8 TIMSK{};
		static constexpr const REG8 TIFR{};
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
		static constexpr const REG8 TCCRA = TCCRA_;
		static constexpr const REG8 TCCRB = TCCRB_;
		static constexpr const REGISTER<TYPE> TCNT = TCNT_;
		static constexpr const REGISTER<TYPE> OCRA = OCRA_;
		static constexpr const REGISTER<TYPE> OCRB = OCRB_;
		static constexpr const REG8 TIMSK = TIMSK_;
		static constexpr const REG8 TIFR = TIFR_;
	};
};

namespace Board
{
	template<DigitalPin PIN>
	constexpr uint8_t BIT()
	{
		return board_traits::DigitalPin_trait<PIN>::BIT;
	}
};	

#endif /* BOARDS_COMMON_TRAITS_HH */
