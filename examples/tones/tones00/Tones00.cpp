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
 * Frequency generator examples, used to test all features.
 * 
 * Wiring:
 * - on Arduino UNO (or NANO or ATmega328 chip):
 *   - D6: connect to a 5V passive piezo buzzer with the other lead connected to ground
 * - on Arduino MEGA:
 *   - D13: connect to a 5V passive piezo buzzer with the other lead connected to ground
 * - on Arduino LEONARDO:
 *   - D9: connect to a 5V passive piezo buzzer with the other lead connected to ground
 * - on ATtinyX4:
 *   - D10 (PB2): connect to a 5V passive piezo buzzer with the other lead connected to ground
 * - on ATtinyX5:
 *   - D0 (PB0): connect to a 5V passive piezo buzzer with the other lead connected to ground
 * - on ATmega644 based boards:
 *   - D11 (PB3): connect to a 5V passive piezo buzzer with the other lead connected to ground
 */

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/time.h>
#include <fastarduino/devices/tone_player.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D6_PD6_OC0A;
#elif defined(ARDUINO_MEGA)
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D13_PB7_OC0A;
#elif defined(ARDUINO_LEONARDO)
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D9_PB5_OC1A;
#elif defined(BREADBOARD_ATTINYX4)
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D10_PB2_OC0A;
#elif defined(BREADBOARD_ATTINYX5)
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D0_PB0_OC0A;
#elif defined (BREADBOARD_ATMEGA644P)
static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D11_PB3_OC0A;
#else
#error "Current target is not yet supported!"
#endif

using devices::audio::Tone;
using devices::audio::TonePlay;
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

static const TONEPLAY C_major_scale[] PROGMEM =
{
	TONEPLAY{Tone::C0, QV},
	TONEPLAY{Tone::D0, QV},
	TONEPLAY{Tone::E0, QV},
	TONEPLAY{Tone::F0, QV},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::C1, QV},
	TONEPLAY{Tone::D1, QV},
	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::F1, QV},
	TONEPLAY{Tone::G1, QV},
	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::B1, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::C2, QV},
	TONEPLAY{Tone::D2, QV},
	TONEPLAY{Tone::E2, QV},
	TONEPLAY{Tone::F2, QV},
	TONEPLAY{Tone::G2, QV},
	TONEPLAY{Tone::A2, QV},
	TONEPLAY{Tone::B2, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::C3, QV},
	TONEPLAY{Tone::D3, QV},
	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::F3, QV},
	TONEPLAY{Tone::G3, QV},
	TONEPLAY{Tone::A3, QV},
	TONEPLAY{Tone::B3, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::C4, QV},
	TONEPLAY{Tone::D4, QV},
	TONEPLAY{Tone::E4, QV},
	TONEPLAY{Tone::F4, QV},
	TONEPLAY{Tone::G4, QV},
	TONEPLAY{Tone::A4, QV},
	TONEPLAY{Tone::B4, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{END, 0}
};

static const TONEPLAY D_major_scale[] PROGMEM =
{
	TONEPLAY{Tone::D0, QV},
	TONEPLAY{Tone::E0, QV},
	TONEPLAY{Tone::Fs0, QV},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::Cs1, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::D1, QV},
	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::Fs1, QV},
	TONEPLAY{Tone::G1, QV},
	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::B1, QV},
	TONEPLAY{Tone::Cs2, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::D2, QV},
	TONEPLAY{Tone::E2, QV},
	TONEPLAY{Tone::Fs2, QV},
	TONEPLAY{Tone::G2, QV},
	TONEPLAY{Tone::A2, QV},
	TONEPLAY{Tone::B2, QV},
	TONEPLAY{Tone::Cs3, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::D3, QV},
	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::Fs3, QV},
	TONEPLAY{Tone::G3, QV},
	TONEPLAY{Tone::A3, QV},
	TONEPLAY{Tone::B3, QV},
	TONEPLAY{Tone::Cs4, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::D4, QV},
	TONEPLAY{Tone::E4, QV},
	TONEPLAY{Tone::Fs4, QV},
	TONEPLAY{Tone::G4, QV},
	TONEPLAY{Tone::A4, QV},
	TONEPLAY{Tone::B4, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{END, 0}
};

