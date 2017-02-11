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

#ifndef ANALOGINPUT_HH
#define ANALOGINPUT_HH

#include "boards/board_traits.h"
#include "time.h"

//TODO LATER: add class (or namespace?) with general methods enable/disable ADC...
//TODO Do we need an Abstract class (no template) to perform all work?
//TODO Shouldn't SAMPLE_TYPE be a template parameter of method sample()
template<	Board::AnalogPin APIN, 
			Board::AnalogReference AREF = Board::AnalogReference::AVCC, 
			typename SAMPLE_TYPE = uint16_t,
			Board::AnalogClock MAXFREQ = Board::AnalogClock::MAX_FREQ_200KHz>
class AnalogInput
{
private:
	using TRAIT = board_traits::AnalogPin_trait<APIN>;
	using GLOBAL_TRAIT = board_traits::GlobalAnalogPin_trait;
	using VREF_TRAIT = board_traits::AnalogReference_trait<AREF>;
	using TYPE_TRAIT = board_traits::AnalogSampleType_trait<SAMPLE_TYPE>;
	using FREQ_TRAIT = board_traits::AnalogClock_trait<MAXFREQ>;
	
public:
	using TYPE = SAMPLE_TYPE;
	static constexpr const uint8_t PRESCALER = FREQ_TRAIT::PRESCALER;
	static constexpr const uint16_t BG_STABILIZATION_DELAY_US = 400;
	
	SAMPLE_TYPE sample()
	{
		// First ensure that any pending sampling is finished
		GLOBAL_TRAIT::ADCSRA_.loop_until_bit_clear(ADSC);
		// Setup multiplexer selection and start conversion
		GLOBAL_TRAIT::ADMUX_ = VREF_TRAIT::MASK | TYPE_TRAIT::ADLAR1 | TRAIT::MUX_MASK1;
		GLOBAL_TRAIT::ADCSRB_ = TRAIT::MUX_MASK2 | TYPE_TRAIT::ADLAR2;

		// The following delay is necessary for bandgap ADC, strangely 70us should be enough (datasheet)
		// but this works only when no other ADC is used "at the same time"
		// In this situation, a delay of minimum 400us seems necessary to ensure bandgap reference voltage is stabilized
		if (TRAIT::IS_BANDGAP)
			Time::delay_us(BG_STABILIZATION_DELAY_US);
		
		GLOBAL_TRAIT::ADCSRA_ = _BV(ADEN) | _BV(ADSC) | TRAIT::MUX_MASK2 | FREQ_TRAIT::PRESCALER_MASK;
		// Wait until sampling is done
		GLOBAL_TRAIT::ADCSRA_.loop_until_bit_clear(ADSC);
		// Should we synchronize ADC reading?
		return TYPE_TRAIT::_ADC;
	}
};

template<Board::AnalogPin BG=Board::AnalogPin::BANDGAP>
class PowerVoltage: public AnalogInput<	BG, 
										Board::AnalogReference::AVCC, 
										uint16_t, 
										Board::AnalogClock::MAX_FREQ_50KHz>
{
private:
	using TRAIT = board_traits::AnalogPin_trait<BG>;
	static_assert(TRAIT::IS_BANDGAP, "BG parameter must be a bandgap ADC input");
	static constexpr const uint16_t REFERENCE_MV = TRAIT::BANDGAP_VOLTAGE_MV;
	
public:
	uint16_t voltage_mV()
	{
		// Get sample
		uint16_t rate = this-> template sample();
		// Do the maths to find out Vcc from rate:
		return REFERENCE_MV * 1024L / rate;
	}
};

//TODO LATER: need ISR also
// Find better class name
//TODO Implement mechanism to ensure only DeferredAnalogInput can be registered (need sth like one AnalogInputManager?))
/*
template<Board::AnalogPin APIN, Board::AnalogReference AREF = Board::AnalogReference::AVCC, typename SAMPLE_TYPE = uint16_t>
class DeferredAnalogInput
{
public:
	DeferredAnalogInput();
	void start();
	void stop();
	SAMPLE_TYPE sample();
	//Other API? eg sample_ready? sample_changed?
	
private:
	SAMPLE_TYPE _sample;
};
*/

#endif /* ANALOGINPUT_HH */
