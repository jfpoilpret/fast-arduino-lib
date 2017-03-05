/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/fast_io.h>
#include <fastarduino/time.h>
#include <fastarduino/timer.h>

#define FASTPWM 1

//constexpr const board::Timer TIMER = board::Timer::TIMER0;
//constexpr const board::Timer TIMER = board::Timer::TIMER2;
constexpr const board::Timer TIMER = board::Timer::TIMER1;
using TIMER_TYPE = timer::Timer<TIMER>;

#if FASTPWM
#define COMPUTE_PWM_PRESCALER TIMER_TYPE::FastPWM_prescaler
#define COMPUTE_PWM_FREQUENCY TIMER_TYPE::FastPWM_frequency
#define TIMER_MODE TimerMode::FAST_PWM
#else
#define COMPUTE_PWM_PRESCALER TIMER_TYPE::PhaseCorrectPWM_prescaler
#define COMPUTE_PWM_FREQUENCY TIMER_TYPE::PhaseCorrectPWM_frequency
#define TIMER_MODE TimerMode::PHASE_CORRECT_PWM
#endif

// Frequency for PWM
constexpr const uint16_t PWM_FREQUENCY = 450;
constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = COMPUTE_PWM_PRESCALER(PWM_FREQUENCY);
//TODO Timer API to check adequacy of prescaler for frequency (within some min/max range)
static_assert(COMPUTE_PWM_FREQUENCY(PRESCALER) >= 450, "PWM Frequency is expected greater than 450");
static_assert(COMPUTE_PWM_FREQUENCY(PRESCALER) < 2000, "PWM Frequency is expected less than 1000");

constexpr const uint16_t LOOP_DELAY_MS = 1000;

using time::delay_ms;
using timer::TimerMode;
using timer::TimerOutputMode;
using gpio::FastPinType;
using gpio::PinMode;

//using LED1_PIN = FastPinType<board::PWMPin::D6_PD6_OC0A>::TYPE;
//using LED2_PIN = FastPinType<board::PWMPin::D5_PD5_OC0B>::TYPE;
//using LED1_PIN = FastPinType<board::PWMPin::D11_PB3_OC2A>::TYPE;
//using LED2_PIN = FastPinType<board::PWMPin::D3_PD3_OC2B>::TYPE;
using LED1_PIN = FastPinType<board::PWMPin::D9_PB1_OC1A>::TYPE;
using LED2_PIN = FastPinType<board::PWMPin::D10_PB2_OC1B>::TYPE;

int main()
{
	LED1_PIN led1{PinMode::OUTPUT};
	LED2_PIN led2{PinMode::OUTPUT};
	TIMER_TYPE timer{TIMER_MODE, TimerOutputMode::NON_INVERTING, TimerOutputMode::NON_INVERTING};
	timer._begin(PRESCALER);
	sei();
	
	TIMER_TYPE::TIMER_TYPE duty1 = 0;
	TIMER_TYPE::TIMER_TYPE duty2 = TIMER_TYPE::PWM_MAX;
	while (true)
	{
		timer.set_max_A(duty1++);
		timer.set_max_B(duty2--);
		if (duty1 > TIMER_TYPE::PWM_MAX) duty1 = 0;
		if (duty2 > TIMER_TYPE::PWM_MAX) duty2 = TIMER_TYPE::PWM_MAX;
		delay_ms(LOOP_DELAY_MS);
	}
}
