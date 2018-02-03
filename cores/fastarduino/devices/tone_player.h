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

#ifndef TONE_PLAYER_HH
#define TONE_PLAYER_HH

#include "../eeprom.h"
#include "../flash.h"
#include "tones.h"

namespace devices
{
	namespace audio
	{
		//TODO improve constexpr, constructors...
		struct TonePlay
		{
			Tone tone;
			uint16_t ms;
		};

		template<board::Timer NTIMER, board::DigitalPin OUTPUT>
		class TonePlayer
		{
		private:
			using GENERATOR = ToneGenerator<NTIMER, OUTPUT>;
		
		public:
			TonePlayer(GENERATOR& tone_generator):generator_{tone_generator}
			{
			}

			inline void play(const TonePlay* melody)
			{
				play_(melody, load_sram);
			}
			inline void play_eeprom(const TonePlay* melody)
			{
				play_(melody, load_eeprom);
			}
			inline void play_flash(const TonePlay* melody)
			{
				play_(melody, load_flash);
			}

		private:
			using LOAD_TONE = const TonePlay* (*)(const TonePlay* address, TonePlay& holder);

			static const TonePlay* load_sram(const TonePlay* address, TonePlay& holder)
			{
				return address;
			}
			static const TonePlay* load_eeprom(const TonePlay* address, TonePlay& holder)
			{
				eeprom::EEPROM::read(address, holder);
				return &holder;
			}
			static const TonePlay* load_flash(const TonePlay* address, TonePlay& holder)
			{
				flash::read_flash(address, holder);
				return &holder;
			}

			void play_(const TonePlay* melody, LOAD_TONE load_tone)
			{
				const TonePlay* repeat_play = 0;
				int8_t repeat_times;
				const TonePlay* play = melody;
				while (true)
				{
					TonePlay holder;
					const TonePlay* current = load_tone(play, holder);
					if (current->tone == Tone::END)
						break;
					if (current->tone == Tone::REPEAT_START)
					{
						repeat_play = play;
						repeat_times = -1;
					}
					else if (current->tone == Tone::REPEAT_END)
					{
						if (repeat_play != 0)
						{
							if (repeat_times == -1)
								repeat_times = current->ms;
							if (repeat_times--)
								play = repeat_play;
							else
								repeat_play = 0;
						}
					}
					else
						generator_.tone(current->tone, current->ms);
					++play;
				}
			}

			GENERATOR& generator_;
		};
	}
}

#endif /* TONE_PLAYER_HH */
