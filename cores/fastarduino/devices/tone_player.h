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
 * API to handle melody playing (tones as simple square waves) to a buzzer.
 */
#ifndef TONE_PLAYER_HH
#define TONE_PLAYER_HH

#include "../eeprom.h"
#include "../flash.h"
#include "tones.h"

namespace devices::audio
{
	/**
	 * This struct is the unit data manipulated by `TonePlayer`: it describes one
	 * `Tone` along with its duration in milliseconds.
	 * It can also hold special tones as defined in `SpecialTone` namespace.
	 * 
	 * Note that, although it is quite easy to define melodies with `TonePlay`,
	 * it is less efficient in terms of code size and performance (this is due to
	 * the need to convert tone frequencies to Timer prescaler and counter). It
	 * is advised, to use `TonePlayer::QTonePlay` optimized structure instead.
	 * 
	 * @sa Tone
	 * @sa TonePlayer
	 * @sa SpecialTone
	 * @sa TonePlayer::QTonePlay
	 */
	struct TonePlay
	{
		/**
		 * The tone for this `TonePlay`.
		 * This may be one of the special tones defined in `SpecialTone`. 
		 */
		Tone tone;
		/**
		 * The duration of this `TonePlay` in milliseconds.
		 * This may have another meaning when`tone` is a special tone.
		 */
		uint16_t ms;
	};

	/**
	 * This namespace defines special "tones" which have an impact on how to play
	 * a melody. They are used by `TonePlayer` and based on special `Tone`s.
	 * @sa TonePlayer
	 * @sa Tone
	 */
	namespace SpecialTone
	{
		/**
		 * This special tone marks the end of a melody (as a sequence of `Tone`s).
		 */
		static constexpr const Tone END = Tone::USER0;

		/**
		 * This special tone marks the beginning of sequence that shall be repeated
		 * later.
		 * @sa REPEAT_END
		 */
		static constexpr const Tone REPEAT_START = Tone::USER1;

		/**
		 * This special tone marks the end of a repeating sequence (started with
		 * `REPEAT_START`). The associated duration in `TonePlay` has a special
		 * meaning: it indicates the number of times to repeat the sequence.
		 * @sa REPEAT_START
		 * @sa TonePlay::ms
		 */
		static constexpr const Tone REPEAT_END = Tone::USER2;
	}

	//TODO allow stopping a play (from an ISR)
	//TODO allow asynchronous play (will need ISR registration)
	/**
	 * This API defines a player of melodies, defined as a sequence of tones and
	 * durations.
	 * 
	 * Melodies are defined as sequence of unit information, which can be either:
	 * - `TonePlay`s: easy to write in source code but not efficient in size of
	 * generated code
	 * - `TonePlayer::QTonePlay`s: requires more effort in source code, but 
	 * reduces generated code size
	 * Most methods exist in two flavours, one for each type.
	 * 
	 * With this API, played melodies can be stored on 3 possible locations:
	 * - in SRAM: this is useful when you get the melody from another support
	 * e.g. an external flash device
	 * - in Flash: this is the mostly used way as flash s the more abundant storage
	 * in AVR MCU
	 * - in EEPROM: this can be useful for short melodies, when you do not want
	 * to waste precious SRAM and Flash
	 * Each play API has 3 distinct methods, one for each storage strategy.
	 * 
	 * @tparam NTIMER the AVR timer to use for the underlying Timer
	 * @tparam OUTPUT the PWMPin connected to the buzzer;
	 * this must be the pin OCnA, where n is the AVR Timer number
	 * 
	 * @sa TonePlay
	 * @sa TonePlayer::QTonePlay
	 */
	template<board::Timer NTIMER, board::PWMPin OUTPUT> class TonePlayer
	{
	private:
		using GENERATOR = ToneGenerator<NTIMER, OUTPUT>;

	public:
		/**
		 * An optimized surrogate to `TonePlay` structure.
		 * Contrarily to `TonePlay`, it does not store `Tone` (i.e. frequency) but
		 * Timer prescaler and counter for the desired frequency.
		 * The advantage is that when you construct a `QTonePlay` from constant
		 * tones, each tone wll be converted to timer prescaler and counter at 
		 * compile-time, hence the generated code is smaller and more efficient;
		 * this can be useful when your MCU is limited in data size.
		 */
		class QTonePlay
		{
			using CALC = timer::Calculator<NTIMER>;
			using TIMER = timer::Timer<NTIMER>;
			using PRESCALER = typename TIMER::PRESCALER;
			using COUNTER = typename TIMER::TYPE;

		public:
			/**
			 * Default constructor, used only to declare an uninitialized
			 * `QTonePlay` variable.
			 * You should always ensure you replace such a variable with one
			 * constructed with the next constructor.
			 * @sa QTonePlay(Tone, uint16_t)
			 */
			QTonePlay() {}

			/**
			 * Construct an optimized tone play for the provided tone and duration.
			 * 
			 * @param t the `Tone` for this `QTonePlay`; it will be automatically
			 * converted (at compile-time if @p t is a constant) to the proper
			 * timer prescaler and counter.
			 * @param ms the duration of this tone in milliseconds; this may have
			 * different meanings if @p t is a `SpecialTone`.
			 */
			constexpr QTonePlay(Tone t, uint16_t ms = 0)
				: flags_{flags(t)}, prescaler_{prescaler(t)}, counter_{counter(t)}, ms_{ms}
			{}

		private:
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
				return flags_ == REPEAT_END;
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
						tone == SpecialTone::REPEAT_END ? REPEAT_END : TONE);
			}

