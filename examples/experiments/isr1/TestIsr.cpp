/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/time.h>
#include <fastarduino/timer.h>
#include <fastarduino/pwm.h>

// Define here which PWM mode you want to test, and which timer you want to use
#define FASTPWM 0
#define TIMER_NUM 5

#if TIMER_NUM == 0
constexpr const board::Timer TIMER = board::Timer::TIMER0;
#elif TIMER_NUM == 1
constexpr const board::Timer TIMER = board::Timer::TIMER1;
#elif TIMER_NUM == 2
constexpr const board::Timer TIMER = board::Timer::TIMER2;
#elif TIMER_NUM == 3
constexpr const board::Timer TIMER = board::Timer::TIMER3;
#elif TIMER_NUM == 4
constexpr const board::Timer TIMER = board::Timer::TIMER4;
#elif TIMER_NUM == 5
constexpr const board::Timer TIMER = board::Timer::TIMER5;
#endif
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
// Check the actual PWM frequency we get with this timer
static_assert(COMPUTE_PWM_FREQUENCY(PRESCALER) >= 450, "PWM Frequency is expected greater than 450");
static_assert(COMPUTE_PWM_FREQUENCY(PRESCALER) < 2000, "PWM Frequency is expected less than 1000");

constexpr const uint16_t LOOP_DELAY_MS = 1000;

using time::delay_ms;
using timer::TimerMode;
using timer::TimerOutputMode;
using analog::PWMOutput;

#if TIMER_NUM == 0
using LED1_PIN = PWMOutput<board::PWMPin::D13_PB7_OC0A>;
using LED2_PIN = PWMOutput<board::PWMPin::D4_PG5_OC0B>;
#elif TIMER_NUM == 1
using LED1_PIN = PWMOutput<board::PWMPin::D11_PB5_OC1A>;
using LED2_PIN = PWMOutput<board::PWMPin::D12_PB6_OC1B>;
#elif TIMER_NUM == 2
using LED1_PIN = PWMOutput<board::PWMPin::D10_PB4_OC2A>;
using LED2_PIN = PWMOutput<board::PWMPin::D9_PH6_OC2B>;
#elif TIMER_NUM == 3
using LED1_PIN = PWMOutput<board::PWMPin::D2_PE4_OC3B>;
using LED2_PIN = PWMOutput<board::PWMPin::D3_PE5_OC3C>;
#elif TIMER_NUM == 4
using LED1_PIN = PWMOutput<board::PWMPin::D6_PH3_OC4A>;
using LED2_PIN = PWMOutput<board::PWMPin::D7_PH4_OC4B>;
#elif TIMER_NUM == 5
using LED1_PIN = PWMOutput<board::PWMPin::D46_PL3_OC5A>;
using LED2_PIN = PWMOutput<board::PWMPin::D44_PL5_OC5C>;
#endif

int main()
{
	TIMER_TYPE timer{TIMER_MODE};
	LED1_PIN led1{timer};
	LED2_PIN led2{timer};
	timer._begin(PRESCALER);
	sei();
	
	LED1_PIN::TYPE duty1 = 0;
	LED2_PIN::TYPE duty2 = LED2_PIN::MAX;
	while (true)
	{
		led1.set_duty(duty1++);
		led2.set_duty(duty2--);
		if (duty1 > LED1_PIN::MAX) duty1 = 0;
		if (duty2 > LED2_PIN::MAX) duty2 = LED2_PIN::MAX;
		delay_ms(LOOP_DELAY_MS);
	}
}
