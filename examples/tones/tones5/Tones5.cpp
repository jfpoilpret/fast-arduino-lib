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

	void start(const TONE_PLAY* melody)
	{
		status_ = Status::NOT_STARTED;
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
using QTONEPLAY = AsyncTonePlayer::TONE_PLAY;

// The Imperial March
static QTONEPLAY music[] =
{
	// First part
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::F1, 350},
	QTONEPLAY{Tone::C2, 150},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::F1, 350},
	QTONEPLAY{Tone::C2, 150},
	QTONEPLAY{Tone::A1, 650},
	QTONEPLAY{Tone::SILENCE, 150},

	// Second part
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::F2, 350},
	QTONEPLAY{Tone::C2, 150},
	QTONEPLAY{Tone::Gs1, 500},
	QTONEPLAY{Tone::F1, 350},
	QTONEPLAY{Tone::C2, 150},
	QTONEPLAY{Tone::A1, 650},
	QTONEPLAY{Tone::SILENCE, 150},

	// Third part (repeated once)
	QTONEPLAY{REPEAT_START},
	QTONEPLAY{Tone::A2, 500},
	QTONEPLAY{Tone::A1, 300},
	QTONEPLAY{Tone::A1, 150},
	QTONEPLAY{Tone::A2, 400},
	QTONEPLAY{Tone::Gs2, 200},
	QTONEPLAY{Tone::G2, 200},
	QTONEPLAY{Tone::Fs2, 125},
	QTONEPLAY{Tone::F2, 125},
	QTONEPLAY{Tone::Fs2, 250},
	QTONEPLAY{Tone::SILENCE, 250},

	QTONEPLAY{Tone::As1, 250},
	QTONEPLAY{Tone::Ds2, 400},
	QTONEPLAY{Tone::D2, 200},
	QTONEPLAY{Tone::Cs2, 200},
	QTONEPLAY{Tone::C2, 125},
	QTONEPLAY{Tone::B1, 125},
	QTONEPLAY{Tone::C2, 250},
	QTONEPLAY{Tone::SILENCE, 250},

	QTONEPLAY{Tone::F1, 125},
	QTONEPLAY{Tone::Gs1, 500},
	QTONEPLAY{Tone::F1, 375},
	QTONEPLAY{Tone::A1, 125},
	QTONEPLAY{Tone::C2, 500},
	QTONEPLAY{Tone::A1, 375},
	QTONEPLAY{Tone::C2, 125},
	QTONEPLAY{Tone::E2, 650},
	QTONEPLAY{Tone::SILENCE, 250},
	QTONEPLAY{REPEAT_END, 1},

	QTONEPLAY{END, 0}
};

REGISTER_RTT_ISR_METHOD(RTTTIMER, AsyncTonePlayer, &AsyncTonePlayer::rtt_update)

using GENERATOR = AsyncTonePlayer::GENERATOR;
using RTT = timer::RTT<NRTTTIMER>;

int main() __attribute__((OS_main));
int main()
{
	sei();

	gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};
	
	GENERATOR generator;
	AsyncTonePlayer player{generator};
	RTT timer;
	timer.begin();

	while (true)
	{
		time::delay_ms(5000);
		player.start(music);
		while (player.is_playing())
		{
			time::delay_ms(500);
			led.toggle();
		}
	}
}
