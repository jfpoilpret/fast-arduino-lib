//   Copyright 2016-2017 Jean-Francois Poilpret
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
 * Simple LED chaser. Take #1
 * This program shows usage of FastArduino events support and port API.
 * The number of LED roundtrips is limited to one because all events are pushed at startup and not regenerated.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO) and on Leonardo:
 *   - D0-D7 (port D) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on Arduino MEGA:
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 */

#include <fastarduino/gpio.h>
#include <fastarduino/events.h>
#include <fastarduino/time.h>

using namespace events;

static const uint8_t EVENT_QUEUE_SIZE = 32;
static const uint8_t NUM_LEDS = 8;

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
static constexpr const board::DigitalPin LED0 = board::DigitalPin::D0_PD0;
static constexpr const board::DigitalPin LED1 = board::DigitalPin::D1_PD1;
static constexpr const board::DigitalPin LED2 = board::DigitalPin::D2_PD2;
static constexpr const board::DigitalPin LED3 = board::DigitalPin::D3_PD3;
static constexpr const board::DigitalPin LED4 = board::DigitalPin::D4_PD4;
static constexpr const board::DigitalPin LED5 = board::DigitalPin::D5_PD5;
static constexpr const board::DigitalPin LED6 = board::DigitalPin::D6_PD6;
static constexpr const board::DigitalPin LED7 = board::DigitalPin::D7_PD7;
#elif defined (ARDUINO_LEONARDO)
static constexpr const board::DigitalPin LED0 = board::DigitalPin::D0_PD2;
static constexpr const board::DigitalPin LED1 = board::DigitalPin::D1_PD3;
static constexpr const board::DigitalPin LED2 = board::DigitalPin::D2_PD1;
static constexpr const board::DigitalPin LED3 = board::DigitalPin::D3_PD0;
static constexpr const board::DigitalPin LED4 = board::DigitalPin::D4_PD4;
static constexpr const board::DigitalPin LED5 = board::DigitalPin::D5_PC6;
static constexpr const board::DigitalPin LED6 = board::DigitalPin::D6_PD7;
static constexpr const board::DigitalPin LED7 = board::DigitalPin::D7_PE6;
#elif defined (ARDUINO_MEGA)
static constexpr const board::DigitalPin LED0 = board::DigitalPin::D22_PA0;
static constexpr const board::DigitalPin LED1 = board::DigitalPin::D23_PA1;
static constexpr const board::DigitalPin LED2 = board::DigitalPin::D24_PA2;
static constexpr const board::DigitalPin LED3 = board::DigitalPin::D25_PA3;
static constexpr const board::DigitalPin LED4 = board::DigitalPin::D26_PA4;
static constexpr const board::DigitalPin LED5 = board::DigitalPin::D27_PA5;
static constexpr const board::DigitalPin LED6 = board::DigitalPin::D28_PA6;
static constexpr const board::DigitalPin LED7 = board::DigitalPin::D29_PA7;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const board::DigitalPin LED0 = board::DigitalPin::D0_PA0;
static constexpr const board::DigitalPin LED1 = board::DigitalPin::D1_PA1;
static constexpr const board::DigitalPin LED2 = board::DigitalPin::D2_PA2;
static constexpr const board::DigitalPin LED3 = board::DigitalPin::D3_PA3;
static constexpr const board::DigitalPin LED4 = board::DigitalPin::D4_PA4;
static constexpr const board::DigitalPin LED5 = board::DigitalPin::D5_PA5;
static constexpr const board::DigitalPin LED6 = board::DigitalPin::D6_PA6;
static constexpr const board::DigitalPin LED7 = board::DigitalPin::D7_PA7;
#else
#error "Current target is not yet supported!"
#endif

// Avoiding multiple inheritance reduces code size:
// - no cxa pure virtual method included in linked exe (2 bytes)
// - one less vtable (for EventHandler itself)
// But this also increases main() stack size of 8 bytes, i.e. 1 byte per LedHandler instance?
template<board::DigitalPin PIN>
class LedHandler: public EventHandler
{
public:
	LedHandler() {}
	LedHandler(uint8_t type) : EventHandler{type}, _led{gpio::PinMode::OUTPUT} {}
	virtual void on_event(UNUSED const Event& event) override
	{
		_led.toggle();
	}
	
private:
	typename gpio::FastPinType<PIN>::TYPE _led;
};

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Prepare event queue
	Event buffer[EVENT_QUEUE_SIZE];
	containers::Queue<Event> event_queue{buffer};
	
	// Prepare Dispatcher and Handlers
	Dispatcher dispatcher;
	LedHandler<LED0> handler0{Type::USER_EVENT};
	LedHandler<LED1> handler1{uint8_t(Type::USER_EVENT + 1)};
	LedHandler<LED2> handler2{uint8_t(Type::USER_EVENT + 2)};
	LedHandler<LED3> handler3{uint8_t(Type::USER_EVENT + 3)};
	LedHandler<LED4> handler4{uint8_t(Type::USER_EVENT + 4)};
	LedHandler<LED5> handler5{uint8_t(Type::USER_EVENT + 5)};
	LedHandler<LED6> handler6{uint8_t(Type::USER_EVENT + 6)};
	LedHandler<LED7> handler7{uint8_t(Type::USER_EVENT + 7)};
	
	dispatcher.insert(handler0);
	dispatcher.insert(handler1);
	dispatcher.insert(handler2);
	dispatcher.insert(handler3);
	dispatcher.insert(handler4);
	dispatcher.insert(handler5);
	dispatcher.insert(handler6);
	dispatcher.insert(handler7);
	
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
		time::delay_ms(250);
	}
	return 0;
}