static const TONEPLAY E_major_scale[] PROGMEM =
{
	TONEPLAY{Tone::E0, QV},
	TONEPLAY{Tone::Fs0, QV},
	TONEPLAY{Tone::Gs0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::Cs1, QV},
	TONEPLAY{Tone::Ds1, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::Fs1, QV},
	TONEPLAY{Tone::Gs1, QV},
	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::B1, QV},
	TONEPLAY{Tone::Cs2, QV},
	TONEPLAY{Tone::Ds2, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::E2, QV},
	TONEPLAY{Tone::Fs2, QV},
	TONEPLAY{Tone::Gs2, QV},
	TONEPLAY{Tone::A2, QV},
	TONEPLAY{Tone::B2, QV},
	TONEPLAY{Tone::Cs3, QV},
	TONEPLAY{Tone::Ds3, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::Fs3, QV},
	TONEPLAY{Tone::Gs3, QV},
	TONEPLAY{Tone::A3, QV},
	TONEPLAY{Tone::B3, QV},
	TONEPLAY{Tone::Cs4, QV},
	TONEPLAY{Tone::Ds4, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::E4, QV},
	TONEPLAY{Tone::Fs4, QV},
	TONEPLAY{Tone::Gs4, QV},
	TONEPLAY{Tone::A4, QV},
	TONEPLAY{Tone::B4, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{END, 0}
};

static const TONEPLAY F_major_scale[] PROGMEM =
{
	TONEPLAY{Tone::F0, QV},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::Bf0, QV},
	TONEPLAY{Tone::C1, QV},
	TONEPLAY{Tone::D1, QV},
	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::F1, QV},
	TONEPLAY{Tone::G1, QV},
	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::Bf1, QV},
	TONEPLAY{Tone::C2, QV},
	TONEPLAY{Tone::D2, QV},
	TONEPLAY{Tone::E2, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::F2, QV},
	TONEPLAY{Tone::G2, QV},
	TONEPLAY{Tone::A2, QV},
	TONEPLAY{Tone::Bf2, QV},
	TONEPLAY{Tone::C3, QV},
	TONEPLAY{Tone::D3, QV},
	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::F3, QV},
	TONEPLAY{Tone::G3, QV},
	TONEPLAY{Tone::A3, QV},
	TONEPLAY{Tone::Bf3, QV},
	TONEPLAY{Tone::C4, QV},
	TONEPLAY{Tone::D4, QV},
	TONEPLAY{Tone::E4, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::F4, QV},
	TONEPLAY{Tone::G4, QV},
	TONEPLAY{Tone::A4, QV},
	TONEPLAY{Tone::Bf4, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{END, 0}
};

static const TONEPLAY G_major_scale[] PROGMEM =
{
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::C1, QV},
	TONEPLAY{Tone::D1, QV},
	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::Fs1, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::G1, QV},
	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::B1, QV},
	TONEPLAY{Tone::C2, QV},
	TONEPLAY{Tone::D2, QV},
	TONEPLAY{Tone::E2, QV},
	TONEPLAY{Tone::Fs2, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::G2, QV},
	TONEPLAY{Tone::A2, QV},
	TONEPLAY{Tone::B2, QV},
	TONEPLAY{Tone::C3, QV},
	TONEPLAY{Tone::D3, QV},
	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::Fs3, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::G3, QV},
	TONEPLAY{Tone::A3, QV},
	TONEPLAY{Tone::B3, QV},
	TONEPLAY{Tone::C4, QV},
	TONEPLAY{Tone::D4, QV},
	TONEPLAY{Tone::E4, QV},
	TONEPLAY{Tone::Fs4, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::G4, QV},
	TONEPLAY{Tone::A4, QV},
	TONEPLAY{Tone::B4, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{END, 0}
};

static const TONEPLAY A_major_scale[] PROGMEM =
{
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::Cs1, QV},
	TONEPLAY{Tone::D1, QV},
	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::Fs1, QV},
	TONEPLAY{Tone::Gs1, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::B1, QV},
	TONEPLAY{Tone::Cs2, QV},
	TONEPLAY{Tone::D2, QV},
	TONEPLAY{Tone::E2, QV},
	TONEPLAY{Tone::Fs2, QV},
	TONEPLAY{Tone::Gs2, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::A2, QV},
	TONEPLAY{Tone::B2, QV},
	TONEPLAY{Tone::Cs3, QV},
	TONEPLAY{Tone::D3, QV},
	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::Fs3, QV},
	TONEPLAY{Tone::Gs3, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::A3, QV},
	TONEPLAY{Tone::B3, QV},
	TONEPLAY{Tone::Cs4, QV},
	TONEPLAY{Tone::D4, QV},
	TONEPLAY{Tone::E4, QV},
	TONEPLAY{Tone::Fs4, QV},
	TONEPLAY{Tone::Gs4, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::A4, QV},
	TONEPLAY{Tone::B4, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{END, 0}
};

