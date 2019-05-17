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
 * API to handle tones (simple square waves) generation to a buzzer.
 */
#ifndef TONES_HH
#define TONES_HH

#include "../square_wave.h"
#include "../time.h"

/**
 * Defines API for audio tones (square waves) generation and simple melodies playing.
 */
namespace devices::audio
{
	/**
	 * This enum defines all possible audio tones that can be generated.
	 * This also defnes "special" values that are not actual tones but are 
	 * reserved for user purposes or for silences:
	 * - `USER0` ... `USER7`: can be used for any purpose defined by the end-developer
	 * - `SILENCE`: used to play no tone at all
	 * All other tones are named according to their musical note (in English scale),
	 * and their octave,. This is similar to the usual
	 * [Scientific Pitch Notation](https://en.wikipedia.org/wiki/Scientific_pitch_notation)
	 * except for the octave index which is different: in this enum, the standard
	 * tuning pitch (440Hz) is `A1` instead of `A4` in *SPN*.
	 * Please note that each tone can be converted to a `uint16_t` which is its
	 * playing frequency.
	 */
	enum class Tone : uint16_t
	{
		USER0 = 0,
		USER1,
		USER2,
		USER3,
		USER4,
		USER5,
		USER6,
		USER7,

		// Use this tone for pause (no tone)
		SILENCE = USER7 + 1,

		C0 = 131,
		Cs0 = 139,
		D0 = 147,
		Ds0 = 156,
		E0 = 165,
		F0 = 175,
		Fs0 = 185,
		G0 = 196,
		Gs0 = 208,
		A0 = 220,
		As0 = 233,
		B0 = 247,

		C1 = 262,
		Cs1 = 277,
		D1 = 294,
		Ds1 = 311,
		E1 = 330,
		F1 = 349,
		Fs1 = 370,
		G1 = 392,
		Gs1 = 415,
		A1 = 440,
		As1 = 466,
		B1 = 494,

		C2 = 523,
		Cs2 = 554,
		D2 = 587,
		Ds2 = 622,
		E2 = 659,
		F2 = 698,
		Fs2 = 740,
		G2 = 784,
		Gs2 = 831,
		A2 = 880,
		As2 = 932,
		B2 = 988,

		C3 = 1046,
		Cs3 = 1109,
		D3 = 1175,
		Ds3 = 1245,
		E3 = 1319,
		F3 = 1397,
		Fs3 = 1480,
		G3 = 1568,
		Gs3 = 1662,
		A3 = 1760,
		As3 = 1865,
		B3 = 1976,

		C4 = 2093,
		Cs4 = 2217,
		D4 = 2349,
		Ds4 = 2489,
		E4 = 2637,
		F4 = 2794,
		Fs4 = 2960,
		G4 = 3136,
		Gs4 = 3322,
		A4 = 3520,
		As4 = 3729,
		B4 = 3951,
	};

	/**
	 * API class for tone generation to a buzzer (or better an amplifier) connected
	 * to pin @p OUTPUT.
	 * This is a rather low-level API. If you want to play suites of tones ("melodies"),
	 * then you should better use `devices::audio::TonePlayer`.
	 * Most provided methods come in two flavours:
	 * - one that takes a `Tone` argument (that will be converted to a note frequency)
	 * - one that takes a TimerPrescaler and a counter value, that will be used to
	 * generate the note frequency from raw timer values.
	 * The first flavour is more readable but less efficient (code size and performance).
	 * The second flavour allows for performance optimization (compile-time calculations)
	 * but is less easy to read and understand.
	 * 
	 * @tparam NTIMER the AVR timer to use for the underlying Timer
	 * @tparam OUTPUT the PWMPin connected to the buzzer;
	 * this must be the pin OCnA, where n is the AVR Timer number
	 * 
	 * @sa Tone
	 * @sa TonePlayer
	 */
	template<board::Timer NTIMER, board::PWMPin OUTPUT> class ToneGenerator
	{
	private:
		using SQWGEN = timer::SquareWave<NTIMER, OUTPUT>;
		static constexpr const uint32_t INTERTONE_DELAY_MS = 20;

	public:
		/** The `TimerPrescaler` type matching the selected NTIMER. */
		using PRESCALER = typename SQWGEN::TIMER::PRESCALER;
		/** The counter type (`uint8_t` or `uint16_t`) for the selected NTIMER. */
		using COUNTER = typename SQWGEN::TIMER::TYPE;

		/**
		 * Create a new generator of tones.
		 */
		ToneGenerator() : generator_{} {}

		/**
		 * Start generating a tone on the connected buzzer until `stop_tone()`
		 * or `pause()` is called.
		 * If you would like to generate a tone for a given duration, you should
		 * use `tone()` instead.
		 * 
		 * @param t the tone to generate
		 * 
		 * @sa stop_tone()
		 * @sa pause()
		 * @sa tone(Tone, uint16_t)
		 * @sa start_tone(PRESCALER, COUNTER)
		 */
		void start_tone(Tone t)
		{
			if (t > Tone::SILENCE) generator_.start_frequency(uint32_t(t));
		}

		/**
		 * Start generating a tone on the connected buzzer until `stop_tone()`
		 * or `pause()` is called.
		 * If you would like to generate a tone for a given duration, you should
		 * use `tone()` instead.
		 * 
		 * @param prescaler the timer prescaler value to use to produce the required tone
		 * @param counter the timer counter value to use to produce the required tone
		 * 
		 * @sa stop_tone()
		 * @sa pause()
		 * @sa tone(PRESCALER, COUNTER, uint16_t)
		 * @sa start_tone(Tone)
		 */
		inline void start_tone(PRESCALER prescaler, COUNTER counter)
		{
			generator_.start_frequency(prescaler, counter);
		}

		/**
		 * Force a delay of the required duration and stop the tone being currently
		 * generated to the connected buzzer.
		 * Note that, after the note generation has been stopped, a short silence
		 * of 20ms is forced.
		 * 
		 * @param ms the time to wait, in milliseconds, until note generation is
		 * stopped
		 * 
		 * @sa start_tone()
		 */
		inline void pause(uint16_t ms)
		{
			time::delay_ms(ms);
			generator_.stop();
			// Short delay between tones
			time::delay_ms(INTERTONE_DELAY_MS);
		}

		/**
		 * Stop the tone being currently generated to the connected buzzer.
		 * @sa start_toner()
		 */
		inline void stop_tone()
		{
			generator_.stop();
		}

		/**
		 * Play the required tone for the given duration.
		 * Note that at the end of the tone, a short silence of 20ms is forced.
		 * 
		 * @param t the tone to play
		 * @param ms the duration, in milliseconds, of the tone to generate
		 * 
		 * @sa start_tone(Tone)
		 */
		void tone(Tone t, uint16_t ms)
		{
			if (t > Tone::SILENCE) generator_.start_frequency(uint32_t(t));
			if (t >= Tone::SILENCE) pause(ms);
		}
		
		/**
		 * Play the required tone for the given duration.
		 * Note that at the end of the tone, a short silence of 20ms is forced.
		 * 
		 * @param prescaler the timer prescaler value to use to produce the required tone
		 * @param counter the timer counter value to use to produce the required tone
		 * @param ms the duration, in milliseconds, of the tone to generate
		 * 
		 * @sa start_tone(PRESCALER, COUNTER)
		 */
		inline void tone(PRESCALER prescaler, COUNTER counter, uint16_t ms)
		{
			generator_.start_frequency(prescaler, counter);
			pause(ms);
		}
		
	private:
		SQWGEN generator_;
	};
}

#endif /* TONES_HH */
/// @endcond
