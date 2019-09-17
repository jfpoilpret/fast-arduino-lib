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

//TODO improve duration config by using musical notes durations (whole, half, quarter...)
// along with a tempo provided at runtime (API).
//TODO support slurs and ties (=> no internote delay)

namespace devices::audio
{
	// Forward declaration
	template<board::Timer NTIMER, board::PWMPin OUTPUT, typename TONEPLAY> class AbstractTonePlayer;

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

	//TODO DOCS

	// The uint8_t is the multiplier to apply to the duration of a 32th note
	// to get the actual duration (in 4 quarters mode)
	// The MSB is special and means "dotted" i.e. "add one half to that duration"
	enum class Duration : uint8_t
	{
		// Common names for notes durations
		WHOLE = 32,
		HALF = 16,
		QUARTER = 8,
		EIGHTH = 4,
		SIXTEENTH = 2,

		// Synonyms for some notes durations
		SEMI_BREVE = WHOLE,
		MINIM = HALF,
		CROTCHET = QUARTER,
		QUAVER = EIGHTH,
		SEMI_QUAVER = SIXTEENTH,
	};

	static constexpr Duration dotted(Duration d)
	{
		return Duration(uint8_t(d) + uint8_t(d) / 2);
	}

	static constexpr Duration triplet(Duration d)
	{
		return Duration(uint8_t(d) * 2 / 3);
	}

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
	 * @sa QTonePlay
	 */
	class TonePlay
	{
	public:
		/**
		 * Default constructor, used only to declare an uninitialized
		 * `TonePlay` variable.
		 * You should always ensure you replace such a variable with one
		 * constructed with the next constructor.
		 * @sa TonePlay(Tone, uint16_t)
		 */
		TonePlay() = default;

		//TODO DOC update
		/**
		 * Construct a tone play with the provided tone and duration.
		 * 
		 * @param tone the `Tone` for this `TonePlay`; it will be automatically
		 * converted (at compile-time if @p t is a constant) to the proper
		 * timer prescaler and counter.
		 * @param ms the duration of this tone in milliseconds; this may have
		 * different meanings if @p t is a `SpecialTone`.
		 */
		constexpr TonePlay(Tone tone, Duration duration) : tone_{tone}, duration_{duration} {}

		//TODO DOC
		constexpr TonePlay(Tone tone, uint8_t repeats = 0) : tone_{tone}, repeats_{repeats} {}

	private:
		Duration duration() const
		{
			return duration_;
		}
		bool is_tone() const
		{
			return tone_ > Tone::SILENCE;
		}
		bool is_pause() const
		{
			return tone_ == Tone::SILENCE;
		}
		bool is_end() const
		{
			return tone_ == SpecialTone::END;
		}
		bool is_repeat_start() const
		{
			return tone_ == SpecialTone::REPEAT_START;
		}
		bool is_repeat_end() const
		{
			return tone_ == SpecialTone::REPEAT_END;
		}
		uint8_t repeat_count() const
		{
			return repeats_;
		}

		template<board::Timer NTIMER, board::PWMPin OUTPUT>
		void generate_tone(ToneGenerator<NTIMER, OUTPUT>& generator) const
		{
			generator.start_tone(tone_);
		}

		Tone tone_;
		union
		{
			Duration duration_;
			uint8_t repeats_;
		};

		template<board::Timer, board::PWMPin, typename> friend class AbstractTonePlayer;
	};

