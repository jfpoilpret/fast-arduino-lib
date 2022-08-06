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

/*
 * Frequency generator example, used to play the Imperial March.
 * This example is playing the melody asynchronously, based on RTT ISR.
 * In this example, the melody is stored in SRAM.
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - D6: connect to a 5V piezo buzzer with the othe lead connected to ground
 *   - D13: embedded LED that blinks synchronously from main()
 */

// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/gpio.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/time.h>
#include <fastarduino/devices/tone_player.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D6_PD6_OC0A;
#define RTTTIMER 1
static constexpr const board::Timer NRTTTIMER = board::Timer::TIMER1;

class AsyncTonePlayer : public devices::audio::AbstractTonePlayer<NTIMER, OUTPUT>
{
	using BASE = AbstractTonePlayer<NTIMER, OUTPUT>;

public:
	using GENERATOR = typename BASE::GENERATOR;
	using TONE_PLAY = typename BASE::TONE_PLAY;

	AsyncTonePlayer(GENERATOR& tone_generator)
	: BASE{tone_generator}, status_{Status::NOT_STARTED}
	{
		interrupt::register_handler(*this);
	}

	void start(const TONE_PLAY* melody, uint8_t bpm)
	{
		status_ = Status::NOT_STARTED;
		set_min_duration(devices::audio::Beat{bpm}.duration());
		prepare_sram(melody);
		next_time_ = 0;
		status_ = Status::STARTED;
	}

	void stop()
	{
		status_ = Status::NOT_STARTED;
	}

	bool is_playing() const
	{
		return status_ != Status::NOT_STARTED;
	}

private:
	enum class Status : uint8_t
	{
		NOT_STARTED = 0,
		STARTED,
		PLAYING_NOTE,
		PLAYING_INTERNOTE
	};

	void rtt_update(uint32_t millis)
	{
		if ((status_ != Status::NOT_STARTED) && (millis >= next_time_))
		{
			uint16_t delay;
			Status next;
			if (status_ == Status::PLAYING_NOTE)
			{
				delay = stop_current_note();
				next = Status::PLAYING_INTERNOTE;
			}
			else
			{
				delay = start_next_note();
				next = Status::PLAYING_NOTE;
			}
			
			if (is_finished())
				status_ = Status::NOT_STARTED;
			else
			{
				next_time_ = millis + delay;
				status_ = next;
			}
		}
	}

	Status status_;
	uint32_t next_time_;

	DECL_RTT_ISR_HANDLERS_FRIEND
};

using devices::audio::Tone;
using namespace devices::audio::SpecialTone;
using TONEPLAY = AsyncTonePlayer::TONE_PLAY;

// Define constants with short names to ease score transcription
using devices::audio::Duration;
static constexpr const Duration WN = Duration::WHOLE;
static constexpr const Duration HN = Duration::HALF;
static constexpr const Duration QN = Duration::QUARTER;
static constexpr const Duration QV = Duration::QUAVER;
static constexpr const Duration SQ = Duration::SEMI_QUAVER;
static constexpr auto DOT = devices::audio::dotted;
static constexpr auto TRIPLET = devices::audio::triplet;

