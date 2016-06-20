/*
 * LED chaser, with input settings
 * This program shows usage of FastArduino port API to handle several output at a time, plus individual input pins.
 * On Arduino, you should branch LED (in series with 330 Ohm resistors to limit current) on the following pins:
 * - D0-D7
 * i.e. all pins mapped to AVR ATmega328 PORT D
 * Then, you should branch 4 switches (I use DIP switches which are convenient on breadboard):
 * - one side to A0-A2 (number of simultaneously lit LED) and A3 (chase direction)
 * - the other side to GND (we use internal pullup resistors for inputs)
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/IO.hh>
#include <fastarduino/Events.hh>

using namespace ::Events;

static const uint8_t EVENT_QUEUE_SIZE = 64;
static const uint8_t NUM_LEDS = 8;

static IOPin debug{Board::LED, PinMode::OUTPUT, 0};

static void debug_delay(uint8_t times = 4)
{
	for (uint8_t i = 0; i < times; ++i)
		_delay_ms(250);
}

static void debug_blink(uint8_t times = 1)
{
	for (uint8_t i = 0; i < times; ++i)
	{
		debug.set();
		_delay_ms(250);
		_delay_ms(250);
		debug.clear();
		_delay_ms(250);
		_delay_ms(250);
	}
}

//class LedHandler: private IOPin
class LedHandler: public IOPin
{
public:
	LedHandler(): _bit{0} {}
	LedHandler(Board::DigitalPin led) : IOPin{led, PinMode::OUTPUT}, _bit{uint8_t(led)} 
	{
		debug_blink(1 + _bit);
		debug_delay(16);
	}
	void operator()(const Event& event)
	{
//		UNUSED(event);
//		debug_blink(1 + event.type() - Type::USER_EVENT);
		toggle();
	}
private:
	uint8_t _bit;
};

//class DebugHandler
//{
//public:
//	void operator()(AbstractHandler& handler)
//	{
//		if (handler.type() == Type::USER_EVENT + 1)
//		{
//			debug.toggle();
//			debug_delay();
//		}
//	}
//};

int main()
{
	// Enable interrupts at startup time
	sei();

	// Prepare event queue
	Event buffer[EVENT_QUEUE_SIZE];
	Queue<Event> event_queue = Queue<Event>::create<EVENT_QUEUE_SIZE>(buffer);
	
	// Prepare Dispatcher and Handlers
	Dispatcher dispatcher;
	FunctorHandler<LedHandler> handlers[NUM_LEDS];
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
	{
		LedHandler handler{(Board::DigitalPin)(Board::D0 + i)};
		handlers[i] = FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + i), handler};
		dispatcher.insert(handlers[i]);
	}
	
	// Debug number of handlers in dispatcher
//	dispatcher.traverse(DebugHandler());
	
	// push some events for a start
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
		event_queue.push(Event{uint8_t(Type::USER_EVENT + i)});
	// Debug number of items in queue
//	uint8_t size = event_queue.items();
//	while (size--)
//	{
//		debug.set();
//		debug_delay();
//		debug.clear();
//		debug_delay();
//	}
//	debug_delay(20);

	// Event Loop
	while (true)
	{
		Event event = event_queue.pull();
		// Debug event type
//		debug_blink(1 + event.type() - Type::USER_EVENT);
		dispatcher.dispatch(event);
//		debug.set();
//		debug_delay();
//		debug.clear();
//		debug_delay();
		debug_delay();
	}
	return 0;
}
