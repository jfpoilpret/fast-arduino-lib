//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * In this example, the melody is stored in SRAM.
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
using namespace devices::audio::SpecialTone;
using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;
using PLAYER = devices::audio::TonePlayer<NTIMER, OUTPUT>;
using TONEPLAY = PLAYER::TONE_PLAY;

// Define constants with short names to ease score transcription
using devices::audio::Duration;
static constexpr const Duration WN = Duration::WHOLE;
static constexpr const Duration HN = Duration::HALF;
static constexpr const Duration QN = Duration::QUARTER;
static constexpr const Duration QV = Duration::QUAVER;
static constexpr const Duration SQ = Duration::SEMI_QUAVER;
static constexpr auto DOT = devices::audio::dotted;
static constexpr auto TRIPLET = devices::audio::triplet;

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

static constexpr const uint8_t BPM = 120;

int main() __attribute__((OS_main));
int main()
{
	sei();
	time::delay_ms(5000);

	GENERATOR generator;
	PLAYER player{generator};
	player.play_sram(music, BPM);
}
