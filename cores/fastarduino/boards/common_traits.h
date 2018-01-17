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

#ifndef BOARDS_COMMON_TRAITS_HH
#define BOARDS_COMMON_TRAITS_HH

#include <avr/io.h>

#include "board.h"
#include "../uart_commons.h"

// This internal macro is used by individual boards headers
#define R_(REG) ((uint16_t)&REG)

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif

namespace board_traits
{
	template<typename T>
	class REGISTER
	{
	public:
		constexpr REGISTER():ADDR(0xFFFF) {}
		constexpr REGISTER(uint16_t ADDR) INLINE:ADDR(ADDR) {}

		bool is_no_reg() const INLINE
		{
			return ADDR == 0xFFFF;
		}
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
			while (*((volatile T*) ADDR) & _BV(bit)) ;
		}
		bool operator ==(int value) const INLINE
		{
			return *((volatile T*) ADDR) == (T) value;
		}
		bool operator !=(int value) const INLINE
		{
			return *((volatile T*) ADDR) != (T) value;
		}
		bool operator >(int value) const INLINE
		{
			return *((volatile T*) ADDR) > (T) value;
		}
		bool operator >=(int value) const INLINE
		{
			return *((volatile T*) ADDR) >= (T) value;
		}
		bool operator <(int value) const INLINE
		{
			return *((volatile T*) ADDR) < (T) value;
		}
		bool operator <=(int value) const INLINE
		{
			return *((volatile T*) ADDR) <= (T) value;
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

	using namespace ::board;
	using REG = uint16_t;
	static constexpr REG NO_REG = 0xFFFF;
	
	template<Port P>
	struct Port_trait
	{
		static constexpr const REG8 PIN{};
		static constexpr const REG8 DDR{};
		static constexpr const REG8 PORT{};
		static constexpr const uint8_t DPIN_MASK = 0x00;
		static constexpr const uint8_t PCINT = 0xFF;
	};
	template<REG PIN_, REG DDR_, REG PORT_, uint8_t DPIN_MASK_, uint8_t PCINT_ = 0xFF>
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
		static constexpr const REGISTER<SAMPLE_TYPE> ADC_{};
	};
	template<typename SAMPLE_TYPE, uint8_t ADLAR1_, uint8_t ADLAR2_, REG ADC__>
	struct AnalogSampleType_trait_impl
	{
		static constexpr const uint8_t ADLAR1 = ADLAR1_;
		static constexpr const uint8_t ADLAR2 = ADLAR2_;
		static constexpr const REGISTER<SAMPLE_TYPE> ADC_ = ADC__;
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
		static constexpr const uint8_t U2X_MASK = 0;
		static constexpr const uint8_t TX_ENABLE_MASK = 0;
		static constexpr const uint8_t RX_ENABLE_MASK = 0;
		static constexpr const uint8_t UDRIE_MASK = 0;
		static constexpr const uint8_t RXCIE_MASK = 0;
		static constexpr const uint8_t DOR_MASK = 0;
		static constexpr const uint8_t FE_MASK = 0;
		static constexpr const uint8_t UPE_MASK = 0;
		static constexpr uint8_t UCSRC_value(serial::Parity parity, serial::StopBits stopbits)
		{
			return 0;
		}
	};
	template<REG UCSRA_, REG UCSRB_, REG UCSRC_, REG UDR_, REG UBRR_, 
			uint8_t U2X_BIT, uint8_t TX_ENABLE_BIT, uint8_t RX_ENABLE_BIT, uint8_t UDRIE_BIT, uint8_t RXCIE_BIT,
			uint8_t DOR_BIT, uint8_t FE_BIT, uint8_t UPE_BIT>
	struct USART_trait_impl
	{
		static constexpr const REG8 UCSRA = UCSRA_;
		static constexpr const REG8 UCSRB = UCSRB_;
		static constexpr const REG8 UCSRC = UCSRC_;
		static constexpr const REG8 UDR = UDR_;
		static constexpr const REG16 UBRR = UBRR_;
		static constexpr const uint8_t U2X_MASK = _BV(U2X_BIT);
		static constexpr const uint8_t TX_ENABLE_MASK = _BV(TX_ENABLE_BIT);
		static constexpr const uint8_t RX_ENABLE_MASK = _BV(RX_ENABLE_BIT);
		static constexpr const uint8_t UDRIE_MASK = _BV(UDRIE_BIT);
		static constexpr const uint8_t RXCIE_MASK = _BV(RXCIE_BIT);
		static constexpr const uint8_t DOR_MASK = _BV(DOR_BIT);
		static constexpr const uint8_t FE_MASK = _BV(FE_BIT);
		static constexpr const uint8_t UPE_MASK = _BV(UPE_BIT);
	};
	