	/**
	 * An optimized surrogate to `TonePlay` structure.
	 * Contrarily to `TonePlay`, it does not store `Tone` (i.e. frequency) but
	 * Timer prescaler and counter for the desired frequency.
	 * The advantage is that when you construct a `QTonePlay` from constant
	 * tones, each tone wll be converted to timer prescaler and counter at 
	 * compile-time, hence the generated code is smaller and more efficient;
	 * this can be useful when your MCU is limited in data size.
	 * @sa TonePlay
	 */
	template<board::Timer NTIMER, board::PWMPin OUTPUT> class QTonePlay
	{
	public:
		/**
		 * Default constructor, used only to declare an uninitialized
		 * `QTonePlay` variable.
		 * You should always ensure you replace such a variable with one
		 * constructed with the next constructor.
		 * @sa QTonePlay(Tone, uint16_t)
		 */
		QTonePlay() = default;

		//TODO DOC update
		/**
		 * Construct an optimized tone play for the provided tone and duration.
		 * 
		 * @param tone the `Tone` for this `QTonePlay`; it will be automatically
		 * converted (at compile-time if @p t is a constant) to the proper
		 * timer prescaler and counter.
		 * @param ms the duration of this tone in milliseconds; this may have
		 * different meanings if @p t is a `SpecialTone`.
		 */
		constexpr QTonePlay(Tone tone, Duration duration)
			: flags_{flags(tone)}, prescaler_{prescaler(tone)}, counter_{counter(tone)}, duration_{duration} {}

		//TODO DOC
		constexpr QTonePlay(Tone tone, uint8_t repeats = 0)
			: flags_{flags(tone)}, prescaler_{prescaler(tone)}, counter_{counter(tone)}, repeats_{repeats} {}

	private:
		using CALC = timer::Calculator<NTIMER>;
		using TIMER = timer::Timer<NTIMER>;
		using PRESCALER = typename TIMER::PRESCALER;
		using COUNTER = typename TIMER::TYPE;

		Duration duration() const
		{
			return duration_;
		}
		bool is_tone() const
		{
			return flags_ == TONE;
		}
		bool is_pause() const
		{
			return flags_ == NONE;
		}
		bool is_end() const
		{
			return flags_ == END;
		}
		bool is_repeat_start() const
		{
			return flags_ == REPEAT_START;
		}
		bool is_repeat_end() const
		{
			return flags_ == REPEAT_END;
		}
		uint8_t repeat_count() const
		{
			return repeats_;
		}

		void generate_tone(ToneGenerator<NTIMER, OUTPUT>& generator) const
		{
			generator.start_tone(prescaler_, counter_);
		}

		// Flags meaning
		static constexpr uint8_t TONE = 0x00;
		static constexpr uint8_t NONE = 0x01;
		static constexpr uint8_t END = 0x02;
		static constexpr uint8_t REPEAT_START = 0x04;
		static constexpr uint8_t REPEAT_END = 0x08;

		uint8_t flags_;
		PRESCALER prescaler_;
		COUNTER counter_;
		union
		{
			Duration duration_;
			uint8_t repeats_;
		};

		static constexpr uint32_t period(Tone tone)
		{
			return ONE_SECOND / 2 / uint16_t(tone);
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
			if (tone == Tone::SILENCE) return NONE;
			if (tone == SpecialTone::END) return END;
			if (tone == SpecialTone::REPEAT_START) return REPEAT_START;
			if (tone == SpecialTone::REPEAT_END) return REPEAT_END;
			return TONE;
		}

		friend class AbstractTonePlayer<NTIMER, OUTPUT, QTonePlay<NTIMER, OUTPUT>>;
	};

	/**
	 * This low-level API defines an abstract player of melodies (defined as a
	 * sequence of tones and durations).
	 * You should normally not need to use it directly in programs, but rather
	 * use specific implementations instead:
	 * - `TonePlay`: a simple player, playing melodies in a synchronous way
	 * (blocking until the whole melody is played until end)
	 * - `AsyncTonePlay`: a player that can play melodies asynchronously, when
	 * used with a Timer ISR.
	 * 
	 * Melodies are defined as sequence of unit information, which can be either:
	 * - `TonePlay`s: easy to write in source code but not efficient in size of
	 * generated code
	 * - `QTonePlay`s: requires more effort in source code, but reduces generated
	 * code size
	 * Which types is used is defined as the @p TONEPLAY template parameter.
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
	 * @tparam OUTPUT the `board::PWMPin` connected to the buzzer;
	 * this must be the pin OCnA, where n is the AVR Timer number
	 * @tparam TONEPLAY the type used to store melody data, `QTonePlay` by default
	 * 
	 * @sa TonePlay
	 * @sa QTonePlay
	 * @sa TonePlayer
	 * @sa AsyncTonePlayer
	 */
	template<board::Timer NTIMER, board::PWMPin OUTPUT, typename TONEPLAY = QTonePlay<NTIMER, OUTPUT>>
	class AbstractTonePlayer
	{
	protected:
		/** The type that holds unit of information of a melody. */
		using TONE_PLAY = TONEPLAY;
		/** The type of `ToneGenerator` to use as constructor's argument. */
		using GENERATOR = ToneGenerator<NTIMER, OUTPUT>;

