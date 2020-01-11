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
 * This example is playing the melody asynchronously, based on RTT events.
 * In this example, the melody is stored in Flash.
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - D6: connect to a 5V piezo buzzer with the othe lead connected to ground
 *   - D13: embedded LED that blinks synchronously from main()
 */

// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March
// Better score (simplified) found at
// http://www.filmmusicnotes.com/john-williams-themes-part-3-of-6-the-imperial-march-darth-vaders-theme/

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/events.h>
#include <fastarduino/gpio.h>
#include <fastarduino/queue.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/time.h>
#include <fastarduino/devices/tone_player.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D6_PD6_OC0A;
#define RTTTIMER 1
static constexpr const board::Timer NRTTTIMER = board::Timer::TIMER1;

using TONEPLAYER = devices::audio::AsyncTonePlayer<NTIMER, OUTPUT>;
using devices::audio::Tone;
using namespace devices::audio::SpecialTone;
using TONEPLAY = TONEPLAYER::TONE_PLAY;

// Define constants with short names to ease score transcription
using devices::audio::Duration;
static constexpr const Duration WN = Duration::WHOLE;
static constexpr const Duration HN = Duration::HALF;
static constexpr const Duration QN = Duration::QUARTER;
static constexpr const Duration QV = Duration::QUAVER;
static constexpr const Duration SQ = Duration::SEMI_QUAVER;
static constexpr auto DOT = devices::audio::dotted;
static constexpr auto TRIPLET = devices::audio::triplet;

static const TONEPLAY music[] PROGMEM =
{
	// March 1st part (4 times)
	TONEPLAY(REPEAT_START, 0),
	TONEPLAY{Tone::G0, QN},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::Ef0, TRIPLET(SQ)},
	TONEPLAY{Tone::Ef0, TRIPLET(SQ)},
	TONEPLAY{Tone::Ef0, TRIPLET(SQ)},
	TONEPLAY{Tone::Ef0, QV},
	TONEPLAY(REPEAT_END, 3),

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

	// March 2nd part (2 times)
	TONEPLAY(REPEAT_START, 0),
	TONEPLAY{Tone::G0, QN},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::G0, TRIPLET(SQ)},
	TONEPLAY{Tone::Ef0, TRIPLET(SQ)},
	TONEPLAY{Tone::Ef0, TRIPLET(SQ)},
	TONEPLAY{Tone::Ef0, TRIPLET(SQ)},
	TONEPLAY{Tone::Ef0, QV},
	TONEPLAY(REPEAT_END, 1),

	// More to come here in the melody, add it someday when I have time...
	
	TONEPLAY{END, 0}
};

static constexpr const uint8_t BPM = 120;

using GENERATOR = TONEPLAYER::GENERATOR;

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

	gpio::FAST_PIN<board::DigitalPin::LED> led{gpio::PinMode::OUTPUT};
	
	RTT timer;
	RTTCALLBACK handler{events_queue};
	interrupt::register_handler(handler);

	GENERATOR generator;
	TONEPLAYER player{generator};

	time::delay_ms(1000);
	player.play_flash(music, BPM);

	timer.begin();
	while (true)
	{
		EVENT event;
		if (events_queue.pull(event) && player.is_playing())
		{
			led.toggle();
			player.update(timer.millis());
		}
	}
}
