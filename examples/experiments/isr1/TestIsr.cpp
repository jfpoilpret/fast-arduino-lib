/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

//TODO try it with a buzzer (direct wire or amplified?)
//TODO write a SquareWave class or namespace with utility methods
//TODO create simple melody
//TODO store melody to EEPROM and read it from there

// Example of square wave generation, using CTC mode and COM toggle
#include <fastarduino/gpio.h>
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
	}

private:
	TIMER timer_;
	PWMPIN output_;
};

enum class TONE: uint16_t
{
	C = 262,
	Cs = 277,
	D = 294,
	Ds = 311,
	E = 330,
	F = 349,
	Fs = 370,
	G = 392,
	Gs = 415,
	A = 440,
	As = 466,
	B = 494
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

	void tone(TONE t, uint16_t ms)
	{
		generator_.start_frequency(uint32_t(t));
		if (ms != 0)
		{
			time::delay_ms(ms);
			generator_.stop();
		}
	}

	void tone(TONE t, int8_t octave, uint16_t ms)
	{
		uint32_t frequency = uint32_t(t);
		if (octave < 0)
			frequency >>= -octave;
		else if (octave > 0)
			frequency <<= octave;
		generator_.start_frequency(frequency);
		if (ms != 0)
		{
			time::delay_ms(ms);
			generator_.stop();
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

int main() __attribute__((OS_main));
int main()
{
	sei();

	gpio::FastPinType<board::DigitalPin::LED>::set_mode(gpio::PinMode::OUTPUT);
	gpio::FastPinType<board::DigitalPin::LED>::set();
	time::delay_ms(5000);
	gpio::FastPinType<board::DigitalPin::LED>::clear();

	GENERATOR generator;

	while (true)
	{
		generator.tone(TONE::C, DEFAULT_DURATION_MS);
		generator.tone(TONE::D, DEFAULT_DURATION_MS);
		generator.tone(TONE::E, DEFAULT_DURATION_MS);
		generator.tone(TONE::F, DEFAULT_DURATION_MS);
		generator.tone(TONE::G, DEFAULT_DURATION_MS);
		generator.tone(TONE::A, DEFAULT_DURATION_MS);
		generator.tone(TONE::B, DEFAULT_DURATION_MS);
		time::delay_ms(1000);
		generator.tone(TONE::C, 1, DEFAULT_DURATION_MS);
		generator.tone(TONE::D, 1, DEFAULT_DURATION_MS);
		generator.tone(TONE::E, 1, DEFAULT_DURATION_MS);
		generator.tone(TONE::F, 1, DEFAULT_DURATION_MS);
		generator.tone(TONE::G, 1, DEFAULT_DURATION_MS);
		generator.tone(TONE::A, 1, DEFAULT_DURATION_MS);
		generator.tone(TONE::B, 1, DEFAULT_DURATION_MS);
		time::delay_ms(1000);
	}
}
