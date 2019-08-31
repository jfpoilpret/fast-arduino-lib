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
 * In this example, the melody is stored in SRAM as an array of TonePlay.
 * 
 * Wiring:
 * - on Arduino UNO:
 *   - D6: connect to a 5V piezo buzzer with the othe lead connected to ground
 */

// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/time.h>
#include <fastarduino/devices/tone_player.h>

// Board-dependent settings
// static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
// static constexpr const board::DigitalPin OUTPUT = board::PWMPin::D9_PB1_OC1A;
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D6_PD6_OC0A;

using devices::audio::Tone;
using devices::audio::TonePlay;
using namespace devices::audio::SpecialTone;
using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;
using PLAYER = devices::audio::TonePlayer<NTIMER, OUTPUT, TonePlay>;
using TONEPLAY = PLAYER::TONE_PLAY;

static TONEPLAY music[] =
{
	// First part
	TONEPLAY{Tone::A1, 500},
	TONEPLAY{Tone::A1, 500},
	TONEPLAY{Tone::A1, 500},
	TONEPLAY{Tone::F1, 350},
	TONEPLAY{Tone::C2, 150},
	TONEPLAY{Tone::A1, 500},
	TONEPLAY{Tone::F1, 350},
	TONEPLAY{Tone::C2, 150},
	TONEPLAY{Tone::A1, 650},
	TONEPLAY{Tone::SILENCE, 150},

	// Second part
	TONEPLAY{Tone::E2, 500},
	TONEPLAY{Tone::E2, 500},
	TONEPLAY{Tone::E2, 500},
	TONEPLAY{Tone::F2, 350},
	TONEPLAY{Tone::C2, 150},
	TONEPLAY{Tone::Gs1, 500},
	TONEPLAY{Tone::F1, 350},
	TONEPLAY{Tone::C2, 150},
	TONEPLAY{Tone::A1, 650},
	TONEPLAY{Tone::SILENCE, 150},

	// Third part (repeated once)
	TONEPLAY{REPEAT_START},
	TONEPLAY{Tone::A2, 500},
	TONEPLAY{Tone::A1, 300},
	TONEPLAY{Tone::A1, 150},
	TONEPLAY{Tone::A2, 400},
	TONEPLAY{Tone::Gs2, 200},
	TONEPLAY{Tone::G2, 200},
	TONEPLAY{Tone::Fs2, 125},
	TONEPLAY{Tone::F2, 125},
	TONEPLAY{Tone::Fs2, 250},
	TONEPLAY{Tone::SILENCE, 250},

	TONEPLAY{Tone::As1, 250},
	TONEPLAY{Tone::Ds2, 400},
	TONEPLAY{Tone::D2, 200},
	TONEPLAY{Tone::Cs2, 200},
	TONEPLAY{Tone::C2, 125},
	TONEPLAY{Tone::B1, 125},
	TONEPLAY{Tone::C2, 250},
	TONEPLAY{Tone::SILENCE, 250},

	TONEPLAY{Tone::F1, 125},
	TONEPLAY{Tone::Gs1, 500},
	TONEPLAY{Tone::F1, 375},
	TONEPLAY{Tone::A1, 125},
	TONEPLAY{Tone::C2, 500},
	TONEPLAY{Tone::A1, 375},
	TONEPLAY{Tone::C2, 125},
	TONEPLAY{Tone::E2, 650},
	TONEPLAY{Tone::SILENCE, 250},
	TONEPLAY{REPEAT_END, 1},

	TONEPLAY{END, 0}
};

int main() __attribute__((OS_main));
int main()
{
	sei();
	time::delay_ms(5000);

	GENERATOR generator;
	PLAYER player{generator};
	player.play_sram(music);
}
