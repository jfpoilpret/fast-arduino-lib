#include <fastarduino/eeprom.h>
#include <fastarduino/devices/tones.h>
#include <fastarduino/time.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::PWMPin OUTPUT = board::PWMPin::D9_PB1_OC1A;

using devices::audio::Tone;
using namespace eeprom;
using GENERATOR = devices::audio::ToneGenerator<NTIMER, OUTPUT>;

struct TonePlay
{
	Tone tone;
	uint16_t ms;
};

// Melody to be played
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
	{Tone::USER0, 0}
};

int main()
{
	sei();
	GENERATOR generator;
	TonePlay* play = music;
	while (true)
	{
		TonePlay tone;
		EEPROM::read(play, tone);
		if (tone.tone == Tone::USER0)
			break;
		generator.start_tone(tone.tone);
		time::delay_ms(tone.ms);
		generator.stop_tone();
		++play;
	}
}
