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
// Better score found at
// https://www.musicnotes.com/sheetmusic/mtd.asp?ppn=MN0017607

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
using QTONEPLAY = TONEPLAYER::TONE_PLAY;

// Define constants with short names to ease score transcription
using devices::audio::Duration;
static constexpr const Duration WN = Duration::WHOLE;
static constexpr const Duration HN = Duration::HALF;
static constexpr const Duration QN = Duration::QUARTER;
static constexpr const Duration QV = Duration::QUAVER;
static constexpr const Duration SQ = Duration::SEMI_QUAVER;
static constexpr auto DOT = devices::audio::dotted;

// The Imperial March
static const QTONEPLAY music[] PROGMEM =
{
	// First part
	QTONEPLAY{Tone::A1, QN},
	QTONEPLAY{Tone::A1, QN},
	QTONEPLAY{Tone::A1, QN},
	QTONEPLAY{Tone::F1, DOT(QV)},
	QTONEPLAY{Tone::C2, SQ},
	QTONEPLAY{Tone::A1, QN},
	QTONEPLAY{Tone::F1, DOT(QV)},
	QTONEPLAY{Tone::C2, SQ},
	QTONEPLAY{Tone::A1, HN},

	// Second part
	QTONEPLAY{Tone::E2, QN},
	QTONEPLAY{Tone::E2, QN},
	QTONEPLAY{Tone::E2, QN},
	QTONEPLAY{Tone::F2, DOT(QV)},
	QTONEPLAY{Tone::C2, SQ},
	QTONEPLAY{Tone::Gs1, QN},
	QTONEPLAY{Tone::F1, DOT(QV)},
	QTONEPLAY{Tone::C2, SQ},
	QTONEPLAY{Tone::A1, HN},

	// Third part (repeated once)
	QTONEPLAY{REPEAT_START},
	QTONEPLAY{Tone::A2, QN},
	QTONEPLAY{Tone::A1, DOT(QV)},
	QTONEPLAY{Tone::A1, SQ},
	QTONEPLAY{Tone::A2, QN},
	QTONEPLAY{Tone::Gs2, DOT(QV)},
	QTONEPLAY{Tone::G2, SQ},
	QTONEPLAY{Tone::Fs2, SQ},
	QTONEPLAY{Tone::F2, SQ},
	QTONEPLAY{Tone::Fs2, QV},
	QTONEPLAY{Tone::SILENCE, QV},

	QTONEPLAY{Tone::As1, QV},
	QTONEPLAY{Tone::Ds2, QN},
	QTONEPLAY{Tone::D2, DOT(QV)},
	QTONEPLAY{Tone::Cs2, SQ},
	QTONEPLAY{Tone::C2, SQ},
	QTONEPLAY{Tone::B1, SQ},
	QTONEPLAY{Tone::C2, QV},
	QTONEPLAY{Tone::SILENCE, QV},

	QTONEPLAY{Tone::F1, SQ},
	QTONEPLAY{Tone::Gs1, QN},
	QTONEPLAY{Tone::F1, DOT(QV)},
	QTONEPLAY{Tone::A1, SQ},
	QTONEPLAY{Tone::C2, QN},
	QTONEPLAY{Tone::A1, DOT(QV)},
	QTONEPLAY{Tone::C2, SQ},
	QTONEPLAY{Tone::E2, HN},
	QTONEPLAY{REPEAT_END, 1},

	QTONEPLAY{END, 0}
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

	gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};
	
	RTT timer;
	RTTCALLBACK handler{events_queue};
	interrupt::register_handler(handler);

	GENERATOR generator;
	TONEPLAYER player{generator};

	time::delay_ms(5000);
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
