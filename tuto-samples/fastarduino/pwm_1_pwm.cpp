#include <fastarduino/time.h>
#include <fastarduino/pwm.h>

static constexpr const board::Timer NTIMER = board::Timer::TIMER0;
using TIMER = timer::Timer<NTIMER>;
using CALC = timer::Calculator<NTIMER>;
static constexpr const uint16_t PWM_FREQUENCY = 450;
static constexpr const TIMER::PRESCALER PRESCALER = CALC::FastPWM_prescaler(PWM_FREQUENCY);

static constexpr const board::PWMPin LED = board::PWMPin::D6_PD6_OC0A;
using LED_PWM = analog::PWMOutput<LED>;

int main()
{
	board::init();
	sei();

	// Initialize timer
	TIMER timer{timer::TimerMode::FAST_PWM, PRESCALER};
	timer.begin();
	
	LED_PWM led{timer};
	// Loop of samplings
	while (true)
	{
		for (LED_PWM::TYPE duty = 0; duty < LED_PWM::MAX; ++duty)
		{
			led.set_duty(duty);
			time::delay_ms(50);
		}
		for (LED_PWM::TYPE duty = LED_PWM::MAX; duty > 0; --duty)
		{
			led.set_duty(duty);
			time::delay_ms(50);
		}
	}
	return 0;
}
