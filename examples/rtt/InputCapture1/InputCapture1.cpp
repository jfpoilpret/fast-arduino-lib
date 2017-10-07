//   Copyright 2016-2017 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/*
 * Input Capture Timer example. Take #1
 * TODO
 * 
 * Wiring:
 * - TODO
 */

#include <fastarduino/flash.h>
#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define TIMER_NUM 1
static constexpr const board::Timer TIMER = board::Timer::TIMER1;

#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(0)

#elif defined (ARDUINO_LEONARDO)
#error "Current target is not yet supported!"
#elif defined (ARDUINO_MEGA)
#error "Current target is not yet supported!"
#elif defined (BREADBOARD_ATTINYX4)
#error "Current target is not yet supported!"
#else
#error "Current target is not yet supported!"
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

// Timer stuff
using TIMER_TYPE = timer::Timer<TIMER>;
using TYPE = TIMER_TYPE::TIMER_TYPE;
static constexpr const board::DigitalPin ICP = TIMER_TYPE::ICP_PIN;
static constexpr const uint32_t PRECISION = 1000UL;
using CALC = timer::Calculator<TIMER>;
static constexpr const TIMER_TYPE::TIMER_PRESCALER PRESCALER = CALC::tick_prescaler(PRECISION);
static_assert(PRESCALER == TIMER_TYPE::TIMER_PRESCALER::DIV_1024, "PRESCALER should be DIV_1024");
static constexpr uint16_t milliseconds(uint16_t ticks)
{
	return ticks * PRECISION / CALC::CTC_frequency(PRESCALER);
}

using timer::TimerInputCapture;
using timer::TimerInterrupt;

class Capture
{
public:
	Capture(TIMER_TYPE& timer):_timer{timer}, _input{gpio::PinMode::INPUT_PULLUP}, _ready{false}, _capture{0} {}

	void on_capture(TYPE capture)
	{
		if (!_input.value())
		{
			// Button pushed, prepare for next capture (on button release)
			_timer._reset();
			_timer.set_input_capture(TimerInputCapture::RISING_EDGE);
		}
		else
		{
			// Button released, stop capture and get captured value
			_timer.set_interrupts({});
			_capture = capture;
			_ready = true;
		}
	}

	void start()
	{
		synchronized
		{
			_ready = false;
			_capture = 0;
			_timer.set_input_capture(TimerInputCapture::FALLING_EDGE);
			_timer.set_interrupts(TimerInterrupt::INPUT_CAPTURE);
			// _timer.set_interrupts(TimerInterrupt::INPUT_CAPTURE | TimerInterrupt::OUTPUT_COMPARE_A);
		}
	}

	TYPE capture()
	{
		while (!_ready) ;
		synchronized return _capture;
	}

private:
	TIMER_TYPE& _timer;
	gpio::FastPinType<ICP>::TYPE _input;
	volatile bool _ready;
	volatile TYPE _capture;
};

REGISTER_TIMER_CAPTURE_ISR_METHOD(TIMER_NUM, Capture, &Capture::on_capture)

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	// Start UART
#if defined (BREADBOARD_ATTINYX4)
	serial::soft::UATX<TX> uatx{output_buffer};
#else
	serial::hard::UATX<UART> uatx{output_buffer};
	uatx.register_handler();
#endif
	uatx.begin(115200);

	streams::FormattedOutput<streams::OutputBuffer> out = uatx.fout();
	out.width(0);
	out << F("Started\n");
	
	TIMER_TYPE timer{timer::TimerMode::NORMAL, PRESCALER};

	timer.begin();
	Capture capture{timer};
	interrupt::register_handler(capture);

	// Event Loop
	while (true)
	{
		out << F("Waiting for button push...\n") << streams::flush;
		capture.start();
		TYPE duration = capture.capture();
		out << F("Push lasted ") << duration << F(" ticks\n") << streams::flush;
		out << F("Push lasted ") << milliseconds(duration) << F(" ms\n") << streams::flush;
	}
}
