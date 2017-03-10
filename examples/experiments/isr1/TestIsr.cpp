/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/analog_input.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/pwm.h>

static constexpr const board::AnalogPin POT = board::AnalogPin::A0;
static constexpr const board::Timer TIMER = board::Timer::TIMER1;
static constexpr const board::DigitalPin SERVO_PIN = board::PWMPin::D9_PB1_OC1A;

template<board::Timer TIMER, board::DigitalPin PIN> 
class Servo
{
	using TIMER_TRAIT = board_traits::Timer_trait<TIMER>;
	
public:
	Servo(timer::Timer<TIMER>& timer, uint16_t neutral, uint16_t minimum, uint16_t maximum)
		:	timer_{timer}, out_{timer, timer::TimerOutputMode::NON_INVERTING}, 
			neutral_{int16_t(neutral)}, minimum_{int16_t(minimum)}, maximum_{int16_t(maximum)}
	{
		static_assert(TIMER_TRAIT::MAX_PWM >= 0x3FF, "TIMER must be a 16 bits timer");
		out_.set_duty(neutral_);
	}
	
	inline void set(uint16_t value)
	{
		out_.set_duty(value);
	}

	inline void rotate(int8_t angle)
	{
		uint16_t duty;
		if (angle > 0)
			duty = angle * (maximum_ - neutral_) / (MAX - 0) + neutral_;
		else if (angle < 0)
			duty = angle * (neutral_ - minimum_) / (0 - MIN) + neutral_;
		else
			duty = neutral_;
		out_.set_duty(duty);
	}

private:
	static const int8_t MAX = 127;
	static const int8_t MIN = -128;
	
	timer::Timer<TIMER>& timer_;
	analog::PWMOutput<PIN> out_;
	const int16_t neutral_;
	const int16_t minimum_;
	const int16_t maximum_;
};

using TIMER_TYPE = timer::Timer<TIMER>;

// Frequency for PWM
constexpr const uint16_t PWM_FREQUENCY = 50;
constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = TIMER_TYPE::FastPWM_prescaler(PWM_FREQUENCY);
// Check the actual frequency we get with this timer
constexpr const uint32_t FREQUENCY = TIMER_TYPE::timer_frequency(PRESCALER);

// Given the following servo constants
constexpr const uint16_t MINIMUM_US = 900;
constexpr const uint16_t MAXIMUM_US = 2100;
constexpr const uint16_t NEUTRAL_US = 1500;

// We calculate values for timer OCR
constexpr const uint16_t MINIMUM = MINIMUM_US * FREQUENCY / 1000000UL;
constexpr const uint16_t MAXIMUM = MAXIMUM_US * FREQUENCY / 1000000UL;
constexpr const uint16_t NEUTRAL = NEUTRAL_US * FREQUENCY / 1000000UL;

//TODO Calculate OCR value for minimum
//TODO Calculate ratio to apply uint8_t to get the maximum value at 255

constexpr const uint16_t LOOP_DELAY_MS = 100;

using time::delay_ms;
using timer::TimerMode;

using SERVO1 = Servo<TIMER, SERVO_PIN>;
using ANALOG_INPUT = analog::AnalogInput<POT, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;

int main()
{
	TIMER_TYPE timer{TimerMode::FAST_PWM};
	ANALOG_INPUT pot;
	SERVO1 servo1{timer, NEUTRAL, MINIMUM, MAXIMUM};
	timer._begin(PRESCALER);
	sei();
	
	while (true)
	{
		ANALOG_INPUT::TYPE value = pot.sample();
		int16_t v = value - 128;
		servo1.rotate(v);
//		servo1.rotate(int8_t(value - 128));
//		servo1.set(MINIMUM + value);
		delay_ms(LOOP_DELAY_MS);
	}
}
