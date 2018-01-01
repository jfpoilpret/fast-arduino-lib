/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/devices/hcsr04.h>
#include <fastarduino/time.h>
#include <fastarduino/uart.h>

#define TIMER_NUM 1
static constexpr const board::Timer TIMER = board::Timer::TIMER1;
using TIMER_TYPE = timer::Timer<TIMER>;
static constexpr const board::DigitalPin TRIGGER = board::DigitalPin::D2;
static constexpr const board::DigitalPin ECHO = TIMER_TYPE::ICP_PIN;

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<board::USART::USART0> uart{output_buffer};
static streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

using SONAR = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, devices::sonar::SonarType::ASYNC_ICP>;
// using SONAR = devices::sonar::HCSR04<TIMER, TRIGGER, ECHO, false>;
static constexpr const uint32_t PRECISION = SONAR::DEFAULT_TIMEOUT_MS * 1000UL;
using CALC = timer::Calculator<TIMER>;
static constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALC::CTC_prescaler(PRECISION);

static constexpr const SONAR::TYPE TIMEOUT = CALC::us_to_ticks(PRESCALER, PRECISION);

// Define vectors we need in the example
REGISTER_UATX_ISR(0)
REGISTER_HCSR04_ICP_ISR(TIMER_NUM, TRIGGER, ECHO)

int main() __attribute__((OS_main));
int main()
{
	sei();

	// Start trace
	uart.register_handler();
	uart.begin(115200);
	out.width(0);
	out << "Start" << streams::endl;

	// Start timer
	TIMER_TYPE timer{timer::TimerMode::NORMAL, PRESCALER, timer::TimerInterrupt::INPUT_CAPTURE};
	// TIMER_TYPE timer{timer::TimerMode::NORMAL, PRESCALER};
	timer.begin();
	out << "Timer started." << streams::endl;

	SONAR sonar{timer};
	sonar.register_handler();
	time::delay_ms(5000);
	while (true)
	{
		out << "#1" << streams::endl;
		sonar.async_echo();
		out << "#2" << streams::endl;
		SONAR::TYPE echo = sonar.await_echo_ticks(TIMEOUT);
		// SONAR::TYPE echo = sonar.echo_ticks(TIMEOUT);
		out << "#3" << streams::endl;
		uint32_t us = CALC::ticks_to_us(PRESCALER, echo);
		uint16_t distance = devices::sonar::echo_us_to_distance_mm(us);
		out << "# " << echo << " ticks, " << us << "us, " << distance << "mm" << streams::endl;
		time::delay_ms(500);
	}
}
