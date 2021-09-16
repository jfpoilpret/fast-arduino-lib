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
 * Frequency generator example, used to play the Imperial March on 2 channels.
 * This example is playing the melody asynchronously, based on RTT events.
 * In this example, the melody is stored in Flash.
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - D6: connect to a 5V piezo buzzer (treble) with the othe lead connected to ground
 *   - D11: connect to a 5V piezo buzzer (bass) with the othe lead connected to ground
 */

// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March
// Better score (simplified) found at
// http://www.filmmusicnotes.com/john-williams-themes-part-3-of-6-the-imperial-march-darth-vaders-theme/
// March score (timpani) thanks:
// http://pop-sheet-music.com/Files/9763704e1f9e5a8fd0492a98cd4b3e54.pdf

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/events.h>
#include <fastarduino/gpio.h>
#include <fastarduino/queue.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/time.h>
#include <fastarduino/devices/tone_player.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER1 = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT1 = board::PWMPin::D6_PD6_OC0A;
static constexpr const board::Timer NTIMER2 = board::Timer::TIMER2;
static constexpr const board::PWMPin OUTPUT2 = board::PWMPin::D11_PB3_OC2A;

#define RTTTIMER 1
static constexpr const board::Timer NRTTTIMER = board::Timer::TIMER1;

using TONEPLAYER1 = devices::audio::AsyncTonePlayer<NTIMER1, OUTPUT1>;
using QTONEPLAY1 = TONEPLAYER1::TONE_PLAY;
using TONEPLAYER2 = devices::audio::AsyncTonePlayer<NTIMER2, OUTPUT2>;
using QTONEPLAY2 = TONEPLAYER2::TONE_PLAY;

// Define constants with short names to ease score transcription
using devices::audio::Tone;
using namespace devices::audio::SpecialTone;
using devices::audio::Duration;
static constexpr const Duration WN = Duration::WHOLE;
static constexpr const Duration HN = Duration::HALF;
static constexpr const Duration QN = Duration::QUARTER;
static constexpr const Duration QV = Duration::QUAVER;
static constexpr const Duration SQ = Duration::SEMI_QUAVER;
static constexpr auto DOT = devices::audio::dotted;
static constexpr auto TRIPLET = devices::audio::triplet;

static const QTONEPLAY1 music1[] PROGMEM =
{
	// Prelude: march (4 bars)
	QTONEPLAY1{Tone::REST, WN},
	QTONEPLAY1{Tone::REST, WN},
	QTONEPLAY1{Tone::REST, WN},
	QTONEPLAY1{Tone::REST, WN},

	// First part
	QTONEPLAY1{Tone::G2, QN},
	QTONEPLAY1{Tone::G2, QN},
	QTONEPLAY1{Tone::G2, QN},
	QTONEPLAY1{Tone::Ef2, DOT(QV)},
	QTONEPLAY1{Tone::Bf2, SQ},

	QTONEPLAY1{Tone::G2, QN},
	QTONEPLAY1{Tone::Ef2, DOT(QV)},
	QTONEPLAY1{Tone::Bf2, SQ},
	QTONEPLAY1{Tone::G2, HN},

	// Second part
	QTONEPLAY1{Tone::D3, QN},
	QTONEPLAY1{Tone::D3, QN},
	QTONEPLAY1{Tone::D3, QN},
	QTONEPLAY1{Tone::Ef3, DOT(QV)},
	QTONEPLAY1{Tone::Bf2, SQ},

	QTONEPLAY1{Tone::Gf2, QN},
	QTONEPLAY1{Tone::Ef2, DOT(QV)},
	QTONEPLAY1{Tone::Bf2, SQ},
	QTONEPLAY1{Tone::G2, HN},

	// Third part
	QTONEPLAY1{Tone::G3, QN},
	QTONEPLAY1{Tone::G2, DOT(QV)},
	QTONEPLAY1{Tone::G2, SQ},
	QTONEPLAY1{Tone::G3, QN},
	QTONEPLAY1{Tone::Fs3, DOT(QV)},
	QTONEPLAY1{Tone::F3, SQ},

	QTONEPLAY1{Tone::E3, SQ},
	QTONEPLAY1{Tone::Ds3, SQ},
	QTONEPLAY1{Tone::E3, QV},
	QTONEPLAY1{Tone::SILENCE, QV},
	QTONEPLAY1{Tone::Gs2, QV},
	QTONEPLAY1{Tone::Cs3, QN},
	QTONEPLAY1{Tone::C3, DOT(QV)},
	QTONEPLAY1{Tone::B2, SQ},

	QTONEPLAY1{Tone::Bf2, SQ},
	QTONEPLAY1{Tone::A2, SQ},
	QTONEPLAY1{Tone::Bf2, QV},
	QTONEPLAY1{Tone::SILENCE, QV},
	QTONEPLAY1{Tone::Ef2, SQ},
	QTONEPLAY1{Tone::Gf2, QN},
	QTONEPLAY1{Tone::Ef2, DOT(QV)},
	QTONEPLAY1{Tone::Gf2, SQ},

	QTONEPLAY1{Tone::Bf2, QN},
	QTONEPLAY1{Tone::G2, DOT(QV)},
	QTONEPLAY1{Tone::Bf2, SQ},
	QTONEPLAY1{Tone::D3, HN},

	// Fourth part (like 3rd part except last bar)
	QTONEPLAY1{Tone::G3, QN},
	QTONEPLAY1{Tone::G2, DOT(QV)},
	QTONEPLAY1{Tone::G2, SQ},
	QTONEPLAY1{Tone::G3, QN},
	QTONEPLAY1{Tone::Fs3, DOT(QV)},
	QTONEPLAY1{Tone::F3, SQ},

	QTONEPLAY1{Tone::E3, SQ},
	QTONEPLAY1{Tone::Ds3, SQ},
	QTONEPLAY1{Tone::E3, QV},
	QTONEPLAY1{Tone::SILENCE, QV},
	QTONEPLAY1{Tone::Gs2, QV},
	QTONEPLAY1{Tone::Cs3, QN},
	QTONEPLAY1{Tone::C3, DOT(QV)},
	QTONEPLAY1{Tone::B2, SQ},

	QTONEPLAY1{Tone::Bf2, SQ},
	QTONEPLAY1{Tone::A2, SQ},
	QTONEPLAY1{Tone::Bf2, QV},
	QTONEPLAY1{Tone::SILENCE, QV},
	QTONEPLAY1{Tone::Ef2, SQ},
	QTONEPLAY1{Tone::Gf2, QN},
	QTONEPLAY1{Tone::Ef2, DOT(QV)},
	QTONEPLAY1{Tone::Bf2, SQ},

	QTONEPLAY1{Tone::G2, QN},
	QTONEPLAY1{Tone::Ef2, DOT(QV)},
	QTONEPLAY1{Tone::Bf2, SQ},
	QTONEPLAY1{Tone::G2, HN},

	QTONEPLAY1{END, 0}
};

