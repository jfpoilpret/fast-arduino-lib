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

//REGISTER_PULSE_TIMER_ISR(1)

template<board::Timer TIMER, board::DigitalPin PIN> 
class Servo
{
	using TIMER_TRAIT = board_traits::Timer_trait<TIMER>;
	using PIN_TYPE = analog::PWMOutput<PIN>;
	
public:
	//TODO remove eventually
	Servo(timer::PulseTimer<TIMER>& timer)
		:	timer_{timer}, out_{timer, timer::TimerOutputMode::NON_INVERTING}, 
			neutral_{}, minimum_{}, maximum_{}
	{
		timer_.register_pin(PIN_TYPE::COM);
		out_.set_duty(neutral_);
	}
	
	Servo(timer::PulseTimer<TIMER>& timer, uint16_t neutral, uint16_t minimum, uint16_t maximum)
		:	timer_{timer}, out_{timer, timer::TimerOutputMode::NON_INVERTING}, 
			neutral_{int16_t(neutral)}, minimum_{int16_t(minimum)}, maximum_{int16_t(maximum)}
	{
		timer_.register_pin(PIN_TYPE::COM);
		out_.set_duty(neutral_);
	}
			
	~Servo()
	{
		timer_.unregister_pin(PIN_TYPE::COM);
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
	
	timer::PulseTimer<TIMER>& timer_;
	PIN_TYPE out_;
	const int16_t neutral_;
	const int16_t minimum_;
	const int16_t maximum_;
};

using TIMER_TYPE = timer::PulseTimer<TIMER>;

// Interval between 2 pulses
constexpr const uint16_t PULSE_FREQUENCY = 50;

//DEBUG CHECK PRESCALER and COUNTER
//static_assert(TIMER_TYPE::PWM_ICR_prescaler(PULSE_FREQUENCY) == TIMER_TYPE::TIMER_PRESCALER::DIV_1024, "");
//static_assert(TIMER_TYPE::PWM_ICR_prescaler(PULSE_FREQUENCY) == TIMER_TYPE::TIMER_PRESCALER::DIV_256, "");
//static_assert(TIMER_TYPE::PWM_ICR_prescaler(PULSE_FREQUENCY) == TIMER_TYPE::TIMER_PRESCALER::DIV_64, "");
static_assert(TIMER_TYPE::PWM_ICR_prescaler(PULSE_FREQUENCY) == TIMER_TYPE::TIMER_PRESCALER::DIV_8, "");
//static_assert(TIMER_TYPE::PWM_ICR_prescaler(PULSE_FREQUENCY) == TIMER_TYPE::TIMER_PRESCALER::NO_PRESCALING, "");
static_assert(TIMER_TYPE::PWM_ICR_counter(PULSE_FREQUENCY) > 39990, "");
static_assert(TIMER_TYPE::PWM_ICR_counter(PULSE_FREQUENCY) < 40010, "");
//static_assert(TIMER_TYPE::PWM_ICR_counter(PULSE_FREQUENCY) == 33920, "");

static_assert(TIMER_TYPE::PWM_ICR_frequency(PULSE_FREQUENCY) > 40, "");
	static_assert(TIMER_TYPE::PWM_ICR_frequency(PULSE_FREQUENCY) < 60, "");

// Given the following servo constants
constexpr const uint16_t MINIMUM_US = 900;
constexpr const uint16_t MAXIMUM_US = 2100;
constexpr const uint16_t NEUTRAL_US = 1500;
//constexpr const uint16_t MINIMUM_US = 400;
//constexpr const uint16_t MAXIMUM_US = 2500;
//constexpr const uint16_t NEUTRAL_US = 1500;

constexpr const uint16_t LOOP_DELAY_MS = 100;

using time::delay_ms;
using timer::TimerMode;

using SERVO1 = Servo<TIMER, SERVO_PIN>;
using ANALOG_INPUT = analog::AnalogInput<POT, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;

int main()
{
	TIMER_TYPE timer{MAXIMUM_US, PULSE_FREQUENCY};
	ANALOG_INPUT pot;
	SERVO1 servo1{timer};
	timer._begin();
	sei();
	
	while (true)
	{
		ANALOG_INPUT::TYPE value = pot.sample();
		TIMER_TYPE::TIMER_TYPE value2 = value;
		if (sizeof(TIMER_TYPE::TIMER_TYPE) == 2)
			value2 <<= 3;
		servo1.set(value2);
//		int16_t v = value - 128;
//		servo1.rotate(v);
//		servo1.rotate(int8_t(value - 128));
//		servo1.set(MINIMUM + value);
		delay_ms(LOOP_DELAY_MS);
	}
}
