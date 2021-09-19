//   Copyright 2016-2021 Jean-Francois Poilpret
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
 * Simple Event loop example.
 * This program shows usage of FastArduino events support and GPIO port API.
 * The example takes input from 8 buttons, each button triggers a specific sequence of
 * LED13 blinks. When a button is pushed, this triggers an event that is then read by
 * the event loop and acted upon.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D0-D7 (port D) branch 8 push buttons connected to ground
 */

#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/events.h>
#include <fastarduino/time.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
#define PCI_NUM 2
static constexpr const board::Port BUTTONS_PORT = board::Port::PORT_D;
static constexpr const board::DigitalPin LED = board::DigitalPin::LED;
#else
#error "Current target is not yet supported!"
#endif

using namespace events;

using EVENT = Event<uint8_t>;
static constexpr const uint8_t BUTTON_EVENT = Type::USER_EVENT;

// Class handling PCI interrupts and transforming them to events
class EventGenerator
{
public:
	EventGenerator(containers::Queue<EVENT>& event_queue)
	:event_queue_{event_queue}, buttons_{0x00, 0xFF}
	{
	}

	void on_pin_change()
	{
		event_queue_.push_(EVENT{BUTTON_EVENT, buttons_.get_PIN()});
	}

private:
	containers::Queue<EVENT>& event_queue_;
	gpio::FastPort<BUTTONS_PORT> buttons_;
};

REGISTER_PCI_ISR_METHOD(PCI_NUM, EventGenerator, &EventGenerator::on_pin_change, board::InterruptPin::D0_PD0_PCI2)

void blink(uint8_t buttons)
{
	// If no button is pressed, do nothing
	if (!buttons) return;

	gpio::FAST_PIN<LED> led;
	// Buttons are plit in 2 groups of four:
	// - 1st group sets 5 iterations
	// - 2nd group sets 10 iterations
	// Note: we multiply by 2 because one blink iteration means toggling the LED twice
	uint8_t iterations = (buttons & 0x0F ? 5 : 10) * 2;
	// In each group, each buttons define the delay between LED toggles
	// - 1st/5th button: 200ms
	// - 2nd/6th button: 400ms
	// - 3rd/7th button: 800ms
	// - 4th/8th button: 1600ms
	uint16_t delay = (buttons & 0x11 ? 200 : buttons & 0x22 ? 400 : buttons & 0x44 ? 800 : 1600);
	while (iterations--)
	{
		led.toggle();
		time::delay_ms(delay);
	}
}

static const uint8_t EVENT_QUEUE_SIZE = 32;

int main() __attribute__((OS_main));
int main()
{
	board::init();

	// Prepare event queue
	EVENT buffer[EVENT_QUEUE_SIZE];
	containers::Queue<EVENT> event_queue{buffer};

	// Create and register event generator
	EventGenerator generator{event_queue};
	interrupt::register_handler(generator);

	// Setup PCI interrupts
	interrupt::PCISignal<PCI_NUM> signal;
	signal.enable_pins_(0xFF);
	signal.enable_();

	// Setup LED pin as output
	gpio::FAST_PIN<LED> led{gpio::PinMode::OUTPUT};

	// Enable interrupts at startup time
	sei();

	// Event Loop
	while (true)
	{
		EVENT event = containers::pull(event_queue);
		if (event.type() == BUTTON_EVENT)
			// Invert levels as 0 means button pushed (and we want 1 instead)
			blink(event.value() ^ 0xFF);
	}
	return 0;
}
