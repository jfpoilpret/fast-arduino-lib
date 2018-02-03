/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/time.h>
#include <fastarduino/devices/tone_player.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin OUTPUT = board::PWMPin::D9_PB1_OC1A;

using devices::audio::Tone;
using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;
using PLAYER = devices::audio::TonePlayer<NTIMER, OUTPUT>;

using devices::audio::TonePlay;

const TonePlay music[] PROGMEM =
{
	// First part
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 650},
	{Tone::NONE, 150},

	// Second part
	{Tone::E2, 500},
	{Tone::E2, 500},
	{Tone::E2, 500},
	{Tone::F2, 350},
	{Tone::C2, 150},
	{Tone::Gs1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 650},
	{Tone::NONE, 150},

	// Third part (repeated once)
	{Tone::REPEAT_START, 0},
	{Tone::A2, 500},
	{Tone::A1, 300},
	{Tone::A1, 150},
	{Tone::A2, 400},
	{Tone::Gs2, 200},	//too short
	{Tone::G2, 200},
	{Tone::Fs2, 125},
	{Tone::F2, 125},
	{Tone::Fs2, 250},
	{Tone::NONE, 250},

	{Tone::As1, 250},
	{Tone::Ds2, 400},
	{Tone::D2, 200},
	{Tone::Cs2, 200},	// this frequency produces "bad buzz" :-)
	{Tone::C2, 125},
	{Tone::B1, 125},
	{Tone::C2, 250},
	{Tone::NONE, 250},

	{Tone::F1, 125},
	{Tone::Gs1, 500},
	{Tone::F1, 375},
	{Tone::A1, 125},
	{Tone::C2, 500},
	{Tone::A1, 375},
	{Tone::C2, 125},
	{Tone::E2, 650},
	{Tone::NONE, 250},
	{Tone::REPEAT_END, 1},
	{Tone::END, 0}
};

int main() __attribute__((OS_main));
int main()
{
	sei();
	time::delay_ms(5000);

	GENERATOR generator;
	PLAYER player{generator};
	player.play_flash(music);
}
