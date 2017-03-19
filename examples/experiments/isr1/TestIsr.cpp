/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/analog_input.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/pwm.h>

template<	board::Timer TIMER, typename timer::Calculator<TIMER>::TIMER_PRESCALER PRESCALER, 
			typename T = typename board_traits::Timer_trait<TIMER>::TYPE>
class PulseTimer: public timer::Timer<TIMER>
{
public:
	PulseTimer(UNUSED uint16_t pulse_frequency):timer::Timer<TIMER>{0, 0} {}
	inline void begin() {}
	inline void _begin() {}
};

constexpr const board::Timer TIMER = board::Timer::TIMER0;
using TCALC = timer::Calculator<TIMER>;
using TPRESCALER = TCALC::TIMER_PRESCALER;

constexpr const uint16_t MAX_PULSE_US = 2000;
constexpr const uint16_t PULSE_FREQUENCY = 50;
constexpr const TPRESCALER PRESCALER = TCALC::PulseTimer_prescaler(MAX_PULSE_US, PULSE_FREQUENCY);

using PULSE_TIMER = PulseTimer<TIMER, PRESCALER>;

static PULSE_TIMER my_timer{PULSE_FREQUENCY};

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

int main()
{
}
