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

//using ::Event::Type;
//using ::Event::Dispatcher;
//using ::Event::Handler;
//using ::Event::Event;

static const uint8_t EVENT_QUEUE_SIZE = 64;
static const uint8_t NUM_LEDS = 8;

class LedHandler: private IOPin
{
public:
	LedHandler() {}
	LedHandler(Board::DigitalPin led) : IOPin{led, PinMode::OUTPUT} {}
	void operator()(const Event::Event& event)
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
	Event::Event buffer[EVENT_QUEUE_SIZE];
	Queue<Event::Event> event_queue = Queue<Event::Event>::create<EVENT_QUEUE_SIZE>(buffer);
	
	// Prepare Dispatcher and Handlers
	Event::Dispatcher dispatcher;
	Event::FunctorHandler<LedHandler> handlers[NUM_LEDS];
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
	{
		handlers[i] = Event::FunctorHandler<LedHandler>{(Event::Type) (Event::USER_EVENT + i), LedHandler{Board::D0}};
		dispatcher.insert(handlers[i]);
	}
	
	// push some events for a start
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
		event_queue.push(Event::Event{(Event::Type) (Event::USER_EVENT + i)});

	// Event Loop
	//FIXME it seems no code is generated for the following code! Why?
	while (true)
	{
		dispatcher.dispatch(event_queue.pull());
		_delay_ms(200);
	}
	return 0;
}
