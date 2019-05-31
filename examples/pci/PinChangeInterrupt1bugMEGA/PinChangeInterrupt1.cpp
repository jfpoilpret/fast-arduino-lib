//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * Pin Change Interrupt example showing current bug on MEGA board.
 * If we use MEGA PJ0 or PJ1 pins as PCINT pins (vector PCINT1) then this shall
 * not work because the same vector is also shared by PE0 PCINT pin,
 * which introduces a shift in masks used in traits.
 * 
 * Wiring:
 * - on Arduino MEGA:
 *   - D15 (PCINT1, PJ0) branch a push button connected to ground
 *   - D13 (PB7) LED connected to ground through a resistor
 */

#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

#if defined (ARDUINO_MEGA)
// constexpr const board::InterruptPin SWITCH = board::InterruptPin::D15_PJ0_PCI1;
constexpr const board::InterruptPin SWITCH = board::InterruptPin::D14_PJ1_PCI1;
#define PCI_NUM 1
#else
#error "Current target is not supported!"
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
	gpio::FastPinType<board::PCI_PIN<SWITCH>()>::TYPE _switch;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;	
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
	interrupt::PCIType<SWITCH>::TYPE pci;
	
	// This code works
	// pci.enable_pins(0xFF);
	// That code does not work
	pci.enable_pin<SWITCH>();
	pci.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
