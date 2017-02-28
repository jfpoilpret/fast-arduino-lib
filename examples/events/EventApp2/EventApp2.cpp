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
 * Simple LED chaser. Take #2
 * This program shows usage of FastArduino events support, with Watchdog generated events, and port API.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D0-D7 (port D) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on Arduino MEGA:
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/fast_io.h>
#include <fastarduino/events.h>
#include <fastarduino/watchdog.h>

using namespace events;

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const board::Port LED_PORT = board::Port::PORT_D;
#elif defined (ARDUINO_MEGA)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#else
#error "Current target is not yet supported!"
#endif

// Define vectors we need in the example
REGISTER_WATCHDOG_CLOCK_ISR()

class LedHandler: public EventHandler, private gpio::FastPort<LED_PORT>
{
public:
	LedHandler() : EventHandler{Type::WDT_TIMER}, FastPort{0xFF}, _value{0} {}
	virtual void on_event(UNUSED const Event& event) override
	{
		uint8_t value = _value;
		if (value == 0)
			value = 0x01;
		else
			value <<= 1;
		set_PORT(value);
		_value = value;
	}
	
private:
	uint8_t _value;
};

static const uint8_t EVENT_QUEUE_SIZE = 32;

// Prepare event queue
static Event buffer[EVENT_QUEUE_SIZE];
static containers::Queue<Event> event_queue{buffer};
	
int main()
{
	// Enable interrupts at startup time
	sei();

	// Prepare Dispatcher and Handlers
	Dispatcher dispatcher;
	LedHandler handler;
	dispatcher.insert(handler);
	
	// Start watchdog
	watchdog::Watchdog watchdog{event_queue};
	watchdog.register_watchdog_handler();
	watchdog.begin(watchdog::Watchdog::TimeOut::TO_64ms);
	
	// Event Loop
	while (true)
	{
		Event event = pull(event_queue);
		dispatcher.dispatch(event);
	}
}
