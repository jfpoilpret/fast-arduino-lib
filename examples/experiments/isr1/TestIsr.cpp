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
#include <fastarduino/analog_input.h>
#include <fastarduino/pwm.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/utilities.h>

// Board-dependent settings
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin OUTPUT = board::PWMPin::D9_PB1_OC1A;
static constexpr const board::AnalogPin FREQ_INPUT = board::AnalogPin::A0;

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

using FREQ = analog::AnalogInput<FREQ_INPUT, uint8_t>;
using GENERATOR = SquareWave<NTIMER, OUTPUT>;

// Frequency range
static constexpr const uint32_t MIN_FREQ = 40UL;
static constexpr const uint32_t DEFAULT_FREQ = 440UL;
static constexpr const uint32_t MAX_FREQ = 20000UL;

static void tone(TONE t, uint16_t ms)
{
	//TODO
}

int main() __attribute__((OS_main));
int main()
{
	sei();

	GENERATOR generator;

	FREQ input;
	FREQ::SAMPLE_TYPE sample = 0xFF;
	while (true)
	{
		// Read analog pin
		FREQ::SAMPLE_TYPE new_sample = input.sample();
		if (new_sample != sample)
		{
			sample = new_sample;
			// Convert to frequency
			uint32_t frequency = utils::map(sample, uint8_t(0), uint8_t(255), MIN_FREQ, MAX_FREQ);
			// Set new frequency
			generator.start_frequency(frequency);
		}
		// Generate square wave for 1000 milliseconds
		time::delay_ms(1000);
	}
}
