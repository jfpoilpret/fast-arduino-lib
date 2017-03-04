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
const bool FAST_PWM = false;
//const bool FAST_PWM = true;
constexpr const uint16_t PWM_FREQUENCY = 450;
constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = TIMER_TYPE::PWM_prescaler(PWM_FREQUENCY, FAST_PWM);
//TODO Timer API to check adequacy of prescaler for frequency (within some min/max range)
static_assert(TIMER_TYPE::PWM_frequency(PRESCALER, FAST_PWM) >= 450, "PWM Frequency is expected greater than 450");
static_assert(TIMER_TYPE::PWM_frequency(PRESCALER, FAST_PWM) < 1000, "PWM Frequency is expected less than 1000");

constexpr const uint16_t LOOP_DELAY_MS = 1000;

using time::delay_ms;
using timer::TimerOutputMode;
using gpio::FastPinType;
using gpio::PinMode;

using LED1_PIN = FastPinType<board::PWMPin::D6_PD6_OC0A>::TYPE;
using LED2_PIN = FastPinType<board::PWMPin::D5_PD5_OC0B>::TYPE;

int main()
{
	LED1_PIN led1{PinMode::OUTPUT};
	LED2_PIN led2{PinMode::OUTPUT};
	TIMER_TYPE timer{TimerOutputMode::NON_INVERTING, TimerOutputMode::NON_INVERTING};
//	timer._begin_FastPWM(PRESCALER);
	timer._begin_PhaseCorrectPWM(PRESCALER);
	sei();
	
	TIMER_TYPE::TIMER_TYPE duty1 = 0;
	TIMER_TYPE::TIMER_TYPE duty2 = 0xFF;
	while (true)
	{
		timer.set_max_A(duty1++);
		timer.set_max_B(duty2--);
		delay_ms(LOOP_DELAY_MS);
	}
}