	template<Port PORT_, uint8_t SS_, uint8_t MOSI_, uint8_t MISO_, uint8_t SCK_>
	struct SPI_trait_impl
	{
		using PORT_TRAIT = Port_trait<PORT_>;
		static constexpr const REG8 DDR = PORT_TRAIT::DDR;
		static constexpr const REG8 PORT = PORT_TRAIT::PORT;
		
		static constexpr const uint8_t SS = SS_;
		static constexpr const uint8_t MOSI = MOSI_;
		static constexpr const uint8_t MISO = MISO_;
		static constexpr const uint8_t SCK = SCK_;
	};
	
	template<Port PORT_, uint8_t SCL_, uint8_t SDA_>
	struct TWI_trait_impl
	{
		using PORT_TRAIT = Port_trait<PORT_>;
		static constexpr const REG8 PORT = PORT_TRAIT::PORT;
		static constexpr const REG8 PIN = PORT_TRAIT::PIN;
		static constexpr const REG8 DDR = PORT_TRAIT::DDR;
		static constexpr const uint8_t SCL_SDA_MASK = _BV(SCL_) | _BV(SDA_);
		static constexpr const uint8_t BIT_SCL = SCL_;
		static constexpr const uint8_t BIT_SDA = SDA_;
	};

	enum TimerPrescalers: uint8_t
	{
		PRESCALERS_1_8_64_256_1024,
		PRESCALERS_1_8_32_64_128_256_1024,
		PRESCALERS_1_TO_16384,
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
	template<> struct TimerPrescalers_trait<TimerPrescalers::PRESCALERS_1_TO_16384>
	{
		enum class TimerPrescaler: uint8_t
		{
			NO_PRESCALING	= 0,
			DIV_2			= 1,
			DIV_4			= 2,
			DIV_8			= 3,
			DIV_16			= 4,
			DIV_32			= 5,
			DIV_64			= 6,
			DIV_128			= 7,
			DIV_256			= 8,
			DIV_512			= 9,
			DIV_1024		= 10,
			DIV_2048		= 11,
			DIV_4096		= 12,
			DIV_8192		= 13,
			DIV_16384		= 14
		};
		using TYPE = TimerPrescaler;
		static constexpr const TimerPrescaler ALL_PRESCALERS[] = 
		{
			TimerPrescaler::NO_PRESCALING,
			TimerPrescaler::DIV_2,
			TimerPrescaler::DIV_4,
			TimerPrescaler::DIV_8,
			TimerPrescaler::DIV_16,
			TimerPrescaler::DIV_32,
			TimerPrescaler::DIV_64,
			TimerPrescaler::DIV_128,
			TimerPrescaler::DIV_256,
			TimerPrescaler::DIV_512,
			TimerPrescaler::DIV_1024,
			TimerPrescaler::DIV_2048,
			TimerPrescaler::DIV_4096,
			TimerPrescaler::DIV_8192,
			TimerPrescaler::DIV_16384
		};
	};
	
	template<typename TYPE>
	struct Timer_type_trait
	{
		static constexpr const uint32_t MAX_COUNTER = 1UL << (8 * sizeof(TYPE));
		static constexpr const uint16_t MAX_PWM = MAX_COUNTER - 1;
	};
	template<>
	struct Timer_type_trait<uint16_t>
	{
		static constexpr const uint32_t MAX_COUNTER = 1UL << (8 * sizeof(uint16_t));
		static constexpr const uint16_t MAX_PWM = 0x3FF;
	};
	
	template<Timer TIMER, uint8_t COM>
	struct Timer_COM_trait
	{
		using TYPE = uint8_t;
		static constexpr const DigitalPin PIN_OCR = DigitalPin::NONE;
		static constexpr const REGISTER<TYPE> OCR{};
		static constexpr const uint8_t COM_MASK = 0;
		static constexpr const uint8_t COM_NORMAL = 0;
		static constexpr const uint8_t COM_TOGGLE = 0;
		static constexpr const uint8_t COM_CLEAR = 0;
		static constexpr const uint8_t COM_SET = 0;
	};
	template<typename TYPE_, DigitalPin PIN_OCR_, REG OCR_, uint8_t COM_MASK_, 
			uint8_t COM_NORMAL_, uint8_t COM_TOGGLE_, uint8_t COM_CLEAR_, uint8_t COM_SET_>
	struct Timer_COM_trait_impl
	{
		using TYPE = TYPE_;
		static constexpr const DigitalPin PIN_OCR = PIN_OCR_;
		static constexpr const REGISTER<TYPE> OCR = OCR_;
		static constexpr const uint8_t COM_MASK = COM_MASK_;
		static constexpr const uint8_t COM_NORMAL = COM_NORMAL_;
		static constexpr const uint8_t COM_TOGGLE = COM_TOGGLE_;
		static constexpr const uint8_t COM_CLEAR = COM_CLEAR_;
		static constexpr const uint8_t COM_SET = COM_SET_;
	};