		/**
		 * Create a new tone player, based on an existing `ToneGenerator`.
		 * @param tone_generator the `ToneGenerator` used to actually produce
		 * tones.
		 */
		explicit AbstractTonePlayer(GENERATOR& tone_generator)
		: generator_{tone_generator}, loader_{}, t32_duration_ms_{}, 
		  current_play_{}, repeat_play_{}, repeat_times_{}, no_delay_{} {}

		//TODO DOC
		//TODO improve by providing a time signature?
		static constexpr uint16_t calculate_min_duration(uint8_t bpm)
		{
			// bpm defines the duration of a quarter note (in 4/4 mode)
			// We want the minimum duration allowed (for a 32nd note, since we allow dotted sixteenth)
			return (60U * 1000U / 8U) / bpm;
		}

		//TODO DOC
		void set_min_duration(uint16_t min_duration)
		{
			t32_duration_ms_ = min_duration;
		}

		//TODO DOC
		uint16_t get_min_duration() const
		{
			return t32_duration_ms_;
		}

		/**
		 * Prepare playing of @p melody, which should be stored in SRAM.
		 * Once preparation is done, actual melody playing is performed by sequenced
		 * calls to `start_next_note()` and `stop_current_note()`.
		 */
		void prepare_sram(const TONE_PLAY* melody)
		{
			prepare_(melody, load_sram);
		}

		/**
		 * Prepare playing of @p melody, which should be stored in EEPROM.
		 * Once preparation is done, actual melody playing is performed by sequenced
		 * calls to `start_next_note()` and `stop_current_note()`.
		 */
		void prepare_eeprom(const TONE_PLAY* melody)
		{
			prepare_(melody, load_eeprom);
		}

		/**
		 * Prepare playing of @p melody, which should be stored in Flash.
		 * Once preparation is done, actual melody playing is performed by sequenced
		 * calls to `start_next_note()` and `stop_current_note()`.
		 */
		void prepare_flash(const TONE_PLAY* melody)
		{
			prepare_(melody, load_flash);
		}

		/**
		 * Ask this player to start playing the next note of the melody.
		 * @return the duration of the next note that just started playing; it is
		 * the responsibility of the caller to wait for that duration until calling
		 * `stop_current_note()`.
		 * @retval 0 if no wait is needed until next call to `stop_current_note()`;
		 * this may happen when the next melody note is not a true note but an
		 * instruction (e.g. repeat start/end), or when the melody is finished
		 * playing.
		 */
		uint16_t start_next_note()
		{
			return start_next_();
		}

		/**
		 * Ask this player to stop playing the current note of the melody.
		 * @return the duration of inter note play (short silence between 
		 * consecutive notes)
		 * @retval 0 if there is no delay needed between the current note and 
		 * the next one; this may also happen when the current note is not a true
		 * note but an instruction (e.g. repeat start/end), or when the melody
		 * is finished playing.
		 */
		uint16_t stop_current_note()
		{
			return stop_current_();
		}

		/**
		 * Indicate if the currently played melody is finished.
		 */
		bool is_finished() const
		{
			return (loader_ == nullptr);
		}

	private:
		static constexpr const uint16_t INTERTONE_DELAY_MS = 20;

		using LOAD_TONE = const TONE_PLAY* (*) (const TONE_PLAY* address, TONE_PLAY& holder);

		void prepare_(const TONE_PLAY* melody, LOAD_TONE load_tone)
		{
			loader_ = load_tone;
			current_play_ = melody;
			repeat_play_ = nullptr;
			repeat_times_ = 0;
		}

		void reset_()
		{
			loader_ = nullptr;
			current_play_ = nullptr;
			repeat_play_ = nullptr;
			repeat_times_ = 0;
		}

		uint16_t start_next_()
		{
			if (loader_ == nullptr) return 0;

			uint16_t delay = 0;
			TONE_PLAY holder;
			const TONE_PLAY* current = loader_(current_play_, holder);
			if (current->is_end())
			{
				reset_();
				return 0;
			}
			if (current->is_repeat_start())
			{
				repeat_play_ = current_play_;
				repeat_times_ = -1;
				no_delay_ = true;
			}
			else if (current->is_repeat_end())
			{
				if (repeat_play_)
				{
					if (repeat_times_ == -1) repeat_times_ = current->repeat_count();
					if (repeat_times_--)
						current_play_ = repeat_play_;
					else
						repeat_play_ = nullptr;
				}
				no_delay_ = true;
			}
			else
			{
				if (current->is_tone()) current->generate_tone(generator_);
				delay = duration(current->duration());
				no_delay_ = false;
			}
			++current_play_;
			return delay;
		}

