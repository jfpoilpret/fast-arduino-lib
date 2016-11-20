/*
 * Real Time Timer test sample.
 * Uses RTT generated events to schedule a job linking LED.
 */

#include <avr/interrupt.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>
#include <fastarduino/Events.hh>
#include <fastarduino/scheduler.hh>

using namespace Events;

// Define vectors we need in the example
USE_RTT_TIMER0()

static const uint32_t PERIOD = 5000;

class LedHandler: public Job, private FastPinType<Board::DigitalPin::LED>::TYPE
{
public:
	LedHandler() : Job{0, PERIOD}, FastPinType<Board::DigitalPin::LED>::TYPE{PinMode::OUTPUT, false} {}
	virtual void on_schedule(UNUSED uint32_t millis) override
	{
		toggle();
	}
};

// Define event queue
static const uint8_t EVENT_QUEUE_SIZE = 32;
static Event buffer[EVENT_QUEUE_SIZE];
static Queue<Event> event_queue{buffer};

int main()
{
	// Enable interrupts at startup time
	sei();

	RTTEventCallback callback{event_queue};
	RTT<Board::Timer::TIMER0> rtt;
	rtt.set_callback(&callback);
	
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
