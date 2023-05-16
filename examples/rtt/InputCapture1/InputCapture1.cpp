//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * Input Capture Timer example.
 * This program demonstrates the use of Timer Input Capture facility by measuring
 * and displaying the time during which a button is pressed.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D8 (PB0, ICP1): connected to a push button connected to GND
 *   - standard USB connected to console for display
 * - on Arduino MEGA:
 *   - D49 (PL0, ICP4): connected to a push button connected to GND
 *   - standard USB connected to console for display
 * - on Arduino LEONARDO:
 *   - D4 (PD4, ICP1): connected to a push button connected to GND
 *   - standard USB connected to console for display
 * - on ATtinyX4 based boards:
 *   - D7 (PA7, ICP): connected to a push button connected to GND
 *   - D1 (PA1, TX): connected through USB Serial converter to console for display
 * - on ATmega644 based boards:
 *   - D30 (PD6, ICP): connected to a push button connected to GND
 *   - D25 (PD1): TX output connected to SerialUSB converter
 */

#include <fastarduino/flash.h>
#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define HAS_UART 1
#define UART_NUM 0
static constexpr const board::USART UART = board::USART::USART0;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#elif defined (ARDUINO_MEGA)
#define HAS_UART 1
#define UART_NUM 0
static constexpr const board::USART UART = board::USART::USART0;
#define TIMER_NUM 4
static constexpr const board::Timer NTIMER = board::Timer::TIMER4;
#elif defined (ARDUINO_LEONARDO)
#define HAS_UART 1
#define UART_NUM 1
static constexpr const board::USART UART = board::USART::USART1;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#elif defined (BREADBOARD_ATTINYX4)
#define HAS_UART 0
static constexpr const board::DigitalPin TX = board::DigitalPin::D1_PA1;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#elif defined (BREADBOARD_ATMEGAXX4P)
#define HAS_UART 1
#define UART_NUM 0
static constexpr const board::USART UART = board::USART::USART0;
#define TIMER_NUM 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#else
#error "Current target is not yet supported!"
#endif

#if HAS_UART
#include <fastarduino/uart.h>
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
// Define vectors we need in the example
REGISTER_UATX_ISR(UART_NUM)
REGISTER_OSTREAMBUF_LISTENERS(serial::hard::UATX<UART>)
#else
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 32;
#include <fastarduino/soft_uart.h>
REGISTER_OSTREAMBUF_LISTENERS(serial::soft::UATX<TX>)
#endif

// Buffers for UART
static char output_buffer[OUTPUT_BUFFER_SIZE];

// Timer stuff
using TIMER = timer::Timer<NTIMER>;
using TYPE = TIMER::TYPE;
static constexpr const board::DigitalPin ICP = TIMER::ICP_PIN;
static constexpr const uint32_t PRECISION = 10000UL;
using CALC = timer::Calculator<NTIMER>;
static constexpr const TIMER::PRESCALER PRESCALER = CALC::tick_prescaler(PRECISION);

static_assert(PRESCALER == TIMER::PRESCALER::DIV_1024, "PRESCALER should be DIV_1024");

static constexpr uint32_t milliseconds(uint32_t ticks, uint32_t overflows)
{
	return (ticks + overflows * TIMER::TIMER_MAX) * 1000UL / CALC::CTC_frequency(PRESCALER);
}

using timer::TimerInputCapture;
using timer::TimerInterrupt;

class Capture
{
public:
	Capture(TIMER& timer):_timer{timer}, _input{gpio::PinMode::INPUT_PULLUP}, _ready{false}, _capture{0} {}

	void on_capture(TYPE capture)
	{
		if (!_input.value())
		{
			// Button pushed, prepare for next capture (on button release)
			_timer.reset_();
			_overflows = 0;
			_timer.set_input_capture(TimerInputCapture::RISING_EDGE);
		}
		else
		{
			// Button released, stop capture and get captured value
			_timer.set_interrupts();
			_capture = capture;
			_ready = true;
		}
	}

	void on_overflow()
	{
		++_overflows;
	}

	void start()
	{
		synchronized
		{
			_ready = false;
			_capture = 0;
			_overflows = 0;
			_timer.set_input_capture(TimerInputCapture::FALLING_EDGE);
			_timer.set_interrupts(TimerInterrupt::INPUT_CAPTURE | TimerInterrupt::OVERFLOW);
		}
	}

	TYPE capture()
	{
		while (!_ready) ;
		synchronized return _capture;
	}

	uint16_t overflows()
	{
		synchronized return _overflows;
	}

private:
	TIMER& _timer;
	gpio::FAST_PIN<ICP> _input;
	volatile bool _ready;
	volatile TYPE _capture;
	volatile uint16_t _overflows;
};

REGISTER_TIMER_CAPTURE_ISR_METHOD(TIMER_NUM, Capture, &Capture::on_capture)
REGISTER_TIMER_OVERFLOW_ISR_METHOD(TIMER_NUM, Capture, &Capture::on_overflow)

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	// Start UART
#if HAS_UART
	serial::hard::UATX<UART> uatx{output_buffer};
#else
	serial::soft::UATX<TX> uatx{output_buffer};
#endif
	uatx.begin(115200);

	streams::ostream out = uatx.out();
	out.width(0);
	out << F("Started\n");
	
	TIMER timer{timer::TimerMode::NORMAL, PRESCALER};

	timer.begin();
	Capture capture{timer};
	interrupt::register_handler(capture);

	// Event Loop
	while (true)
	{
		out << F("Waiting for button push...") << streams::endl;
		capture.start();
		TYPE duration = capture.capture();
		uint16_t overflows = capture.overflows();
		out << F("Push lasted ") << duration << F(" ticks, ") << overflows << F(" overflows") << streams::endl;
		out << F("Push lasted ") << milliseconds(duration, overflows) << F(" ms") << streams::endl;
	}
}
