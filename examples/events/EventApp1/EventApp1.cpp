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

class LedHandler: public Event::Handler
{
public:
	LedHandler() : Event::Handler{}, _led{} {}
	LedHandler(Event::Type event, Board::DigitalPin led) : Event::Handler{event}, _led{led, PinMode::OUTPUT} {}
	virtual void on_event(const Event::Event& event)
	{
		UNUSED(event);
		_led.toggle();
	}

private:
	//TODO use private inheritance instead?
	IOPin _led;
};

int main()
{
	// Enable interrupts at startup time
	sei();
	// Prepare port to write to LEDs (debug)
//	IOPort PortD{Board::PORT_D, 0xFF};

	// Prepare event queue
	Event::Event buffer[EVENT_QUEUE_SIZE];
	Queue<Event::Event> event_queue = Queue<Event::Event>::create<EVENT_QUEUE_SIZE>(buffer);
	
	// Prepare Dispatcher and Handlers
	Event::Dispatcher dispatcher;
	LedHandler handlers[NUM_LEDS];
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
	{
		handlers[i] = LedHandler{(Event::Type) (Event::USER_EVENT + i), Board::D0};
		dispatcher.insert(handlers[i]);
	}
	
	// Event Loop
	while (true)
	{
		dispatcher.dispatch(event_queue.pull());
	}
	return 0;
}