		uint16_t stop_current_()
		{
			if (no_delay_)
				return 0;
			generator_.stop_tone();
			return INTERTONE_DELAY_MS;
		}

		uint16_t duration(Duration d) const
		{
			return uint8_t(d) * t32_duration_ms_;
		}

		static const TONE_PLAY* load_sram(const TONE_PLAY* address, TONE_PLAY& holder UNUSED)
		{
			return address;
		}
		static const TONE_PLAY* load_eeprom(const TONE_PLAY* address, TONE_PLAY& holder)
		{
			eeprom::EEPROM::read(address, holder);
			return &holder;
		}
		static const TONE_PLAY* load_flash(const TONE_PLAY* address, TONE_PLAY& holder)
		{
			flash::read_flash(address, holder);
			return &holder;
		}

		GENERATOR& generator_;
		LOAD_TONE loader_;
		uint16_t t32_duration_ms_;
		const TONE_PLAY* current_play_;
		const TONE_PLAY* repeat_play_;
		int8_t repeat_times_;
		bool no_delay_;
	};

	/**
	 * This API defines a player of melodies, defined as a sequence of tones and
	 * durations.
	 * This player is synchronous, i.e. when asking it to play a melody, the called
	 * method will not return until the melody is finished playing (or if `stop()` 
	 * has been called, e.g. by an ISR).
	 * 
	 * Melodies are defined as sequence of unit information, which can be either:
	 * - `TonePlay`s: easy to write in source code but not efficient in size of
	 * generated code
	 * - `QTonePlay`s: requires more effort in source code, but reduces generated
	 * code size
	 * Which types is used is defined as the @p TONEPLAY template parameter.
	 * 
	 * With this API, played melodies can be stored on 3 possible locations:
	 * - in SRAM: this is useful when you get the melody from another support
	 * e.g. an external flash device
	 * - in Flash: this is the mostly used way as flash s the more abundant storage
	 * in AVR MCU
	 * - in EEPROM: this can be useful for short melodies, when you do not want
	 * to waste precious SRAM and Flash
	 * 
	 * Each API has 3 distinct methods, one for each storage strategy.
	 * 
	 * @tparam NTIMER the AVR timer to use for the underlying Timer
	 * @tparam OUTPUT the `board::PWMPin` connected to the buzzer;
	 * this must be the pin OCnA, where n is the AVR Timer number
	 * @tparam TONEPLAY the type used to store melody data, `QTonePlay` by default
	 * 
	 * @sa TonePlay
	 * @sa QTonePlay
	 * @sa AsyncTonePlayer
	 */
	template<board::Timer NTIMER, board::PWMPin OUTPUT, typename TONEPLAY = QTonePlay<NTIMER, OUTPUT>>
	class TonePlayer : public AbstractTonePlayer<NTIMER, OUTPUT, TONEPLAY>
	{
		using BASE = AbstractTonePlayer<NTIMER, OUTPUT, TONEPLAY>;

	public:
		/** The type of `ToneGenerator` to use as constructor's argument. */
		using GENERATOR = typename BASE::GENERATOR;
		/** The type that holds unit of information of a melody. */
		using TONE_PLAY = typename BASE::TONE_PLAY;

		/**
		 * Create a new synchronous tone player, based on an existing `ToneGenerator`.
		 * @param tone_generator the `ToneGenerator` used to actually produce
		 * tones.
		 */
		explicit TonePlayer(GENERATOR& tone_generator) : BASE{tone_generator}, stop_{true} {}

		//TODO DOC update
		/**
		 * Play a melody, defined by a sequence of `TONE_PLAY`s, stored in SRAM.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in SRAM, to the sequence of `TONE_PLAY` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 * @sa play_eeprom()
		 * @sa play_flash()
		 * @sa stop()
		 */
		void play_sram(const TONE_PLAY* melody, uint8_t bpm)
		{
			this->set_min_duration(BASE::calculate_min_duration(bpm));
			this->prepare_sram(melody);
			play_();
		}

