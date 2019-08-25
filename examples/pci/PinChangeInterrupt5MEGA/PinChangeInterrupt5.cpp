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
 * Pin Change Interrupt example verifying bug #40 fix on MEGA board.
 * If we use MEGA PJ0 or PJ1 pins as PCINT pins (vector PCINT1) then this shall
 * not work because the same vector is also shared by PE0 PCINT pin,
 * which introduces a shift in masks used in traits.
 * 
 * If hte bug is properly fixed, then we should see the proper LED lit when pushing
 * the matching button (3 buttons, 3 LEDs).
 * 
 * Wiring:
 * - on Arduino MEGA:
 *   - D0 (PCINT1, PE0) branch a push button connected to ground
 *   - D14 (PCINT1, PJ1) branch a push button connected to ground
 *   - D15 (PCINT1, PJ0) branch a push button connected to ground
 *   - D8-D10: LEDs in series with 330 Ohm resistor to GND
 */

#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

#if defined (ARDUINO_MEGA)
constexpr const board::InterruptPin SWITCH0 = board::InterruptPin::D0_PE0_PCI1;
constexpr const board::InterruptPin SWITCH1 = board::InterruptPin::D15_PJ0_PCI1;
constexpr const board::InterruptPin SWITCH2 = board::InterruptPin::D14_PJ1_PCI1;

constexpr const board::DigitalPin LED0 = board::DigitalPin::D10_PB4;
constexpr const board::DigitalPin LED1 = board::DigitalPin::D11_PB5;
constexpr const board::DigitalPin LED2 = board::DigitalPin::D12_PB6;

#define PCI_NUM 1
#else
#error "Current target is not supported!"
#endif

class PinChangeHandler
{
public:
	PinChangeHandler()
	:	_switch0{gpio::PinMode::INPUT_PULLUP},
		_switch1{gpio::PinMode::INPUT_PULLUP},
		_switch2{gpio::PinMode::INPUT_PULLUP},
		_led0{gpio::PinMode::OUTPUT},
		_led1{gpio::PinMode::OUTPUT},
		_led2{gpio::PinMode::OUTPUT}
	{}
	
	void on_pin_change()
	{
		if (_switch0.value()) _led0.clear(); else _led0.set();
		if (_switch1.value()) _led1.clear(); else _led1.set();
		if (_switch2.value()) _led2.clear(); else _led2.set();
	}
	
private:
	gpio::FastPinType<board::PCI_PIN<SWITCH0>()>::TYPE _switch0;
	gpio::FastPinType<board::PCI_PIN<SWITCH1>()>::TYPE _switch1;
	gpio::FastPinType<board::PCI_PIN<SWITCH2>()>::TYPE _switch2;
	gpio::FastPinType<LED0>::TYPE _led0;
	gpio::FastPinType<LED1>::TYPE _led1;
	gpio::FastPinType<LED2>::TYPE _led2;
};

// Define vectors we need in the example
REGISTER_PCI_ISR_METHOD(PCI_NUM, PinChangeHandler, &PinChangeHandler::on_pin_change, SWITCH0, SWITCH1, SWITCH2)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::PCIType<SWITCH0>::TYPE pci;
	
	pci.enable_pin<SWITCH0>();
	pci.enable_pin<SWITCH1>();
	pci.enable_pin<SWITCH2>();
	pci.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
