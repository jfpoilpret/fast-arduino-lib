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

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const Board::DigitalPin LED0 = Board::DigitalPin::D0;
static constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D1;
static constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D2;
static constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D3;
static constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D4;
static constexpr const Board::DigitalPin LED5 = Board::DigitalPin::D5;
static constexpr const Board::DigitalPin LED6 = Board::DigitalPin::D6;
static constexpr const Board::DigitalPin LED7 = Board::DigitalPin::D7;
#elif defined (ARDUINO_MEGA)
static constexpr const Board::DigitalPin LED0 = Board::DigitalPin::D22;
static constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D23;
static constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D24;
static constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D25;
static constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D26;
static constexpr const Board::DigitalPin LED5 = Board::DigitalPin::D27;
static constexpr const Board::DigitalPin LED6 = Board::DigitalPin::D28;
static constexpr const Board::DigitalPin LED7 = Board::DigitalPin::D29;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::DigitalPin LED0 = Board::DigitalPin::D0;
static constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D1;
static constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D2;
static constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D3;
static constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D4;
static constexpr const Board::DigitalPin LED5 = Board::DigitalPin::D5;
static constexpr const Board::DigitalPin LED6 = Board::DigitalPin::D6;
static constexpr const Board::DigitalPin LED7 = Board::DigitalPin::D7;
#else
#error "Current target is not yet supported!"
#endif

class LedHandler: private IOPin
{
public:
	LedHandler() {}
	LedHandler(Board::DigitalPin led) : IOPin{led, PinMode::OUTPUT} {}
	void operator()(const Event& event UNUSED)
	{
		toggle();
	}
};

int main()
{
	// Enable interrupts at startup time
	sei();

	// Prepare event queue
	Event buffer[EVENT_QUEUE_SIZE];
	Queue<Event> event_queue{buffer};
	
	// Prepare Dispatcher and Handlers
	Dispatcher dispatcher;
	FunctorHandler<LedHandler> handlers[NUM_LEDS]
	{
		FunctorHandler<LedHandler>{Type::USER_EVENT, LedHandler{LED0}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 1), LedHandler{LED1}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 2), LedHandler{LED2}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 3), LedHandler{LED3}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 4), LedHandler{LED4}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 5), LedHandler{LED5}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 6), LedHandler{LED6}},
		FunctorHandler<LedHandler>{uint8_t(Type::USER_EVENT + 7), LedHandler{LED7}}
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
		Event event = pull(event_queue);
		dispatcher.dispatch(event);
		_delay_ms(250);
	}
	return 0;
}