// The Imperial March
static TONEPLAY music[] =
{
	// Melody first part
	TONEPLAY{Tone::G2, QN},
	TONEPLAY{Tone::G2, QN},
	TONEPLAY{Tone::G2, QN},
	TONEPLAY{Tone::Ef2, DOT(QV)},
	TONEPLAY{Tone::Bf2, SQ},

	TONEPLAY{Tone::G2, QN},
	TONEPLAY{Tone::Ef2, DOT(QV)},
	TONEPLAY{Tone::Bf2, SQ},
	TONEPLAY{Tone::G2, HN},

	// Melody second part
	TONEPLAY{Tone::D3, QN},
	TONEPLAY{Tone::D3, QN},
	TONEPLAY{Tone::D3, QN},
	TONEPLAY{Tone::Ef3, DOT(QV)},
	TONEPLAY{Tone::Bf2, SQ},

	TONEPLAY{Tone::Gf2, QN},
	TONEPLAY{Tone::Ef2, DOT(QV)},
	TONEPLAY{Tone::Bf2, SQ},
	TONEPLAY{Tone::G2, HN},

	// Melody third part
	TONEPLAY{Tone::G3, QN},
	TONEPLAY{Tone::G2, DOT(QV)},
	TONEPLAY{Tone::G2, SQ},
	TONEPLAY{Tone::G3, QN},
	TONEPLAY{Tone::Fs3, DOT(QV)},
	TONEPLAY{Tone::F3, SQ},

	TONEPLAY{Tone::E3, SQ},
	TONEPLAY{Tone::Ds3, SQ},
	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::SILENCE, QV},
	TONEPLAY{Tone::Gs2, QV},
	TONEPLAY{Tone::Cs3, QN},
	TONEPLAY{Tone::C3, DOT(QV)},
	TONEPLAY{Tone::B2, SQ},

	TONEPLAY{Tone::Bf2, SQ},
	TONEPLAY{Tone::A2, SQ},
	TONEPLAY{Tone::Bf2, QV},
	TONEPLAY{Tone::SILENCE, QV},
	TONEPLAY{Tone::Ef2, SQ},
	TONEPLAY{Tone::Gf2, QN},
	TONEPLAY{Tone::Ef2, DOT(QV)},
	TONEPLAY{Tone::Gf2, SQ},

	TONEPLAY{Tone::Bf2, QN},
	TONEPLAY{Tone::G2, DOT(QV)},
	TONEPLAY{Tone::Bf2, SQ},
	TONEPLAY{Tone::D3, HN},

	// Melody fourth part (like 3rd part except last bar)
	TONEPLAY{Tone::G3, QN},
	TONEPLAY{Tone::G2, DOT(QV)},
	TONEPLAY{Tone::G2, SQ},
	TONEPLAY{Tone::G3, QN},
	TONEPLAY{Tone::Fs3, DOT(QV)},
	TONEPLAY{Tone::F3, SQ},

	TONEPLAY{Tone::E3, SQ},
	TONEPLAY{Tone::Ds3, SQ},
	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::SILENCE, QV},
	TONEPLAY{Tone::Gs2, QV},
	TONEPLAY{Tone::Cs3, QN},
	TONEPLAY{Tone::C3, DOT(QV)},
	TONEPLAY{Tone::B2, SQ},

	TONEPLAY{Tone::Bf2, SQ},
	TONEPLAY{Tone::A2, SQ},
	TONEPLAY{Tone::Bf2, QV},
	TONEPLAY{Tone::SILENCE, QV},
	TONEPLAY{Tone::Ef2, SQ},
	TONEPLAY{Tone::Gf2, QN},
	TONEPLAY{Tone::Ef2, DOT(QV)},
	TONEPLAY{Tone::Bf2, SQ},

	TONEPLAY{Tone::G2, QN},
	TONEPLAY{Tone::Ef2, DOT(QV)},
	TONEPLAY{Tone::Bf2, SQ},
	TONEPLAY{Tone::G2, HN},

	TONEPLAY{END, 0}
};

REGISTER_RTT_ISR_METHOD(RTTTIMER, AsyncTonePlayer, &AsyncTonePlayer::rtt_update)

using GENERATOR = AsyncTonePlayer::GENERATOR;
using RTT = timer::RTT<NRTTTIMER>;

static constexpr const uint8_t BPM = 120;

int main() __attribute__((OS_main));
int main()
{
	sei();

	gpio::FAST_PIN<board::DigitalPin::LED> led{gpio::PinMode::OUTPUT};
	
	GENERATOR generator;
	AsyncTonePlayer player{generator};
	RTT timer;
	timer.begin();

	while (true)
	{
		time::delay_ms(5000);
		player.start(music, BPM);
		while (player.is_playing())
		{
			time::delay_ms(500);
			led.toggle();
		}
	}
}
