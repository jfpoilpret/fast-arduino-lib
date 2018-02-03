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

#ifndef TONES_HH
#define TONES_HH

#include "square_wave.h"
#include "time.h"

namespace devices
{
	namespace audio
	{
		//TODO reserve a set of codes between END and NONE and then define constants for REAPEAT_START...
		// so that the current implementation is open enough and not specific to a future TonePlayer class.
		enum class Tone: uint16_t
		{
			// Special marker for a tone sequence, meaning there is no more tone in the sequence
			// This is useful when you don't know the sequence size in advance
			END = 0,

			// Use this "tone" to mark the beginning of a sequence that shall be repeated when REPEAT_END is encountered
			REPEAT_START = 1,
			// Use this "tone" to mark the end of a sequence to repeat from REPEAT_START
			// In TonePlay, ms then contains the number of times to repeat the sequence
			REPEAT_END = 2,

			// Use this tone for pause (no tone)
			NONE = 3,

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
		
		template<board::Timer NTIMER, board::DigitalPin OUTPUT>
		class ToneGenerator
		{
		private:
			using SQWGEN = timer::SquareWave<NTIMER, OUTPUT>;
			static constexpr const uint32_t INTERTONE_DELAY_MS = 20;

		public:
			using PRESCALER = typename SQWGEN::TIMER::PRESCALER;
			using COUNTER = typename SQWGEN::TIMER::TYPE;
			
			ToneGenerator():generator_{}
			{
			}

			//TODO complete API
			// - overloaded methods with specialized arguments prescaler/counter to optimize code size
			void start_tone(Tone t)
			{
				if (t > Tone::END)
					generator_.start_frequency(uint32_t(t));
			}
			inline void start_tone(PRESCALER prescaler, COUNTER counter)
			{
				generator_.start_frequency(prescaler, counter);
			}
			inline void stop_tone()
			{
				generator_.stop();
			}

			void tone(Tone t, uint16_t ms)
			{
				if (t > Tone::NONE)
					generator_.start_frequency(uint32_t(t));
				if (t >= Tone::NONE)
					pause(ms);
			}
			inline void tone(PRESCALER prescaler, COUNTER counter, uint16_t ms)
			{
				generator_.start_frequency(prescaler, counter);
				pause(ms);
			}
			inline void pause(uint16_t ms)
			{
				time::delay_ms(ms);
				generator_.stop();
				// Short delay between tones
				time::delay_ms(INTERTONE_DELAY_MS);
			}

		private:
			SQWGEN generator_;
		};

	}
}

#endif /* TONES_HH */