			friend class TonePlayer<NTIMER, OUTPUT>;
		};

		/**
		 * Create a new tone player, based on an existing `ToneGenerator`.
		 * @param tone_generator the `ToneGenerator` used to actually produce
		 * tones.
		 */
		TonePlayer(GENERATOR& tone_generator) : generator_{tone_generator} {}

		/**
		 * Play a melody, defined by a sequence of `TonePlay`s, stored in SRAM.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in SRAM, to the sequence of `TonePlay` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 */
		inline void play(const TonePlay* melody)
		{
			play_(melody, load_sram);
		}

		/**
		 * Play a melody, defined by a sequence of `TonePlay`s, stored in EEPROM.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in EEPROM, to the sequence of `TonePlay` to 
		 * be played; the sequence MUST finish with a `SpecialTone::END`.
		 */
		inline void play_eeprom(const TonePlay* melody)
		{
			play_(melody, load_eeprom);
		}

		/**
		 * Play a melody, defined by a sequence of `TonePlay`s, stored in Flash.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in Flash, to the sequence of `TonePlay` to 
		 * be played; the sequence MUST finish with a `SpecialTone::END`.
		 */
		inline void play_flash(const TonePlay* melody)
		{
			play_(melody, load_flash);
		}

		/**
		 * Play a melody, defined by a sequence of `QTonePlay`s, stored in SRAM.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in SRAM, to the sequence of `QTonePlay` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 */
		inline void play(const QTonePlay* melody)
		{
			play_(melody, load_sram);
		}

		/**
		 * Play a melody, defined by a sequence of `QTonePlay`s, stored in EEPROM.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in EEPROM, to the sequence of `QTonePlay` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 */
		inline void play_eeprom(const QTonePlay* melody)
		{
			play_(melody, load_eeprom);
		}

		/**
		 * Play a melody, defined by a sequence of `QTonePlay`s, stored in Flash.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in Flash, to the sequence of `QTonePlay` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 */
		inline void play_flash(const QTonePlay* melody)
		{
			play_(melody, load_flash);
		}

	private:
		using LOAD_TONE = const TonePlay* (*) (const TonePlay* address, TonePlay& holder);

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
				if (current->tone == SpecialTone::END) break;
				if (current->tone == SpecialTone::REPEAT_START)
				{
					repeat_play = play;
					repeat_times = -1;
				}
				else if (current->tone == SpecialTone::REPEAT_END)
				{
					if (repeat_play != 0)
					{
						if (repeat_times == -1) repeat_times = current->ms;
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

		using LOAD_QTONE = const QTonePlay* (*) (const QTonePlay* address, QTonePlay& holder);

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
				if (current->is_end()) break;
				if (current->is_repeat_start())
				{
					repeat_play = play;
					repeat_times = -1;
				}
				else if (current->is_repeat_end())
				{
					if (repeat_play != 0)
					{
						if (repeat_times == -1) repeat_times = current->repeat_count();
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

#endif /* TONE_PLAYER_HH */
/// @endcond
