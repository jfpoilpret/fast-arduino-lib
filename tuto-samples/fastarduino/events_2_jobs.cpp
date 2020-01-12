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

protected:
	virtual void on_schedule(UNUSED uint32_t millis) override
	{
		led_.toggle();
	}
	
private:
	gpio::FAST_PIN<board::DigitalPin::LED> led_;
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