		//TODO DOC update
		/**
		 * Play a melody, defined by a sequence of `TONE_PLAY`s, stored in EEPROM.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in EEPROM, to the sequence of `TONE_PLAY` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 * @sa play_sram()
		 * @sa play_flash()
		 * @sa stop()
		 */
		void play_eeprom(const TONE_PLAY* melody, uint8_t bpm)
		{
			this->set_min_duration(BASE::calculate_min_duration(bpm));
			this->prepare_eeprom(melody);
			play_();
		}

		//TODO DOC update
		/**
		 * Play a melody, defined by a sequence of `TONE_PLAY`s, stored in Flash.
		 * This method is blocking: it will return only when the melody is
		 * finished playing.
		 * @param melody a pointer, in Flash, to the sequence of `TONE_PLAY` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 * @sa play_eeprom()
		 * @sa play_sram()
		 * @sa stop()
		 */
		void play_flash(const TONE_PLAY* melody, uint8_t bpm)
		{
			this->set_min_duration(BASE::calculate_min_duration(bpm));
			this->prepare_flash(melody);
			play_();
		}

		/**
		 * Stop playing current melody (if any).
		 * Effect is not immediate but will stop at the end of the current tone.
		 */
		void stop()
		{
			stop_ = true;
		}

		/**
		 * Tell if a melody is currently playing.
		 */
		bool is_playing() const
		{
			return !stop_;
		}

	private:
		void play_()
		{
			stop_ = false;
			while (!stop_)
			{
				uint16_t delay = this->start_next_note();
				if (delay) time::delay_ms(delay);
				delay = this->stop_current_note();
				if (delay) time::delay_ms(delay);
				if (this->is_finished())
					stop_ = true;
			}
		}

		volatile bool stop_;
	};

	/**
	 * This API defines a player of melodies, defined as a sequence of tones and
	 * durations.
	 * This player is asynchronous, i.e. when asking it to play a melody, the called
	 * method will return immediately even before the melody starts playing; then
	 * its `update()` method must be called frequently (from an ISR, or from a main
	 * event loop).
	 * 
	 * Melodies are defined as sequence of unit information, which can be either:
	 * - `TonePlay`s: easy to write in source code but not efficient in size of
	 * generated code
	 * - `QTonePlay`s: requires more effort in source code, but reduces generated
	 * code size
	 * Which types is used is defined as the @p TONEPLAY template parameter.
	 * 
	 * With this API, played melodies can be stored on 3 possible locations:
	 * - in SRAM: this is useful when you get the melody from another support
	 * e.g. an external flash device
	 * - in Flash: this is the mostly used way as flash s the more abundant storage
	 * in AVR MCU
	 * - in EEPROM: this can be useful for short melodies, when you do not want
	 * to waste precious SRAM and Flash
	 * 
	 * Each API has 3 distinct methods, one for each storage strategy.
	 * 
	 * @tparam NTIMER the AVR timer to use for the underlying Timer
	 * @tparam OUTPUT the `board::PWMPin` connected to the buzzer;
	 * this must be the pin OCnA, where n is the AVR Timer number
	 * @tparam TONEPLAY the type used to store melody data, `QTonePlay` by default
	 * 
	 * @sa TonePlay
	 * @sa QTonePlay
	 * @sa TonePlayer
	 */
	template<board::Timer NTIMER, board::PWMPin OUTPUT, typename TONEPLAY = QTonePlay<NTIMER, OUTPUT>>
	class AsyncTonePlayer : public AbstractTonePlayer<NTIMER, OUTPUT, TONEPLAY>
	{
		using BASE = AbstractTonePlayer<NTIMER, OUTPUT, TONEPLAY>;
		using THIS = AsyncTonePlayer<NTIMER, OUTPUT, TONEPLAY>;

	public:
		/** The type of `ToneGenerator` to use as constructor's argument. */
		using GENERATOR = typename BASE::GENERATOR;
		/** The type that holds unit of information of a melody. */
		using TONE_PLAY = typename BASE::TONE_PLAY;

		/**
		 * Create a new asynchronous tone player, based on an existing `ToneGenerator`.
		 * @param tone_generator the `ToneGenerator` used to actually produce
		 * tones.
		 */
		explicit AsyncTonePlayer(GENERATOR& tone_generator)
		: BASE{tone_generator}, status_{Status::NOT_STARTED}, next_time_{} {}

