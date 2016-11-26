/*
 * Simple LED chaser. Take #4
 * This program shows usage of FastArduino periodic jobs support, triggered by Watchdog, and port API.
 * It is similar to EventApp3 except that it consumes less power, by spending most time in POWER DOWN mode.
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

#include <fastarduino/IO.hh>
#include <fastarduino/Events.hh>
#include <fastarduino/watchdog.hh>
#include <fastarduino/scheduler.hh>
#include <fastarduino/power.hh>

using namespace Events;

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const Board::Port LED_PORT = Board::Port::PORT_D;
#elif defined (ARDUINO_MEGA)
static constexpr const Board::Port LED_PORT = Board::Port::PORT_A;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::Port LED_PORT = Board::Port::PORT_A;
#else
#error "Current target is not yet supported!"
#endif

static const uint32_t PERIOD = 1000;

class LedHandler: public Job, private IOPort
{
public:
	LedHandler() : Job{0, PERIOD}, IOPort{LED_PORT, 0xFF}, _value{0} {}
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
static Event buffer[EVENT_QUEUE_SIZE];
static Queue<Event> event_queue{buffer};

int main()
{
	// Set power settings
	Power::set_default_mode(Board::SleepMode::POWER_DOWN);
	// Enable interrupts at startup time
	sei();

	// Prepare Dispatcher and Handlers
	Dispatcher dispatcher;
	Watchdog watchdog{event_queue};
	Scheduler<Watchdog> scheduler{watchdog, Type::WDT_TIMER};
	dispatcher.insert(scheduler);

	LedHandler job;
	scheduler.schedule(job);
	
	// Start watchdog
	watchdog.begin(Watchdog::TimeOut::TO_125ms);
	
	// Event Loop
	while (true)
	{
		Event event = pull(event_queue);
		dispatcher.dispatch(event);
	}
}