	// Constants for all possible Timer interrupts
	namespace TimerInterrupt
	{
		static constexpr const uint8_t OVERFLOW = 0x01;
		static constexpr const uint8_t OUTPUT_COMPARE_A = 0x02;
		static constexpr const uint8_t OUTPUT_COMPARE_B = 0x04;
		static constexpr const uint8_t OUTPUT_COMPARE_C = 0x08;
		static constexpr const uint8_t INPUT_CAPTURE = 0x10;
	};

	template<Timer TIMER>
	struct Timer_trait
	{
		using TYPE = uint8_t;
		static constexpr const bool IS_16BITS = false;
		static constexpr const uint32_t MAX_COUNTER = 0;
		static constexpr const uint16_t MAX_PWM = 0;

		static constexpr const TimerPrescalers PRESCALERS = TimerPrescalers::PRESCALERS_NONE;
		using PRESCALERS_TRAIT = TimerPrescalers_trait<PRESCALERS>;
		using TIMER_PRESCALER = PRESCALERS_TRAIT::TYPE;
		
		static constexpr const uint8_t COM_COUNT = 0;
		static constexpr const uint8_t COM_MASK = 0;

		static constexpr const uint8_t F_PWM_TCCRA = 0;
		static constexpr const uint8_t F_PWM_TCCRB = 0;
		static constexpr const uint8_t PC_PWM_TCCRA = 0;
		static constexpr const uint8_t PC_PWM_TCCRB = 0;
		static constexpr const uint8_t CTC_TCCRA  = 0;
		static constexpr const uint8_t CTC_TCCRB  = 0;

		static constexpr const uint8_t CS_MASK_TCCRB = 0;
		static constexpr const uint8_t MODE_MASK_TCCRA = 0;
		static constexpr const uint8_t MODE_MASK_TCCRB = 0;
		
		static constexpr const REG8 TCCRA{};
		static constexpr const REG8 TCCRB{};
		static constexpr const REGISTER<TYPE> TCNT{};
		static constexpr const REGISTER<TYPE> OCRA{};

		// 16 bits extended modes
		static constexpr const REGISTER<TYPE> ICR{};
		static constexpr const uint8_t CTC_ICR_TCCRA = 0;
		static constexpr const uint8_t CTC_ICR_TCCRB = 0;
		static constexpr const uint8_t F_PWM_ICR_TCCRA = 0;
		static constexpr const uint8_t F_PWM_ICR_TCCRB = 0;
		static constexpr const uint8_t PC_PWM_ICR_TCCRB = 0;
		static constexpr const uint8_t PC_PWM_ICR_TCCRA = 0;

		static constexpr const REG8 TIMSK_{};
		static constexpr const uint8_t TIMSK_MASK = 0xFF;
		static constexpr const REG8 TIFR_{};
		static constexpr uint8_t TCCRB_prescaler(TIMER_PRESCALER p)
		{
			return 0;
		}
		static constexpr uint8_t TIMSK_INT_MASK(uint8_t interrupt)
		{
			return 0;
		}

		// Input-capture stuff
		static constexpr const board::DigitalPin ICP_PIN = board::DigitalPin::NONE;
		static constexpr const uint8_t ICES_TCCRB = 0;
	};

	template<typename TYPE_, TimerPrescalers PRESCALERS_, 
			uint8_t COM_COUNT_,
			uint8_t MODE_MASK_TCCRA_, uint8_t MODE_MASK_TCCRB_, uint8_t CS_MASK_TCCRB_,
			uint8_t F_PWM_TCCRA_, uint8_t F_PWM_TCCRB_, 
			uint8_t PC_PWM_TCCRA_, uint8_t PC_PWM_TCCRB_, 
			uint8_t CTC_TCCRA_, uint8_t CTC_TCCRB_, 
			REG TCCRA_, REG TCCRB_, REG TCNT_, REG OCRA_, 
			REG TIMSK__, REG TIFR__, uint8_t TIMSK_MASK_ = 0xFF,
			REG ICR_ = 0, 
			uint8_t CTC_ICR_TCCRA_ = 0, uint8_t CTC_ICR_TCCRB_ = 0, 
			uint8_t F_PWM_ICR_TCCRA_ = 0, uint8_t F_PWM_ICR_TCCRB_ = 0, 
			uint8_t PC_PWM_ICR_TCCRA_ = 0, uint8_t PC_PWM_ICR_TCCRB_ = 0,
			board::DigitalPin ICP_PIN_ = board::DigitalPin::NONE,
			uint8_t ICES_TCCRB_ = 0>
	struct Timer_trait_impl
	{
		using TYPE = TYPE_;
		static constexpr const bool IS_16BITS = (sizeof(TYPE) > 1);
		static constexpr const uint32_t MAX_COUNTER = Timer_type_trait<TYPE>::MAX_COUNTER;
		static constexpr const uint16_t MAX_PWM = Timer_type_trait<TYPE>::MAX_PWM;

