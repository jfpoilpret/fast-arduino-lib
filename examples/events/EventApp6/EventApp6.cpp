//   Copyright 2016-2018 Jean-Francois Poilpret
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
 * Simple LED blinker.
 * This program shows usage of FastArduino periodic jobs support, triggered by Watchdog.
 * It simply blinks LED D13 (Arduino UNO).
 */

#include <fastarduino/gpio.h>
#include <fastarduino/events.h>
#include <fastarduino/watchdog.h>
#include <fastarduino/scheduler.h>

using namespace events;
using EVENT = Event<void>;

// Define vectors we need in the example
REGISTER_WATCHDOG_CLOCK_ISR(EVENT)

static const uint32_t PERIOD = 1000;

class LedBlinkerJob: public Job
{
public:
	LedBlinkerJob() : Job{0, PERIOD}, led_{gpio::PinMode::OUTPUT} {}
	virtual void on_schedule(UNUSED uint32_t millis) override
	{
		led_.toggle();
	}
	
private:
	gpio::FastPinType<board::DigitalPin::LED>::TYPE led_;
};

// Define event queue
static const uint8_t EVENT_QUEUE_SIZE = 32;
static EVENT buffer[EVENT_QUEUE_SIZE];
static containers::Queue<EVENT> event_queue{buffer};

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Prepare Dispatcher and Handlers
	Dispatcher<EVENT> dispatcher;
	watchdog::Watchdog<EVENT> watchdog{event_queue};
	watchdog.register_watchdog_handler();
	Scheduler<watchdog::Watchdog<EVENT>, EVENT> scheduler{watchdog, Type::WDT_TIMER};
	dispatcher.insert(scheduler);

	LedBlinkerJob job;
	scheduler.schedule(job);
	
	// Start watchdog
	watchdog.begin(watchdog::TimeOut::TO_64ms);
	
	// Event Loop
	while (true)
	{
		EVENT event = pull(event_queue);
		dispatcher.dispatch(event);
	}
}
