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
constexpr const uint32_t PERIOD_US = 1000000 / PWM_FREQUENCY;

constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = TIMER_TYPE::prescaler(PERIOD_US);
static_assert(TIMER_TYPE::is_adequate(PRESCALER, PERIOD_US), "TIMER_TYPE::is_adequate(PRESCALER, PERIOD_US)");

constexpr const uint16_t LOOP_DELAY_MS = 10;

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
	
	TIMER_TYPE::TIMER_TYPE duty = 0;
	while (true)
	{
		timer.set_max_A(duty++);
		delay_ms(LOOP_DELAY_MS);
	}
}
