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
using namespace eeprom;
using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;
using PLAYER = devices::audio::TonePlayer<NTIMER, OUTPUT>;

using devices::audio::TonePlay;

// TonePlay music[] EEMEM =
const TonePlay music[] PROGMEM =
// TonePlay music[] =
{
	// Intro
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 500},
	{Tone::F1, 350},
	{Tone::C2, 150},
	{Tone::A1, 650},

	// Marker for end of melody
	{Tone::END, 0}
};

int main() __attribute__((OS_main));
int main()
{
	sei();
	time::delay_ms(5000);

	GENERATOR generator;
	PLAYER player{generator};
	// player.play_eeprom(music);
	player.play_flash(music);
	// player.play(music);
}
