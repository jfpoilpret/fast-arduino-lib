/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>
#include <fastarduino/uart.h>

#define TIMER_NUM 1
static constexpr const board::Timer TIMER = board::Timer::TIMER1;
using TIMER_TYPE = timer::Timer<TIMER, true>;
static constexpr const board::DigitalPin ICP = TIMER_TYPE::ICP_PIN;

static constexpr const uint32_t PRECISION = 1000UL;
using CALC = timer::Calculator<TIMER>;
static constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALC::CTC_prescaler(PRECISION);

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];
static serial::hard::UATX<board::USART::USART0> uart{output_buffer};
static streams::FormattedOutput<streams::OutputBuffer> out = uart.fout();

static volatile TIMER_TYPE::TIMER_TYPE _capture;
static volatile uint8_t _captured = 0;

void callback(TIMER_TYPE::TIMER_TYPE capture)
{
	gpio::FastPinType<board::DigitalPin::LED>::toggle();
	_capture = capture;
	++_captured;
}

// Define vectors we need in the example
REGISTER_UATX_ISR(0)
REGISTER_TIMER_CAPTURE_ISR_FUNCTION(TIMER_NUM, callback)

int main() __attribute__((OS_main));
int main()
{
	sei();

	// Start trace
	uart.register_handler();
	uart.begin(115200);
	out.width(0);
	out.base(streams::FormatBase::Base::hex);
	out << "Start\n" << streams::flush;

	gpio::FastPinType<board::DigitalPin::LED>::set_mode(gpio::PinMode::OUTPUT, false);
	gpio::FastPinType<ICP>::set_mode(gpio::PinMode::INPUT_PULLUP);

	// interrupt::register_handler()
	
	// Start timer
	uint8_t captured = 0;
	TIMER_TYPE timer{timer::TimerMode::NORMAL};
	timer.set_input_capture(timer::TimerInputCapture::FALLING_EDGE);
	timer.begin(PRESCALER);
	out << "Timer started. You can press on button.\n" << streams::flush;

	while (true)
	{
		while (captured == _captured);
		TIMER_TYPE::TIMER_TYPE capture = _capture;
		captured = _captured;
		out << "#" << captured << ": " << capture << "\n" << streams::flush;
	}
}