		//TODO DOC
		//TODO check better API (statis?) to find out best timer settings in relation to bpm
		uint16_t get_min_duration() const
		{
			return BASE::get_min_duration();
		}

		//TODO DOC update
		/**
		 * Start playing a melody, defined by a sequence of `TONE_PLAY`s, stored in SRAM.
		 * This method is asynchronous: it returns immediately even ebfore starting
		 * playing the first melody's note.
		 * Actual play is performed by frequent calls of `update()`.
		 * @param melody a pointer, in SRAM, to the sequence of `TONE_PLAY` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 * @sa play_eeprom()
		 * @sa play_flash()
		 * @sa stop()
		 * @sa update()
		 */
		void play_sram(const TONE_PLAY* melody, uint8_t bpm)
		{
			status_ = Status::NOT_STARTED;
			this->set_min_duration(BASE::calculate_min_duration(bpm));
			this->prepare_sram(melody);
			next_time_ = 0;
			status_ = Status::STARTED;
		}

		//TODO DOC update
		/**
		 * Start playing a melody, defined by a sequence of `TONE_PLAY`s, stored in EEPROM.
		 * This method is asynchronous: it returns immediately even ebfore starting
		 * playing the first melody's note.
		 * Actual play is performed by frequent calls of `update()`.
		 * @param melody a pointer, in EEPROM, to the sequence of `TONE_PLAY` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 * @sa play_sram()
		 * @sa play_flash()
		 * @sa stop()
		 * @sa update()
		 */
		void play_eeprom(const TONE_PLAY* melody, uint8_t bpm)
		{
			status_ = Status::NOT_STARTED;
			this->set_min_duration(BASE::calculate_min_duration(bpm));
			this->prepare_eeprom(melody);
			next_time_ = 0;
			status_ = Status::STARTED;
		}

		//TODO DOC update
		/**
		 * Start playing a melody, defined by a sequence of `TONE_PLAY`s, stored in Flash.
		 * This method is asynchronous: it returns immediately even ebfore starting
		 * playing the first melody's note.
		 * Actual play is performed by frequent calls of `update()`.
		 * @param melody a pointer, in Flash, to the sequence of `TONE_PLAY` to be
		 * played; the sequence MUST finish with a `SpecialTone::END`.
		 * @sa play_eeprom()
		 * @sa play_sram()
		 * @sa stop()
		 * @sa update()
		 */
		void play_flash(const TONE_PLAY* melody, uint8_t bpm)
		{
			status_ = Status::NOT_STARTED;
			this->set_min_duration(BASE::calculate_min_duration(bpm));
			this->prepare_flash(melody);
			next_time_ = 0;
			status_ = Status::STARTED;
		}

		/**
		 * Stop playing current melody (if any).
		 * Effect is not immediate but will stop at the end of the current tone.
		 */
		void stop()
		{
			status_ = Status::NOT_STARTED;
			this->stop_current_note();
		}

		/**
		 * Tell if a melody is currently playing.
		 */
		bool is_playing() const
		{
			return status_ != Status::NOT_STARTED;
		}

		/**
		 * Ask this player to update current play if needed, based on current
		 * time (as returned by an `timer::RTT` for example).
		 * This may be called from an ISR or from an event loop in `main()`.
		 * This is the end program responsibility to call this method at proper
		 * intervals, in order to ensure fidelity of melody tempo.
		 * 
		 * @param rtt_millis the current real time (in milliseconds), as obtained 
		 * from an `timer::RTT` instance.
		 */
		void update(uint32_t rtt_millis)
		{
			if ((status_ != Status::NOT_STARTED) && (rtt_millis >= next_time_))
			{
				uint16_t delay;
				Status next;
				if (status_ == Status::PLAYING_NOTE)
				{
					delay = this->stop_current_note();
					next = Status::PLAYING_INTERNOTE;
				}
				else
				{
					delay = this->start_next_note();
					next = Status::PLAYING_NOTE;
				}
				
				if (this->is_finished())
					status_ = Status::NOT_STARTED;
				else
				{
					next_time_ = rtt_millis + delay;
					status_ = next;
				}
			}
		}

	private:
		enum class Status : uint8_t
		{
			NOT_STARTED = 0,
			STARTED,
			PLAYING_NOTE,
			PLAYING_INTERNOTE
		};

		Status status_;
		uint32_t next_time_;
	};
}

#endif /* TONE_PLAYER_HH */
/// @endcond
