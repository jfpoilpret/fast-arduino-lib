/*
 * Simple LED chaser. Take #1
 * This program shows usage of FastArduino events support and port API.
 * The number of LED roundtrips is limited to one because all events are pushed at startup and not regenerated.
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

class LedHandler: public EventHandler, private IOPin
{
public:
	LedHandler() {}
	LedHandler(uint8_t type, Board::DigitalPin led) : EventHandler{type}, IOPin{led, PinMode::OUTPUT} {}
	virtual void on_event(UNUSED const Event& event) override
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
	LedHandler handlers[NUM_LEDS]
	{
		LedHandler{Type::USER_EVENT, LED0},
		LedHandler{uint8_t(Type::USER_EVENT + 1), LED1},
		LedHandler{uint8_t(Type::USER_EVENT + 2), LED2},
		LedHandler{uint8_t(Type::USER_EVENT + 3), LED3},
		LedHandler{uint8_t(Type::USER_EVENT + 4), LED4},
		LedHandler{uint8_t(Type::USER_EVENT + 5), LED5},
		LedHandler{uint8_t(Type::USER_EVENT + 6), LED6},
		LedHandler{uint8_t(Type::USER_EVENT + 7), LED7}
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
