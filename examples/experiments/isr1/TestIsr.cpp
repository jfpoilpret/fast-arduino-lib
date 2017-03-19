/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/analog_input.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/pwm.h>

constexpr const board::Timer TIMER0 = board::Timer::TIMER0;
using TCALC0 = timer::Calculator<TIMER0>;
using TPRESCALER0 = TCALC0::TIMER_PRESCALER;

constexpr const board::Timer TIMER1 = board::Timer::TIMER1;
using TCALC1 = timer::Calculator<TIMER1>;
using TPRESCALER1 = TCALC1::TIMER_PRESCALER;

constexpr const uint16_t MAX_PULSE_US = 2000;
constexpr const uint16_t PULSE_FREQUENCY = 50;
constexpr const TPRESCALER0 PRESCALER0 = TCALC0::PulseTimer_prescaler(MAX_PULSE_US, PULSE_FREQUENCY);
constexpr const TPRESCALER1 PRESCALER1 = TCALC1::PulseTimer_prescaler(MAX_PULSE_US, PULSE_FREQUENCY);

using PULSE_TIMER0 = timer::PulseTimer<TIMER0, PRESCALER0>;
using PULSE_TIMER1 = timer::PulseTimer<TIMER1, PRESCALER1>;

//template<board::Timer TIMER, board::DigitalPin PIN> 
//class Servo
//{
//public:
//	//TODO remove eventually
//	Servo(timer::PulseTimer<TIMER>& timer)
//		:	timer_{timer}, out_{timer, timer::TimerOutputMode::NON_INVERTING}, 
//			neutral_{}, minimum_{}, maximum_{}
//	{
//	}
//	
//	Servo(timer::PulseTimer<TIMER>& timer, uint16_t neutral, uint16_t minimum, uint16_t maximum)
//		:	timer_{timer}, out_{timer, timer::TimerOutputMode::NON_INVERTING}, 
//			neutral_{int16_t(neutral)}, minimum_{int16_t(minimum)}, maximum_{int16_t(maximum)}
//	{
//	}
//			
//	~Servo()
//	{
//	}
//	
//	inline void set(uint16_t value)
//	{
//		out_.set_duty(value);
//	}
//
//	inline void rotate(int8_t angle)
//	{
//		uint16_t duty;
//		if (angle > 0)
//			duty = angle * (maximum_ - neutral_) / (MAX - 0) + neutral_;
//		else if (angle < 0)
//			duty = angle * (neutral_ - minimum_) / (0 - MIN) + neutral_;
//		else
//			duty = neutral_;
//		out_.set_duty(duty);
//	}
//
//private:
//	static const int8_t MAX = 127;
//	static const int8_t MIN = -128;
//	
//	timer::PulseTimer<TIMER>& timer_;
//	PIN_TYPE out_;
//	const int16_t neutral_;
//	const int16_t minimum_;
//	const int16_t maximum_;
//};

int main() __attribute__((OS_main));
int main()
{
	PULSE_TIMER0 timer0{PULSE_FREQUENCY};
	timer0._begin();
	PULSE_TIMER1 timer1{PULSE_FREQUENCY};
	timer1._begin();
	sei();
	while (true);
}
