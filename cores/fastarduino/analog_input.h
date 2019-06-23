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

/// @cond api

/**
 * @file 
 * Analog Input API.
 */
#ifndef ANALOGINPUT_HH
#define ANALOGINPUT_HH

#include "boards/board_traits.h"
#include "time.h"

/**
 * Defines all API to manipulate analog input/output.
 */
namespace analog
{
	//TODO LATER: add class (or namespace?) with general methods enable/disable ADC...

	/**
	 * API that handles a given analog input pin of the target MCU.
	 * 
	 * @tparam APIN_ a unique analog pin for the MCU target; this may also not be
	 * a real pin but an internal sensor (e.g. temperature or bandgap).
	 * @tparam AREF_ the analog reference to use for that input
	 * @tparam SAMPLE_TYPE_ the type of samples, either `uint8_t` (8 bits) or
	 * `uint16_t` (10 bits)
	 * @tparam MAXFREQ_ the maximum input clock frequency of the ADC circuit; higher
	 * frequencies imply lower precision of samples.
	 * @sa board::AnalogPin
	 * @sa board::AnalogReference
	 * @sa board::AnalogClock
	 */
	template<board::AnalogPin APIN_, typename SAMPLE_TYPE_ = uint16_t,
			 board::AnalogReference AREF_ = board::AnalogReference::AVCC,
			 board::AnalogClock MAXFREQ_ = board::AnalogClock::MAX_FREQ_200KHz>
	class AnalogInput
	{
	public:
		/**
		 * The analog pin for this AnalogInput; this may also not be
		 * a real pin but an internal sensor (e.g. temperature or bandgap).
		 */
		static constexpr const board::AnalogPin APIN = APIN_;
		/**
		 * The analog reference to use for that analog input.
		 */
		static constexpr const board::AnalogReference AREF = AREF_;
		/**
		 * The type of samples returned by `sample()`.
		 */
		using SAMPLE_TYPE = SAMPLE_TYPE_;
		/**
		 * The maximum input clock frequency of the ADC circuit; higher
		 * frequencies imply lower precision of samples.
		 */
		static constexpr const board::AnalogClock MAXFREQ = MAXFREQ_;

	private:
		using TRAIT = board_traits::AnalogPin_trait<APIN>;
		using GLOBAL_TRAIT = board_traits::GlobalAnalogPin_trait;
		using AREF_TRAIT = board_traits::AnalogReference_trait<AREF>;
		using TYPE_TRAIT = board_traits::AnalogSampleType_trait<SAMPLE_TYPE>;
		using FREQ_TRAIT = board_traits::AnalogClock_trait<MAXFREQ>;

		static constexpr const uint16_t BG_STABILIZATION_DELAY_US = 400;

	public:
		/**
		 * The type of samples returned by `sample()`.
		 */
		using TYPE = SAMPLE_TYPE;

		/**
		 * The prescaler used by ADC circuitry, calculated from `MAXFREQ` template
		 * parameter.
		 */
		static constexpr const uint8_t PRESCALER = FREQ_TRAIT::PRESCALER;

		/**
		 * Start an analog-digital conversion for this analog input pin and return
		 * sample value.
		 * 
		 * @return the sample value converted from this analog input pin
		 */
		SAMPLE_TYPE sample()
		{
			// First ensure that any pending sampling is finished
			GLOBAL_TRAIT::ADCSRA_.loop_until_bit_clear(ADSC);
			// Setup multiplexer selection and start conversion
			GLOBAL_TRAIT::ADMUX_ = AREF_TRAIT::MASK | TYPE_TRAIT::ADLAR1 | TRAIT::MUX_MASK1;
			GLOBAL_TRAIT::ADCSRB_ = TRAIT::MUX_MASK2 | TYPE_TRAIT::ADLAR2;

			// The following delay is necessary for bandgap ADC, strangely 70us should be enough (datasheet)
			// but this works only when no other ADC is used "at the same time"
			// In this situation, a delay of minimum 400us seems necessary to ensure bandgap reference voltage is stabilized
			if (TRAIT::IS_BANDGAP) time::delay_us(BG_STABILIZATION_DELAY_US);

			GLOBAL_TRAIT::ADCSRA_ = BV8(ADEN) | BV8(ADSC) | TRAIT::MUX_MASK2 | FREQ_TRAIT::PRESCALER_MASK;
			// Wait until sampling is done
			GLOBAL_TRAIT::ADCSRA_.loop_until_bit_clear(ADSC);
			// Should we synchronize ADC reading?
			return TYPE_TRAIT::ADC_;
		}
	};

	/**
	 * API that uses bandgap feature to calculate current voltage fed to the MCU.
	 * 
	 * @tparam BANDGAP_ a unique bandgap analog pin for the MCU target; although the
	 * accepted type is `board::AnalogPin` enum, only bandgap inputs are allowed
	 * otherwise a compilation error will occur.
	 * @sa board::AnalogPin
	 */
	template<board::AnalogPin BANDGAP_ = board::AnalogPin::BANDGAP>
	class PowerVoltage
		: public AnalogInput<BANDGAP_, uint16_t, board::AnalogReference::AVCC, board::AnalogClock::MAX_FREQ_50KHz>
	{
	public:
		/**
		 * The bandgap analog pin of the MCU target, used for this `PowerVoltage`.
		 */
		static constexpr const board::AnalogPin BANDGAP = BANDGAP_;

	private:
		using TRAIT = board_traits::AnalogPin_trait<BANDGAP>;
		static_assert(TRAIT::IS_BANDGAP, "BANDGAP_ parameter must be a bandgap ADC input");
		static constexpr const uint16_t REFERENCE_MV = TRAIT::BANDGAP_VOLTAGE_MV;

	public:
		/**
		 * Get the voltage, in mV, feeding the MCU.
		 * 
		 * @return the current voltage in mV
		 */
		uint16_t voltage_mV()
		{
			// Get sample
			uint16_t rate = this->template sample();
			// Do the maths to find out Vcc from rate:
			return REFERENCE_MV * 1024L / rate;
		}
	};

	//TODO LATER: need ISR also
	// Find better class name
	//TODO Implement mechanism to ensure only DeferredAnalogInput can be registered (need sth like one AnalogInputManager?))
	// template<board::AnalogPin APIN, board::AnalogReference AREF = board::AnalogReference::AVCC, typename SAMPLE_TYPE = uint16_t>
	// class DeferredAnalogInput
	// {
	// public:
	// 	DeferredAnalogInput();
	// 	void start();
	// 	void stop();
	// 	SAMPLE_TYPE sample();
	// 	//Other API? eg sample_ready? sample_changed?

	// private:
	// 	SAMPLE_TYPE sample_;
	// };
}

#endif /* ANALOGINPUT_HH */
/// @endcond
