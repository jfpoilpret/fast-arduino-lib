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
 * Pin Change Interrupt example. Multiple PCI.
 * This program shows usage of Pin Change Interrupt (PCI) FastArduino support to light LEDs when buttons are pushed.
 * This sample uses a handler called by PCINT vector.
 * Concretely, the example takes PCI input from 3 switches and lights one of 3 LEDs (1 LED per button).
 * It also toggles a 4th LED on each PCI interrupt.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D1, D3, D5, D7 (port D) branch 4 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D14, D16, D17 (port C, ADC0, ADC2, ADC3) branch 3 buttons connected to ground
 * - on Arduino MEGA:
 *   - D22-D25 (port A) branch 4 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D53-D51 (port B) branch 3 buttons connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D3 (port A) branch 4 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D8-D10 (port B) branch 3 buttons connected to ground
 */

#include <avr/interrupt.h>

#include <fastarduino/fast_io.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH1 = Board::InterruptPin::D14_PCI1;
constexpr const Board::DigitalPin SWITCH2 = Board::InterruptPin::D16_PCI1;
constexpr const Board::DigitalPin SWITCH3 = Board::InterruptPin::D17_PCI1;
constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D1;
constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D3;
constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D5;
constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D7;
#define PCI_NUM 1
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH1 = Board::InterruptPin::D53_PCI0;
constexpr const Board::DigitalPin SWITCH2 = Board::InterruptPin::D52_PCI0;
constexpr const Board::DigitalPin SWITCH3 = Board::InterruptPin::D51_PCI0;
constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D22;
constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D23;
constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D24;
constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D25;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin SWITCH1 = Board::InterruptPin::D8_PCI1;
constexpr const Board::DigitalPin SWITCH2 = Board::InterruptPin::D9_PCI1;
constexpr const Board::DigitalPin SWITCH3 = Board::InterruptPin::D10_PCI1;
constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D0;
constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D1;
constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D2;
constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D3;
#define PCI_NUM 1
#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler
{
public:
	PinChangeHandler()
	:	_switch1{PinMode::INPUT_PULLUP},
		_switch2{PinMode::INPUT_PULLUP},
		_switch3{PinMode::INPUT_PULLUP},
		_led1{PinMode::OUTPUT},
		_led2{PinMode::OUTPUT},
		_led3{PinMode::OUTPUT},
		_led4{PinMode::OUTPUT}
	{
	}
	
	void on_pin_change()
	{
		if (_switch1.value()) _led1.clear(); else _led1.set();
		if (_switch2.value()) _led2.clear(); else _led2.set();
		if (_switch3.value()) _led3.clear(); else _led3.set();
		_led4.toggle();
	}
	
private:
	FastPinType<SWITCH1>::TYPE _switch1;
	FastPinType<SWITCH2>::TYPE _switch2;
	FastPinType<SWITCH3>::TYPE _switch3;
	FastPinType<LED1>::TYPE _led1;
	FastPinType<LED2>::TYPE _led2;
	FastPinType<LED3>::TYPE _led3;
	FastPinType<LED4>::TYPE _led4;
};

// Define vectors we need in the example
REGISTER_PCI_ISR_METHOD(PCI_NUM, PinChangeHandler, &PinChangeHandler::on_pin_change)

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();

	PinChangeHandler handler;
	register_handler(handler);
	PCIType<SWITCH1>::TYPE pci;
	
	pci.enable_pin<SWITCH1>();
	pci.enable_pin<SWITCH2>();
	pci.enable_pin<SWITCH3>();
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
