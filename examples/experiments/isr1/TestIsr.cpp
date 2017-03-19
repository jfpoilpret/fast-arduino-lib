/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/boards/board.h>
#include <fastarduino/analog_input.h>
#include <fastarduino/time.h>
#include <fastarduino/pulse_timer.h>
#include <fastarduino/pwm.h>
#include <fastarduino/utilities.h>

template<typename TIMER, board::DigitalPin PIN> 
class Servo
{
	using CALC = typename TIMER::CALCULATOR;
	using TPRESCALER = typename CALC::TIMER_PRESCALER;
	static constexpr const TPRESCALER PRESCALER = TIMER::PRESCALER;
	
public:
	using TYPE = typename TIMER::TIMER_TYPE;
	
	Servo(	TIMER& timer, uint16_t us_minimum, uint16_t us_maximum, 
			uint16_t us_neutral = 0)
		:	out_{timer}, 
			US_MINIMUM_{us_minimum}, 
			US_MAXIMUM_{us_maximum}, 
			US_NEUTRAL_{us_neutral ? us_neutral : ((us_maximum + us_minimum) / 2)},
			COUNTER_MINIMUM_{counter(US_MINIMUM_)},
			COUNTER_MAXIMUM_{counter(US_MAXIMUM_)},
			COUNTER_NEUTRAL_{counter(US_NEUTRAL_)} {}
			
	inline void detach() INLINE
	{
		out_.set_duty(0);
	}
	
	inline void set_counter(TYPE value) INLINE
	{
		out_.set_duty(utils::constrain(value, COUNTER_MINIMUM_, COUNTER_MAXIMUM_));
	}
	
	inline void set_pulse(uint16_t pulse_us)
	{
		// Constrain pulse to min/max and convert pulse to timer counter value
		out_.set_duty(counter(utils::constrain(pulse_us, US_MINIMUM_, US_MAXIMUM_)));
	}

	//TODO Better API name?
	inline void rotate(int8_t angle)
	{
		angle = utils::constrain(angle, MIN, MAX);
		TYPE count = (	angle >= 0 ? 
						utils::map(int16_t(angle), 0, int16_t(MAX), COUNTER_NEUTRAL_, COUNTER_MAXIMUM_) :
						utils::map(int16_t(angle), int16_t(MIN), 0, COUNTER_MINIMUM_, COUNTER_NEUTRAL_));
		out_.set_duty(count);
	}

private:
	static constexpr TYPE counter(uint16_t pulse_us)
	{
		return CALC::PulseTimer_value(PRESCALER, pulse_us);
	}
	
	static const int8_t MAX = +90;
	static const int8_t MIN = -90;
	
	analog::PWMOutput<PIN> out_;
	
	const uint16_t US_MINIMUM_;
	const uint16_t US_MAXIMUM_;
	const uint16_t US_NEUTRAL_;
	const TYPE COUNTER_MINIMUM_;
	const TYPE COUNTER_MAXIMUM_;
	const TYPE COUNTER_NEUTRAL_;
};

constexpr const board::Timer TIMER = board::Timer::TIMER0;
using TCALC = timer::Calculator<TIMER>;
using TPRESCALER = TCALC::TIMER_PRESCALER;

constexpr const uint16_t MAX_PULSE_US = 2000;
constexpr const uint16_t MIN_PULSE_US = 1000;
constexpr const uint16_t PULSE_FREQUENCY = 50;
constexpr const TPRESCALER PRESCALER = TCALC::PulseTimer_prescaler(MAX_PULSE_US, PULSE_FREQUENCY);

constexpr const board::DigitalPin SERVO_PIN1 = board::PWMPin::D6_PD6_OC0A;
constexpr const board::AnalogPin POT1 = board::AnalogPin::A1;

using PULSE_TIMER = timer::PulseTimer<TIMER, PRESCALER>;
using SERVO1 = Servo<PULSE_TIMER, SERVO_PIN1>;
using ANALOG1_INPUT = analog::AnalogInput<POT1, board::AnalogReference::AVCC, uint8_t, board::AnalogClock::MAX_FREQ_200KHz>;

// Register ISR needed for PulseTimer (8 bits specific)
//REGISTER_PULSE_TIMER8_AB_ISR(0, PRESCALER, SERVO_PIN1, SERVO_PIN2)
REGISTER_PULSE_TIMER8_A_ISR(0, PRESCALER, SERVO_PIN1)

int main() __attribute__((OS_main));
int main()
{
	PULSE_TIMER servo_timer{PULSE_FREQUENCY};
	SERVO1 servo1{servo_timer, MIN_PULSE_US, MAX_PULSE_US};
	ANALOG1_INPUT pot1;
	servo_timer._begin();
	sei();

//	servo1.detach();
	while (true)
	{
		uint16_t input1 = pot1.sample();
//		servo1.set_counter(input1);
//		servo1.set_pulse(MIN_PULSE_US + input1 * 4);
		int16_t angle = int16_t(input1) - 128;
		servo1.rotate(angle);
		
		time::delay_ms(100);
	}
}
