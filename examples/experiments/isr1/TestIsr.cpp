/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

//TODO improve Tone to include repeat "loops"
//TODO store melody to EEPROM and read it from there
//TODO add generic stuff (SquareWaveGenerator, ToneGenerator... to FastArduino core library)
// Imperial march tones thanks:
// http://processors.wiki.ti.com/index.php/Playing_The_Imperial_March

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/pwm.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/utilities.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin OUTPUT = board::PWMPin::D9_PB1_OC1A;

template<board::Timer NTIMER_, board::DigitalPin OUTPUT_>
class SquareWave
{
public:
	static constexpr const board::Timer NTIMER = NTIMER_;
	static constexpr const board::DigitalPin OUTPUT = OUTPUT_;

	using CALC = timer::Calculator<NTIMER>;
	using TIMER = timer::Timer<NTIMER>;
	using PWMPIN = analog::PWMOutput<OUTPUT>;

	SquareWave()
		:timer_{timer::TimerMode::CTC, TIMER::PRESCALER::NO_PRESCALING}, output_{timer_, timer::TimerOutputMode::TOGGLE}
	{
	}

	TIMER& timer() const
	{
		return timer_;
	}
	
	void start_frequency(uint32_t frequency)
	{
		timer_.end();
		const uint32_t period = 1000000UL / 2 / frequency;
		typename TIMER::PRESCALER prescaler = CALC::CTC_prescaler(period);
		typename TIMER::TYPE counter = CALC::CTC_counter(prescaler, period);
		timer_.set_prescaler(prescaler);
		timer_.begin();
		output_.set_duty(counter);
	}

	void stop()
	{
		timer_.end();
		output_.set_duty(0);
	}

private:
	TIMER timer_;
	PWMPIN output_;
};

enum class Tone: uint16_t
{
	NONE = 0,
	//TODO special "tones" for repeating measures
	// LABEL = 1,
	// GOTO = 2,

	C0 = 131,
	Cs0 = 139,
	D0 = 147,
	Ds0 = 156,
	E0 = 165,
	F0 = 175,
	Fs0 = 185,
	G0 = 196,
	Gs0 = 208,
	A0 = 220,
	As0 = 233,
	B0 = 247,

	C1 = 262,
	Cs1 = 277,
	D1 = 294,
	Ds1 = 311,
	E1 = 330,
	F1 = 349,
	Fs1 = 370,
	G1 = 392,
	Gs1 = 415,
	A1 = 440,
	As1 = 466,
	B1 = 494,

	C2 = 523,
	Cs2 = 554,
	D2 = 587,
	Ds2 = 622,
	E2 = 659,
	F2 = 698,
	Fs2 = 740,
	G2 = 784,
	Gs2 = 831,
	A2 = 880,
	As2 = 932,
	B2 = 988,

	C3 = 1046,
	Cs3 = 1109,
	D3 = 1175,
	Ds3 = 1245,
	E3 = 1319,
	F3 = 1397,
	Fs3 = 1480,
	G3 = 1568,
	Gs3 = 1662,
	A3 = 1760,
	As3 = 1865,
	B3 = 1976,
};

template<board::Timer NTIMER, board::DigitalPin OUTPUT>
class ToneGenerator
{
private:
	using GENERATOR = SquareWave<NTIMER, OUTPUT>;

public:
	ToneGenerator():generator_{}
	{
	}

	void tone(Tone t, uint16_t ms)
	{
		generator_.start_frequency(uint32_t(t));
		if (ms != 0)
		{
			time::delay_ms(ms);
			generator_.stop();
			// Short delay between tones
			time::delay_ms(20);
		}
	}

	void no_tone()
	{
		generator_.stop();
	}

private:
	GENERATOR generator_;
};

using GENERATOR = ToneGenerator<NTIMER, OUTPUT>;
static constexpr const uint16_t DEFAULT_DURATION_MS = 1000;

struct TonePlay
{
	Tone tone;
	uint16_t ms;
};

static TonePlay music[] =
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
	{Tone::A2, 500},
	{Tone::A1, 300},
	{Tone::A1, 150},
	{Tone::A2, 400},
	{Tone::Gs2, 200},
	{Tone::G2, 200},
	{Tone::Fs2, 125},
	{Tone::F2, 125},
	{Tone::Fs2, 250},
	{Tone::NONE, 250},

	{Tone::As1, 250},
	{Tone::Ds2, 400},
	{Tone::D2, 200},
	{Tone::Cs2, 200},
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

	// Third part (second time)
	{Tone::A2, 500},
	{Tone::A1, 300},
	{Tone::A1, 150},
	{Tone::A2, 400},
	{Tone::Gs2, 200},
	{Tone::G2, 200},
	{Tone::Fs2, 125},
	{Tone::F2, 125},
	{Tone::Fs2, 250},
	{Tone::NONE, 250},

	{Tone::As1, 250},
	{Tone::Ds2, 400},
	{Tone::D2, 200},
	{Tone::Cs2, 200},
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
};

static constexpr const size_t NUM_TONES = sizeof music / sizeof music[0];

int main() __attribute__((OS_main));
int main()
{
	sei();
	time::delay_ms(5000);

	GENERATOR generator;
	while (true)
	{
		for (size_t i = 0; i < NUM_TONES; ++i)
		{
			TonePlay tone = music[i];
			if (tone.tone == Tone::NONE)
				time::delay_ms(tone.ms);
			else
				generator.tone(tone.tone, tone.ms);
		}

		time::delay_ms(2000);
	}
}
