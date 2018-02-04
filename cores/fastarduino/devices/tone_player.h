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
		namespace SpecialTone
		{
			static constexpr const Tone END = Tone::USER0;
			static constexpr const Tone REPEAT_START = Tone::USER1;
			static constexpr const Tone REPEAT_END = Tone::USER2;
		}

		//NOTE prefer using QTonePlay as it is more code size and speed efficient
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
			class QTonePlay
			{
				using CALC = timer::Calculator<NTIMER>;
				using TIMER = timer::Timer<NTIMER>;
				using PRESCALER = typename TIMER::PRESCALER;
				using COUNTER = typename TIMER::TYPE;
				
			public:
				QTonePlay()
				{
				}
				constexpr QTonePlay(Tone t, uint16_t ms = 0)
					:flags_{flags(t)}, prescaler_{prescaler(t)}, counter_{counter(t)}, ms_{ms}
				{
				}

				inline PRESCALER prescaler() const
				{
					return prescaler_;
				}
				inline COUNTER counter() const
				{
					return counter_;
				}
				inline uint16_t duration() const
				{
					return ms_;
				}
				inline bool is_tone() const
				{
					return flags_ == TONE;
				}
				inline bool is_pause() const
				{
					return flags_ == NONE;
				}
				inline bool is_end() const
				{
					return flags_ == END;
				}
				inline bool is_repeat_start() const
				{
					return flags_ == REPEAT_START;
				}
				inline bool is_repeat_end() const
				{
					return flags_ == REPEAT_START;
				}
				inline uint16_t repeat_count() const
				{
					return ms_;
				}

			private:
				static constexpr uint8_t TONE = 0x00;
				static constexpr uint8_t NONE = 0x01;
				static constexpr uint8_t END = 0x02;
				static constexpr uint8_t REPEAT_START = 0x04;
				static constexpr uint8_t REPEAT_END = 0x08;
				
				// const uint8_t flags_;
				// const PRESCALER prescaler_;
				// const COUNTER counter_;
				// const uint16_t ms_;
				uint8_t flags_;
				PRESCALER prescaler_;
				COUNTER counter_;
				uint16_t ms_;

				static constexpr uint32_t period(Tone tone)
				{
					return 1000000UL / 2 / uint16_t(tone);
				}
				static constexpr PRESCALER prescaler(Tone tone)
				{
					return (tone > Tone::SILENCE ? CALC::CTC_prescaler(period(tone)) : PRESCALER::NO_PRESCALING);
				}
				static constexpr COUNTER counter(Tone tone)
				{
					return (tone > Tone::SILENCE ? CALC::CTC_counter(prescaler(tone), period(tone)) : 0);
				}
				static constexpr uint8_t flags(Tone tone)
				{
					return (tone == Tone::SILENCE ? NONE :
							tone == SpecialTone::END ? END :
							tone == SpecialTone::REPEAT_START ? REPEAT_START :
							tone == SpecialTone::REPEAT_END ? REPEAT_END :
							TONE);
				}
			};

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

			inline void play(const QTonePlay* melody)
			{
				play_(melody, load_sram);
			}
			inline void play_eeprom(const QTonePlay* melody)
			{
				play_(melody, load_eeprom);
			}
			inline void play_flash(const QTonePlay* melody)
			{
				play_(melody, load_flash);
			}

		private:
			using LOAD_TONE = const TonePlay* (*)(const TonePlay* address, TonePlay& holder);

			static const TonePlay* load_sram(const TonePlay* address, TonePlay& holder UNUSED)
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
					if (current->tone == SpecialTone::END)
						break;
					if (current->tone == SpecialTone::REPEAT_START)
					{
						repeat_play = play;
						repeat_times = -1;
					}
					else if (current->tone == SpecialTone::REPEAT_END)
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

			using LOAD_QTONE = const QTonePlay* (*)(const QTonePlay* address, QTonePlay& holder);

			static const QTonePlay* load_sram(const QTonePlay* address, QTonePlay& holder UNUSED)
			{
				return address;
			}
			static const QTonePlay* load_eeprom(const QTonePlay* address, QTonePlay& holder)
			{
				eeprom::EEPROM::read(address, holder);
				return &holder;
			}
			static const QTonePlay* load_flash(const QTonePlay* address, QTonePlay& holder)
			{
				flash::read_flash(address, holder);
				return &holder;
			}

			void play_(const QTonePlay* melody, LOAD_QTONE load_tone)
			{
				const QTonePlay* repeat_play = 0;
				int8_t repeat_times;
				const QTonePlay* play = melody;
				while (true)
				{
					QTonePlay holder;
					const QTonePlay* current = load_tone(play, holder);
					if (current->is_end())
						break;
					if (current->is_repeat_start())
					{
						repeat_play = play;
						repeat_times = -1;
					}
					else if (current->is_repeat_end())
					{
						if (repeat_play != 0)
						{
							if (repeat_times == -1)
								repeat_times = current->repeat_count();
							if (repeat_times--)
								play = repeat_play;
							else
								repeat_play = 0;
						}
					}
					else if (current->is_pause())
						generator_.pause(current->duration());
					else
						generator_.tone(current->prescaler(), current->counter(), current->duration());
					++play;
				}
			}

			GENERATOR& generator_;
		};
	}
}

#endif /* TONE_PLAYER_HH */