static const TONEPLAY B_major_scale[] PROGMEM =
{
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::Cs1, QV},
	TONEPLAY{Tone::Ds1, QV},
	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::Fs1, QV},
	TONEPLAY{Tone::Gs1, QV},
	TONEPLAY{Tone::As1, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::B1, QV},
	TONEPLAY{Tone::Cs2, QV},
	TONEPLAY{Tone::Ds2, QV},
	TONEPLAY{Tone::E2, QV},
	TONEPLAY{Tone::Fs2, QV},
	TONEPLAY{Tone::Gs2, QV},
	TONEPLAY{Tone::As2, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::B2, QV},
	TONEPLAY{Tone::Cs3, QV},
	TONEPLAY{Tone::Ds3, QV},
	TONEPLAY{Tone::E3, QV},
	TONEPLAY{Tone::Fs3, QV},
	TONEPLAY{Tone::Gs3, QV},
	TONEPLAY{Tone::As3, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::B3, QV},
	TONEPLAY{Tone::Cs4, QV},
	TONEPLAY{Tone::Ds4, QV},
	TONEPLAY{Tone::E4, QV},
	TONEPLAY{Tone::Fs4, QV},
	TONEPLAY{Tone::Gs4, QV},
	TONEPLAY{Tone::As4, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::B4, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{END, 0}
};

static const TONEPLAY durations[] PROGMEM =
{
	TONEPLAY{Tone::A1, WN},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{Tone::A1, HN},
	TONEPLAY{Tone::REST, HN},

	TONEPLAY{Tone::A1, QN},
	TONEPLAY{Tone::REST, QN},

	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::A1, SQ},
	TONEPLAY{Tone::REST, SQ},

	TONEPLAY{END, 0}
};

static const TONEPLAY durations_dots[] PROGMEM =
{
	TONEPLAY{Tone::A1, WN},
	TONEPLAY{Tone::REST, WN},
	TONEPLAY{Tone::A1, DOT(WN)},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{Tone::A1, HN},
	TONEPLAY{Tone::REST, HN},
	TONEPLAY{Tone::A1, DOT(HN)},
	TONEPLAY{Tone::REST, HN},

	TONEPLAY{Tone::A1, QN},
	TONEPLAY{Tone::REST, QN},
	TONEPLAY{Tone::A1, DOT(QN)},
	TONEPLAY{Tone::REST, QN},

	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{Tone::A1, DOT(QV)},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{Tone::A1, SQ},
	TONEPLAY{Tone::REST, SQ},
	TONEPLAY{Tone::A1, DOT(SQ)},
	TONEPLAY{Tone::REST, SQ},

	TONEPLAY{END, 0}
};

static const TONEPLAY durations_triplets[] PROGMEM =
{
	TONEPLAY{Tone::A1, WN},
	TONEPLAY{Tone::A1, TRIPLET(HN)},
	TONEPLAY{Tone::A1, TRIPLET(HN)},
	TONEPLAY{Tone::A1, TRIPLET(HN)},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{Tone::A1, HN},
	TONEPLAY{Tone::A1, TRIPLET(QN)},
	TONEPLAY{Tone::A1, TRIPLET(QN)},
	TONEPLAY{Tone::A1, TRIPLET(QN)},
	TONEPLAY{Tone::REST, HN},

	TONEPLAY{Tone::A1, QN},
	TONEPLAY{Tone::A1, TRIPLET(QV)},
	TONEPLAY{Tone::A1, TRIPLET(QV)},
	TONEPLAY{Tone::A1, TRIPLET(QV)},
	TONEPLAY{Tone::REST, QN},

	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::A1, TRIPLET(SQ)},
	TONEPLAY{Tone::A1, TRIPLET(SQ)},
	TONEPLAY{Tone::A1, TRIPLET(SQ)},
	TONEPLAY{Tone::REST, QV},

	TONEPLAY{END, 0}
};

static const TONEPLAY repeat_zero[] PROGMEM =
{
	TONEPLAY{REPEAT_START, 0},
	TONEPLAY{Tone::C0, QV},
	TONEPLAY{Tone::D0, QV},
	TONEPLAY{Tone::E0, QV},
	TONEPLAY{Tone::F0, QV},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{REPEAT_END, 0},

	TONEPLAY{END, 0}
};

static const TONEPLAY repeat_once[] PROGMEM =
{
	TONEPLAY{REPEAT_START, 0},
	TONEPLAY{Tone::C0, QV},
	TONEPLAY{Tone::D0, QV},
	TONEPLAY{Tone::E0, QV},
	TONEPLAY{Tone::F0, QV},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{REPEAT_END, 1},

	TONEPLAY{END, 0}
};

static const TONEPLAY repeat_twice[] PROGMEM =
{
	TONEPLAY{REPEAT_START, 0},
	TONEPLAY{Tone::C0, QV},
	TONEPLAY{Tone::D0, QV},
	TONEPLAY{Tone::E0, QV},
	TONEPLAY{Tone::F0, QV},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{REPEAT_END, 2},

	TONEPLAY{END, 0}
};

