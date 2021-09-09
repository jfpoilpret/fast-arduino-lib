//   Copyright 2016-2020 Jean-Francois Poilpret
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
 * Pin Change Interrupt example. Take #1
 * This program shows usage of Pin Change Interrupt (PCI) FastArduino support to 
 * light a LED when a button is pushed.
 * This sample uses a handler called by PCINT vector.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D14 (PCINT8, PC0, ADC0) branch a push button connected to ground
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino LEONARDO:
 *   - D8 (PCINT4, PB4) branch a push button connected to ground
 *   - D13 (PC7) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D53 (PCINT0, PB0) branch a push button connected to ground
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D8 (PCINT8, PB0) branch a push button connected to ground
 *   - D7 (PA7) LED connected to ground through a resistor
 * - on ATtinyX5 based boards:
 *   - D1 (PCINT1, PB1) branch a push button connected to ground
 *   - D0 (PB0) LED connected to ground through a resistor
 * - on ATmega644 based boards:
 *   - D0 (PCINT0, PA0) branch a push button connected to ground
 *   - D8 (PB0) LED connected to ground through a resistor
 */

#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
constexpr const board::InterruptPin SWITCH = board::InterruptPin::D14_PC0_PCI1;
#define PCI_NUM 1
#elif defined (ARDUINO_LEONARDO)
constexpr const board::InterruptPin SWITCH = board::InterruptPin::D8_PB4_PCI0;
#define PCI_NUM 0
#elif defined (ARDUINO_MEGA)
constexpr const board::InterruptPin SWITCH = board::InterruptPin::D53_PB0_PCI0;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATTINYX4)
constexpr const board::InterruptPin SWITCH = board::InterruptPin::D8_PB0_PCI1;
#define PCI_NUM 1
#elif defined (BREADBOARD_ATTINYX5)
constexpr const board::InterruptPin SWITCH = board::InterruptPin::D1_PB1_PCI0;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATMEGA644P)
constexpr const board::InterruptPin SWITCH = board::InterruptPin::D0_PA0_PCI0;
#define PCI_NUM 0
#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler
{
public:
	PinChangeHandler()
	:	_switch{gpio::PinMode::INPUT_PULLUP},
		_led{gpio::PinMode::OUTPUT}
	{}
	
	void on_pin_change()
	{
		if (_switch.value())
			_led.clear();
		else
			_led.set();
	}
	
private:
	gpio::FAST_INT_PIN<SWITCH> _switch;
	gpio::FAST_PIN<board::DigitalPin::LED> _led;	
};

// Define vectors we need in the example
REGISTER_PCI_ISR_METHOD(PCI_NUM, PinChangeHandler, &PinChangeHandler::on_pin_change, SWITCH)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::PCI_SIGNAL<SWITCH> pci;
	
	pci.enable_pin<SWITCH>();
	pci.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
