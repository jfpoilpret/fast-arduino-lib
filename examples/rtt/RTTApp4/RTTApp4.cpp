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
 * Real Time Timer example. Take #4
 * This program shows usage of FastArduino Timer-based RTT (Real Time Timer) support along with periodic job scheduling.
 * The program blinks a LED at a half-period of 5 seconds, forever.
 * 
 * Wiring:
 * - on Arduino UNO and Arduino MEGA:
 *   - no wiring needed as the program uses default LED on D13
 * - on ATmega328P based boards:
 *   - D13 (PB5) connected to a LED through a 330Ohm resistor then linked to GND
 * - on ATtinyX4 based boards:
 *   - D7 (LED, PA7) connected to a LED through a 330Ohm resistor then linked to GND
 */

#include <avr/interrupt.h>

#include <fastarduino/fast_io.h>
#include <fastarduino/realtime_timer.h>
#include <fastarduino/events.h>
#include <fastarduino/scheduler.h>

// Define vectors we need in the example
REGISTER_RTT_ISR_METHOD(0, RTTEventCallback<>, &RTTEventCallback<>::on_rtt_change)

using namespace Events;

static const uint32_t PERIOD = 5000;

class LedHandler: public Job
{
public:
	LedHandler() : Job{0, PERIOD}, _led{PinMode::OUTPUT, false} {}
	virtual void on_schedule(UNUSED uint32_t millis) override
	{
		_led.toggle();
	}
	
private:
	typename FastPinType<Board::DigitalPin::LED>::TYPE _led;
};

// Define event queue
static const uint8_t EVENT_QUEUE_SIZE = 32;
static Event buffer[EVENT_QUEUE_SIZE];
static Queue<Event> event_queue{buffer};

int main()
{
	// Enable interrupts at startup time
	sei();

	RTTEventCallback<> callback{event_queue};
	RTT<Board::Timer::TIMER0> rtt;
	rtt.register_rtt_handler();
	register_handler(callback);
	
	// Prepare Dispatcher and Handlers
	Dispatcher dispatcher;
	Scheduler<RTT<Board::Timer::TIMER0>> scheduler{rtt, Type::RTT_TIMER};
	dispatcher.insert(scheduler);

	LedHandler job;
	scheduler.schedule(job);

	// Start RTT clock
	rtt.begin();

	// Event Loop
	while (true)
	{
		Event event = pull(event_queue);
		dispatcher.dispatch(event);
	}
}