static const TONEPLAY two_repeats[] PROGMEM =
{
	TONEPLAY{REPEAT_START, 0},
	TONEPLAY{Tone::C0, QV},
	TONEPLAY{Tone::D0, QV},
	TONEPLAY{Tone::E0, QV},
	TONEPLAY{Tone::F0, QV},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{REPEAT_END, 1},

	TONEPLAY{REPEAT_START, 0},
	TONEPLAY{Tone::C1, QV},
	TONEPLAY{Tone::D1, QV},
	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::F1, QV},
	TONEPLAY{Tone::G1, QV},
	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::B1, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{REPEAT_END, 1},

	TONEPLAY{END, 0}
};

static const TONEPLAY bad_repeats[] PROGMEM =
{
	TONEPLAY{REPEAT_START, 0},
	TONEPLAY{Tone::C0, QV},
	TONEPLAY{Tone::D0, QV},
	TONEPLAY{Tone::E0, QV},
	TONEPLAY{Tone::F0, QV},
	TONEPLAY{Tone::G0, QV},
	TONEPLAY{Tone::A0, QV},
	TONEPLAY{Tone::B0, QV},
	TONEPLAY{Tone::REST, QV},
	TONEPLAY{REPEAT_END, 1},

	// TONEPLAY{REPEAT_START, 0},
	TONEPLAY{Tone::C1, QV},
	TONEPLAY{Tone::D1, QV},
	TONEPLAY{Tone::E1, QV},
	TONEPLAY{Tone::F1, QV},
	TONEPLAY{Tone::G1, QV},
	TONEPLAY{Tone::A1, QV},
	TONEPLAY{Tone::B1, QV},
	TONEPLAY{Tone::REST, QV},
	// There is nothing to repeat (no repeat start active)
	TONEPLAY{REPEAT_END, 1},

	TONEPLAY{Tone::C2, QV},
	TONEPLAY{END, 0}
};

static const TONEPLAY ties[] PROGMEM =
{
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{TIE, 1},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{TIE, 2},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{TIE, 3},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{END, 0}
};

static const TONEPLAY slurs[] PROGMEM =
{
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::D0, QN},
	TONEPLAY{Tone::E0, QN},
	TONEPLAY{Tone::F0, QN},
	TONEPLAY{Tone::G0, QN},
	TONEPLAY{Tone::A0, QN},
	TONEPLAY{Tone::B0, QN},
	TONEPLAY{Tone::C1, QN},
	TONEPLAY{Tone::REST, WN},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{SLUR, 7},
	TONEPLAY{Tone::C0, QN},
	TONEPLAY{Tone::D0, QN},
	TONEPLAY{Tone::E0, QN},
	TONEPLAY{Tone::F0, QN},
	TONEPLAY{Tone::G0, QN},
	TONEPLAY{Tone::A0, QN},
	TONEPLAY{Tone::B0, QN},
	TONEPLAY{Tone::C1, QN},
	TONEPLAY{Tone::REST, WN},
	TONEPLAY{Tone::REST, WN},

	TONEPLAY{END, 0}
};

static constexpr const uint8_t BPM = 60;

int main() __attribute__((OS_main));
int main()
{
	sei();
	time::delay_ms(1000);

	GENERATOR generator;
	PLAYER player{generator};

	// Check scales
	player.play_flash(C_major_scale, BPM);
	time::delay_ms(1000);
	player.play_flash(D_major_scale, BPM);
	time::delay_ms(1000);
	player.play_flash(E_major_scale, BPM);
	time::delay_ms(1000);
	player.play_flash(F_major_scale, BPM);
	time::delay_ms(1000);
	player.play_flash(G_major_scale, BPM);
	time::delay_ms(1000);
	player.play_flash(A_major_scale, BPM);
	time::delay_ms(1000);
	player.play_flash(B_major_scale, BPM);
	time::delay_ms(1000);

	// Check durations, dots, triplets, including rests
	player.play_flash(durations, BPM * 2);
	time::delay_ms(1000);
	player.play_flash(durations_dots, BPM * 2);
	time::delay_ms(1000);
	player.play_flash(durations_triplets, BPM * 2);
	time::delay_ms(1000);

	// Check single and multiple repeats
	player.play_flash(repeat_zero, BPM * 2);
	time::delay_ms(1000);
	player.play_flash(repeat_once, BPM * 2);
	time::delay_ms(1000);
	player.play_flash(repeat_twice, BPM * 2);
	time::delay_ms(1000);
	player.play_flash(two_repeats, BPM * 2);
	time::delay_ms(1000);
	player.play_flash(bad_repeats, BPM * 2);
	time::delay_ms(1000);

	// Check slurs and ties when implemented
	player.play_flash(ties, BPM * 2);
	time::delay_ms(1000);
	player.play_flash(slurs, BPM * 2);
	time::delay_ms(1000);
}
