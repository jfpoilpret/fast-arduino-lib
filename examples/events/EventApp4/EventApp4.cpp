//   Copyright 2016-2021 Jean-Francois Poilpret
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
 * Simple LED chaser. Take #4
 * This program shows usage of FastArduino periodic jobs support, triggered by 
 * Watchdog, and GPIO port API.
 * It is similar to EventApp3 except that it consumes less power, by spending 
 * most time in POWER DOWN mode.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D0-D7 (port D) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on Arduino LEONARDO:
 *	- D3-D2-D0-D1-D4-TXLED-D12-D6 (port D) branch 8 LED (except for TXLED) in series with 330 Ohm resistors
 * - on Arduino MEGA:
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATmega644 based boards:
 *   - D0-D7 (port A): branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 */

#include <fastarduino/gpio.h>
#include <fastarduino/events.h>
#include <fastarduino/watchdog.h>
#include <fastarduino/scheduler.h>
#include <fastarduino/power.h>

using namespace events;

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO) || defined(ARDUINO_LEONARDO)
static constexpr const board::Port LED_PORT = board::Port::PORT_D;
#elif defined (ARDUINO_MEGA)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#elif defined (BREADBOARD_ATMEGAXX4P)
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
#else
#error "Current target is not yet supported!"
#endif

using EVENT = Event<void>;

// Define vectors we need in the example
REGISTER_WATCHDOG_CLOCK_ISR(EVENT)

static const uint32_t PERIOD = 1000;

class LedHandler: public Job, private gpio::FastPort<LED_PORT>
{
public:
	LedHandler() : Job{0, PERIOD}, FastPort{0xFF}, _value{0} {}

protected:
	virtual void on_schedule(UNUSED uint32_t millis) override
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

// Define event queue
static const uint8_t EVENT_QUEUE_SIZE = 32;
static EVENT buffer[EVENT_QUEUE_SIZE];
static containers::Queue<EVENT> event_queue{buffer};

int main()
{
	board::init();
	// Set power settings
	power::Power::set_default_mode(board::SleepMode::POWER_DOWN);
	// Enable interrupts at startup time
	sei();

	// Prepare Dispatcher and Handlers
	Dispatcher<EVENT> dispatcher;
	watchdog::Watchdog<EVENT> watchdog{event_queue};
	Scheduler<watchdog::Watchdog<EVENT>, EVENT> scheduler{watchdog, Type::WDT_TIMER};
	dispatcher.insert(scheduler);

	LedHandler job;
	scheduler.schedule(job);
	
	// Start watchdog
	watchdog.begin(watchdog::TimeOut::TO_125ms);
	
	// Event Loop
	while (true)
	{
		EVENT event = pull(event_queue);
		dispatcher.dispatch(event);
	}
}
