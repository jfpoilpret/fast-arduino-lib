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

using namespace Events;

static const uint8_t EVENT_QUEUE_SIZE = 32;
static const uint8_t NUM_LEDS = 8;

static IOPin debug{Board::LED, PinMode::OUTPUT, 0};

static void debug_delay(uint8_t times = 4)
{
	for (uint8_t i = 0; i < times; ++i)
		_delay_ms(250);
}

class LedHandler: private IOPin
{
public:
	LedHandler() {}
	LedHandler(Board::DigitalPin led) : IOPin{led, PinMode::OUTPUT} {}
	void operator()(const Event& event)
	{
		UNUSED(event);
		toggle();
	}
};

int main()
{
	// Enable interrupts at startup time
	sei();

	// Prepare event queue
	Event buffer[EVENT_QUEUE_SIZE];
	Queue<Event> event_queue = Queue<Event>::create<EVENT_QUEUE_SIZE>(buffer);
	
	// Prepare Dispatcher and Handlers
	Dispatcher dispatcher;
	FunctorHandler<LedHandler> handlers[NUM_LEDS]
	{
		FunctorHandler<LedHandler>{Type::USER_EVENT, LedHandler{Board::D0}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 1), LedHandler{Board::D1}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 2), LedHandler{Board::D2}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 3), LedHandler{Board::D3}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 4), LedHandler{Board::D4}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 5), LedHandler{Board::D5}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 6), LedHandler{Board::D6}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 7), LedHandler{Board::D7}}
	};
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
		dispatcher.insert(handlers[i]);
	
	// push some events for a start
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
	{
		event_queue.push(Event{uint8_t(Type::USER_EVENT + i)});
		event_queue.push(Event{uint8_t(Type::USER_EVENT + i)});
	}

	// Event Loop
	while (true)
	{
		Event event = event_queue.pull();
		dispatcher.dispatch(event);
		debug_delay(1);
	}
	return 0;
}
