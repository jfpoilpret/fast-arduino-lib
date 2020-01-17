//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * Analog Comparator example.
 * This program shows usage of FastArduino AnalogComparator API.
 * It compares AIN1 with Bandgap and counts time elapsed between two transitions,
 * by using Input Capture connection to Timer1.
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D7 (AIN1): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - direct USB access
 * - on Arduino MEGA:
 *   - D5 (AIN1): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - direct USB access
 * - on ATtinyX4 based boards:
 *   - D1 (PA1, AIN1): connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D8 (PB0): TX conencted to a Serial2USB converter
 */

#include <fastarduino/time.h>
#include <fastarduino/analog_comparator.h>
#include <fastarduino/gpio.h>
#include <fastarduino/timer.h>

#if defined(ARDUINO_UNO) || defined(ARDUINO_NANO) || defined(BREADBOARD_ATMEGA328P)
#define NUM_TIMER 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
#elif defined(ARDUINO_MEGA)
#define NUM_TIMER 4
static constexpr const board::Timer NTIMER = board::Timer::TIMER4;
#define HARDWARE_UART 1
#include <fastarduino/uart.h>
static constexpr const board::USART UART = board::USART::USART0;
REGISTER_UATX_ISR(0)
#elif defined(BREADBOARD_ATTINYX4)
#define NUM_TIMER 1
static constexpr const board::Timer NTIMER = board::Timer::TIMER1;
#define HARDWARE_UART 0
#include <fastarduino/soft_uart.h>
static constexpr const board::DigitalPin TX = board::DigitalPin::D8_PB0;
#else
#error "Current target is not yet supported!"
#endif

// UART for traces
static constexpr const uint8_t OUTPUT_BUFFER_SIZE = 64;
static char output_buffer[OUTPUT_BUFFER_SIZE];

using streams::endl;

using TIMER = timer::Timer<NTIMER>;
using TYPE = TIMER::TYPE;
static constexpr const uint32_t PRECISION = 100000UL;
using CALC = timer::Calculator<NTIMER>;
static constexpr const TIMER::PRESCALER PRESCALER = CALC::tick_prescaler(PRECISION);

static constexpr uint32_t milliseconds(uint32_t ticks, uint32_t overflows)
{
	return (ticks + overflows * TIMER::TIMER_MAX) * 1000UL / CALC::CTC_frequency(PRESCALER);
}

using timer::TimerInputCapture;
using timer::TimerInterrupt;

class Capture
{
public:
	Capture(TIMER& timer, analog::AnalogComparator& comparator)
	: timer_{timer}, comparator_{comparator}, ready_{false}, capture_{0}
	{
		interrupt::register_handler(*this);
	}

	void on_capture(TYPE capture)
	{
		if (!comparator_.output())
		{
			// Button pushed, prepare for next capture (on button release)
			timer_.reset_();
			overflows_ = 0;
			timer_.set_input_capture(TimerInputCapture::RISING_EDGE);
		}
		else
		{
			// Button released, stop capture and get captured value
			timer_.set_interrupts();
			capture_ = capture;
			ready_ = true;
		}
	}

	void on_overflow()
	{
		++overflows_;
	}

	void start()
	{
		synchronized
		{
			ready_ = false;
			capture_ = 0;
			overflows_ = 0;
			timer_.set_input_capture(TimerInputCapture::FALLING_EDGE);
			timer_.set_interrupts(TimerInterrupt::INPUT_CAPTURE | TimerInterrupt::OVERFLOW);
		}
	}

	TYPE capture()
	{
		while (!ready_) ;
		synchronized return capture_;
	}

	uint16_t overflows()
	{
		synchronized return overflows_;
	}

private:
	TIMER& timer_;
	analog::AnalogComparator& comparator_;
	volatile bool ready_;
	volatile TYPE capture_;
	volatile uint16_t overflows_;
};

REGISTER_TIMER_CAPTURE_ISR_METHOD(NUM_TIMER, Capture, &Capture::on_capture)
REGISTER_TIMER_OVERFLOW_ISR_METHOD(NUM_TIMER, Capture, &Capture::on_overflow)

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	#if HARDWARE_UART
	serial::hard::UATX<UART> uart{output_buffer};
	#else
	serial::soft::UATX<TX> uart{output_buffer};
	#endif
	uart.begin(115200);
	streams::ostream out = uart.out();
	out << F("Start") << endl;

	// Declare Timer and Analog comparator
	analog::AnalogComparator comparator;
	TIMER timer{timer::TimerMode::NORMAL, PRESCALER};
	timer.set_capture_noise_canceller(true);
	Capture capture{timer, comparator};

	timer.begin();
	comparator.begin<board::AnalogPin::NONE, true>(analog::ComparatorInterrupt::NONE, true);

	// Event Loop
	while (true)
	{
		out << F("Waiting for pot turn...") << streams::endl;
		capture.start();
		TYPE duration = capture.capture();
		uint16_t overflows = capture.overflows();
		out << F("Lower level lasted ") << duration << F(" ticks, ") << overflows << F(" overflows") << streams::endl;
		out << F("Lower level lasted ") << milliseconds(duration, overflows) << F(" ms") << streams::endl;
	}

	return 0;
}
