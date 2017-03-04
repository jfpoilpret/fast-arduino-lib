/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/fast_io.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>

constexpr const board::Timer TIMER = board::Timer::TIMER0;
using TIMER_TYPE = timer::Timer<TIMER>;
// Frequency for PWM
constexpr const uint16_t PWM_FREQUENCY = 500;

constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = TIMER_TYPE::PWM_prescaler(PWM_FREQUENCY);
//static_assert(PRESCALER == TIMER_TYPE::TIMER_PRESCALER::DIV_64, "PRESCALER should be /64");

//TODO Timer API to chech adequcy of prescaler for frequency (within some min/max range)
static_assert(TIMER_TYPE::PWM_frequency(PRESCALER) >= 500, "PWM Frequency is expected greater than 450");
static_assert(TIMER_TYPE::PWM_frequency(PRESCALER) < 1000, "PWM Frequency is expected less than 1000");

constexpr const uint16_t LOOP_DELAY_MS = 1000;

using time::delay_ms;
using timer::TimerOutputMode;
using gpio::FastPinType;
using gpio::PinMode;

using LED_PIN = FastPinType<board::PWMPin::D6_PD6_OC0A>::TYPE;

int main()
{
	LED_PIN led{PinMode::OUTPUT};
	TIMER_TYPE timer;
	timer._begin_FastPWM(PRESCALER, TimerOutputMode::NON_INVERTING, TimerOutputMode::DISCONNECTED);
	sei();
	
	timer.set_max_A(1);
	while (true) ;
//	TIMER_TYPE::TIMER_TYPE duty = 0;
//	while (true)
//	{
//		timer.set_max_A(duty++);
//		delay_ms(LOOP_DELAY_MS);
//	}
}
