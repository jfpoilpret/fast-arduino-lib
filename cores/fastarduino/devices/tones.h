//   Copyright 2016-2021 Jean-Francois Poilpret
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

namespace devices
{
	/**
	 * Defines API for audio tones (square waves) generation and simple melodies playing.
	 */
	namespace audio
	{
	}
}

namespace devices::audio
{
	/**
	 * This enum defines all possible audio tones that can be generated.
	 * This also defnes "special" values that are not actual tones but are 
	 * reserved for user purposes or for silences:
	 * - `USER0` ... `USER7`: can be used for any purpose defined by the end-developer
	 * - `SILENCE` or `REST`: used to play no tone at all
	 * All other tones are named according to their musical note (in English scale),
	 * and their octave,. This is similar to the usual
	 * [Scientific Pitch Notation](https://en.wikipedia.org/wiki/Scientific_pitch_notation)
	 * except for the octave index which is different: in this enum, the standard
	 * tuning pitch (440Hz) is `A1` instead of `A4` in *SPN*.
	 * 
	 * Sharps are noted as `s` between the note and its octave, as in `Cs0`.
	 * Flats are noted as `f` between the note and its octave, as in `Df0`.
	 * 
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

		// Use this tone for rest (no tone)
		SILENCE = USER7 + 1,
		REST = SILENCE,

		C0 = 131,
		Cs0 = 139,
		Df0 = Cs0,
		D0 = 147,
		Ds0 = 156,
		Ef0 = Ds0,
		E0 = 165,
		F0 = 175,
		Fs0 = 185,
		Gf0 = Fs0,
		G0 = 196,
		Gs0 = 208,
		Af0 = Gs0,
		A0 = 220,
		As0 = 233,
		Bf0 = As0,
		B0 = 247,

		C1 = 262,
		Cs1 = 277,
		Df1 = Cs1,
		D1 = 294,
		Ds1 = 311,
		Ef1 = Ds1,
		E1 = 330,
		F1 = 349,
		Fs1 = 370,
		Gf1 = Fs1,
		G1 = 392,
		Gs1 = 415,
		Af1 = Gs1,
		A1 = 440,
		As1 = 466,
		Bf1 = As1,
		B1 = 494,

		C2 = 523,
		Cs2 = 554,
		Df2 = Cs2,
		D2 = 587,
		Ds2 = 622,
		Ef2 = Ds2,
		E2 = 659,
		F2 = 698,
		Fs2 = 740,
		Gf2 = Fs2,
		G2 = 784,
		Gs2 = 831,
		Af2 = Gs2,
		A2 = 880,
		As2 = 932,
		Bf2 = As2,
		B2 = 988,

		C3 = 1046,
		Cs3 = 1109,
		Df3 = Cs3,
		D3 = 1175,
		Ds3 = 1245,
		Ef3 = Ds3,
		E3 = 1319,
		F3 = 1397,
		Fs3 = 1480,
		Gf3 = Fs3,
		G3 = 1568,
		Gs3 = 1662,
		Af3 = Gs3,
		A3 = 1760,
		As3 = 1865,
		Bf3 = As3,
		B3 = 1976,

		C4 = 2093,
		Cs4 = 2217,
		Df4 = Cs4,
		D4 = 2349,
		Ds4 = 2489,
		Ef4 = Ds4,
		E4 = 2637,
		F4 = 2794,
		Fs4 = 2960,
		Gf4 = Fs4,
		G4 = 3136,
		Gs4 = 3322,
		Af4 = Gs4,
		A4 = 3520,
		As4 = 3729,
		Bf4 = As4,
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
	 * @tparam OUTPUT the `board::PWMPin` connected to the buzzer;
	 * this must be the pin OCnA, where n is the AVR Timer number
	 * 
	 * @sa Tone
	 * @sa TonePlayer
	 */
	template<board::Timer NTIMER, board::PWMPin OUTPUT> class ToneGenerator
	{
	private:
		using SQWGEN = timer::SquareWave<NTIMER, OUTPUT>;

	public:
		ToneGenerator(const ToneGenerator<NTIMER, OUTPUT>&) = delete;
		ToneGenerator<NTIMER, OUTPUT>& operator=(const ToneGenerator<NTIMER, OUTPUT>&) = delete;
		
		/** The `TimerPrescaler` type matching the selected NTIMER. */
		using PRESCALER = typename SQWGEN::TIMER::PRESCALER;
		/** The counter type (`uint8_t` or `uint16_t`) for the selected NTIMER. */
		using COUNTER = typename SQWGEN::TIMER::TYPE;

		/**
		 * Create a new generator of tones.
		 */
		ToneGenerator() = default;

		/**
		 * Start generating a tone on the connected buzzer until `stop_tone()`
		 * is called.
		 * If you would like to generate a tone for a given duration, you should
		 * use `tone()` instead.
		 * 
		 * @param tone the tone to generate
		 * 
		 * @sa stop_tone()
		 * @sa tone(Tone)
		 * @sa start_tone(PRESCALER, COUNTER)
		 */
		void start_tone(Tone tone)
		{
			if (tone > Tone::SILENCE) generator_.start_frequency(uint32_t(tone));
		}

		/**
		 * Start generating a tone on the connected buzzer until `stop_tone()`
		 * is called.
		 * If you would like to generate a tone for a given duration, you should
		 * use `tone()` instead.
		 * 
		 * @param prescaler the timer prescaler value to use to produce the required tone
		 * @param counter the timer counter value to use to produce the required tone
		 * 
		 * @sa stop_tone()
		 * @sa tone(PRESCALER, COUNTER)
		 * @sa start_tone(Tone)
		 */
		void start_tone(PRESCALER prescaler, COUNTER counter)
		{
			generator_.start_frequency(prescaler, counter);
		}

		/**
		 * Stop the tone being currently generated to the connected buzzer.
		 * @sa start_toner()
		 */
		void stop_tone()
		{
			generator_.stop();
		}

	private:
		SQWGEN generator_;
	};
}

#endif /* TONES_HH */
/// @endcond