		static constexpr const TimerPrescalers PRESCALERS = PRESCALERS_;
		using PRESCALERS_TRAIT = TimerPrescalers_trait<PRESCALERS>;
		using TIMER_PRESCALER = typename PRESCALERS_TRAIT::TYPE;
		
		static constexpr const uint8_t COM_COUNT = COM_COUNT_;
		static constexpr const uint8_t COM_MASK = ~(0xFF >> (2 * COM_COUNT));

		static constexpr const uint8_t CS_MASK_TCCRB = CS_MASK_TCCRB_;
		static constexpr const uint8_t MODE_MASK_TCCRA = MODE_MASK_TCCRA_;
		static constexpr const uint8_t MODE_MASK_TCCRB = MODE_MASK_TCCRB_;

		static constexpr const uint8_t F_PWM_TCCRA = F_PWM_TCCRA_;
		static constexpr const uint8_t F_PWM_TCCRB = F_PWM_TCCRB_;
		static constexpr const uint8_t PC_PWM_TCCRA = PC_PWM_TCCRA_;
		static constexpr const uint8_t PC_PWM_TCCRB = PC_PWM_TCCRB_;
		static constexpr const uint8_t CTC_TCCRA  = CTC_TCCRA_;
		static constexpr const uint8_t CTC_TCCRB  = CTC_TCCRB_;
		
		static constexpr const uint8_t CTC_ICR_TCCRA = CTC_ICR_TCCRA_;
		static constexpr const uint8_t CTC_ICR_TCCRB = CTC_ICR_TCCRB_;
		static constexpr const uint8_t F_PWM_ICR_TCCRA = F_PWM_ICR_TCCRA_;
		static constexpr const uint8_t F_PWM_ICR_TCCRB = F_PWM_ICR_TCCRB_;
		static constexpr const uint8_t PC_PWM_ICR_TCCRA = PC_PWM_ICR_TCCRA_;
		static constexpr const uint8_t PC_PWM_ICR_TCCRB = PC_PWM_ICR_TCCRB_;

		static constexpr const REG8 TCCRA = TCCRA_;
		static constexpr const REG8 TCCRB = TCCRB_;
		static constexpr const REGISTER<TYPE> TCNT = TCNT_;
		static constexpr const REGISTER<TYPE> OCRA = OCRA_;
		static constexpr const REGISTER<TYPE> ICR = ICR_;
		
		static constexpr const REG8 TIMSK_ = TIMSK__;
		static constexpr const uint8_t TIMSK_MASK = TIMSK_MASK_;
		static constexpr const REG8 TIFR_ = TIFR__;

		static constexpr const board::DigitalPin ICP_PIN = ICP_PIN_;
		static constexpr const uint8_t ICES_TCCRB = ICES_TCCRB_;
	};
	
	template<DigitalPin PIN>
	struct PWMPin_trait
	{
		static constexpr const bool HAS_PWM = false;
		static constexpr const uint8_t COM = 0;
		static constexpr const Timer TIMER = Timer::TIMER0;
		using TIMER_TRAIT = Timer_trait<TIMER>;
		using TYPE = uint8_t;
	};
	template<Timer TIMER_, uint8_t COM_> 
	struct PWMPin_trait_impl
	{
		static constexpr const bool HAS_PWM = true;
		static constexpr const uint8_t COM = COM_;
		static constexpr const Timer TIMER = TIMER_;
		using TIMER_TRAIT = Timer_trait<TIMER>;
		using TYPE = typename TIMER_TRAIT::TYPE;
	};
};

namespace board
{
	template<DigitalPin PIN>
	constexpr uint8_t BIT() INLINE;
	template<DigitalPin PIN>
	constexpr uint8_t BIT()
	{
		return board_traits::DigitalPin_trait<PIN>::BIT;
	}
};	

#endif /* BOARDS_COMMON_TRAITS_HH */