// March melody (each part is one bar), melody starts after 4 bars
static const QTONEPLAY2 music2[] PROGMEM =
{
	// First part (7 times)
	QTONEPLAY2(REPEAT_START, 0),
	QTONEPLAY2{Tone::G0, QN},
	QTONEPLAY2{Tone::G0, QV},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, QV},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::Ef0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::Ef0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::Ef0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::Ef0, QV},
	QTONEPLAY2(REPEAT_END, 6),

	// 3rd part (once)
	QTONEPLAY2{Tone::Ef0, QN},
	QTONEPLAY2{Tone::C1, QN},
	QTONEPLAY2{Tone::G0, QV},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, QV},

	// 4th part (like 2nd part, only once)
	QTONEPLAY2{Tone::G0, QN},
	QTONEPLAY2{Tone::G0, QV},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, QV},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, TRIPLET(SQ)},
	QTONEPLAY2{Tone::G0, QV},

	// Followed by 12 bars melody...

	QTONEPLAY2{END, 0}
};

static constexpr const uint8_t BPM = 112;

using GENERATOR1 = TONEPLAYER1::GENERATOR;
using GENERATOR2 = TONEPLAYER2::GENERATOR;

using RTT = timer::RTT<NRTTTIMER>;

using EVENT = events::Event<void>;
static constexpr const uint8_t EVENT_QUEUE_SIZE = 32;
static EVENT buffer[EVENT_QUEUE_SIZE];
static containers::Queue<EVENT> events_queue{buffer};

static constexpr const uint16_t PERIOD = 32; 
using RTTCALLBACK = timer::RTTEventCallback<EVENT, PERIOD>;

REGISTER_RTT_EVENT_ISR(RTTTIMER, EVENT, PERIOD)

int main() __attribute__((OS_main));
int main()
{
	sei();

	RTT timer;
	RTTCALLBACK handler{events_queue};
	interrupt::register_handler(handler);

	GENERATOR1 generator1;
	TONEPLAYER1 player1{generator1};
	GENERATOR2 generator2;
	TONEPLAYER2 player2{generator2};

	time::delay_ms(1000);
	player1.play_flash(music1, BPM);
	player2.play_flash(music2, BPM);

	timer.begin();
	while (true)
	{
		if ((!player1.is_playing()) && (!player2.is_playing())) break;
		EVENT event;
		if (events_queue.pull(event))
		{
			if (player1.is_playing())
				player1.update(timer.millis());
			if (player2.is_playing())
				player2.update(timer.millis());
		}
	}
}
