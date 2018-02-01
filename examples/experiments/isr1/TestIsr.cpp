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

using FREQ = analog::AnalogInput<FREQ_INPUT, uint8_t>;
using CALC = timer::Calculator<NTIMER>;
using TIMER = timer::Timer<NTIMER>;
using PWMPIN = analog::PWMOutput<OUTPUT>;

// Frequency range
static constexpr const uint32_t MIN_FREQ = 40UL;
static constexpr const uint32_t DEFAULT_FREQ = 440UL;
static constexpr const uint32_t MAX_FREQ = 20000UL;

static void init_frequency(TIMER& timer, PWMPIN& output, uint32_t frequency)
{
	timer.suspend();
	const uint32_t period = 1000000UL / 2 / frequency;
	TIMER::PRESCALER prescaler = CALC::CTC_prescaler(period);
	TIMER::TYPE counter = CALC::CTC_counter(prescaler, period);
	timer.set_prescaler(prescaler);
	output.set_duty(counter);
	timer.resume();
}

int main() __attribute__((OS_main));
int main()
{
	sei();

	FREQ input;
	TIMER timer{timer::TimerMode::CTC, TIMER::PRESCALER::NO_PRESCALING};
	PWMPIN output = PWMPIN{timer, timer::TimerOutputMode::TOGGLE};
	timer.begin();

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
			init_frequency(timer, output, frequency);
		}
		// Generate square wave for 100 milliseconds
		time::delay_ms(1000);
	}
}
