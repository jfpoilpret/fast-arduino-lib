/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/eeprom.h>
#include <fastarduino/time.h>
#include <fastarduino/devices/tones.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin OUTPUT = board::PWMPin::D9_PB1_OC1A;

using devices::audio::Tone;
using namespace eeprom;
using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;

struct TonePlay
{
	Tone tone;
	uint16_t ms;
};

TonePlay music[] EEMEM =
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
	TonePlay* play = music;
	while (true)
	{
		TonePlay tone;
		EEPROM::read(play, tone);
		if (tone.tone == Tone::END)
			break;
		generator.tone(tone.tone, tone.ms);
		++play;
	}
}
