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

// The Imperial March
// Durations have been reviewed based on official score (at 120 BPM)
static const QTONEPLAY music[] PROGMEM =
{
	// First part
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::F1, 375},
	QTONEPLAY{Tone::C2, 125},
	QTONEPLAY{Tone::A1, 500},
	QTONEPLAY{Tone::F1, 375},
	QTONEPLAY{Tone::C2, 125},
	QTONEPLAY{Tone::A1, 1000},

	// Second part
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::E2, 500},
	QTONEPLAY{Tone::F2, 375},
	QTONEPLAY{Tone::C2, 125},
	QTONEPLAY{Tone::Gs1, 500},
	QTONEPLAY{Tone::F1, 375},
	QTONEPLAY{Tone::C2, 125},
	QTONEPLAY{Tone::A1, 1000},

	// Third part (repeated once)
	QTONEPLAY{REPEAT_START},
	QTONEPLAY{Tone::A2, 500},
	QTONEPLAY{Tone::A1, 375},
	QTONEPLAY{Tone::A1, 125},
	QTONEPLAY{Tone::A2, 500},
	QTONEPLAY{Tone::Gs2, 375},
	QTONEPLAY{Tone::G2, 125},
	QTONEPLAY{Tone::Fs2, 125},
	QTONEPLAY{Tone::F2, 125},
	QTONEPLAY{Tone::Fs2, 250},
	QTONEPLAY{Tone::SILENCE, 250},

	QTONEPLAY{Tone::As1, 250},
	QTONEPLAY{Tone::Ds2, 500},
	QTONEPLAY{Tone::D2, 375},
	QTONEPLAY{Tone::Cs2, 125},
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
	QTONEPLAY{Tone::E2, 1000},
	QTONEPLAY{REPEAT_END, 1},

	QTONEPLAY{END, 0}
};

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
	player.play_flash(music);

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
