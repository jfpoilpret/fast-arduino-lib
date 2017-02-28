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
 * Pin Change Interrupt example. Take #1
 * This program shows usage of Pin Change Interrupt (PCI) FastArduino support to light a LED when a button is pushed.
 * This sample uses a handler called by 2 PCINT vectors.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D14 (PC0, ADC0) branch a push button connected to ground
 *   - D8 (PB0) branch a push button connected to ground
 *   - D13 (PB5) LED connected to ground through a resistor
 */

#include <avr/interrupt.h>

#include <fastarduino/fast_io.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const board::DigitalPin SWITCH_ON = board::InterruptPin::D14_PC0_PCI1;
constexpr const board::DigitalPin SWITCH_OFF = board::InterruptPin::D8_PB0_PCI0;
#define PCINT_ON	1
#define PCINT_OFF	0
#elif defined(ARDUINO_MEGA)
constexpr const board::DigitalPin SWITCH_ON = board::InterruptPin::D53_PB0_PCI0;
constexpr const board::DigitalPin SWITCH_OFF = board::InterruptPin::D62_PK0_PCI2;
#define PCINT_ON	0
#define PCINT_OFF	2
#elif defined(BREADBOARD_ATTINYX4)
#define PCINT_ON	0
#define PCINT_OFF	1
constexpr const board::DigitalPin SWITCH_ON = board::InterruptPin::D0_PA0_PCI0;
constexpr const board::DigitalPin SWITCH_OFF = board::InterruptPin::D8_PB0_PCI1;
#define PCINT_ON	0
#define PCINT_OFF	1
#else
#error "Current target is not yet supported!"
#endif

class SwitchHandler
{
public:
	SwitchHandler()
	:	_switch_on{gpio::PinMode::INPUT_PULLUP},
		_switch_off{gpio::PinMode::INPUT_PULLUP},
		_led{gpio::PinMode::OUTPUT}
	{}
	
	void on_switch_on_change()
	{
		if (!_switch_on.value())
			_led.set();
	}
	
	void on_switch_off_change()
	{
		if (!_switch_off.value())
			_led.clear();
	}
	
private:
	gpio::FastPinType<SWITCH_ON>::TYPE _switch_on;
	gpio::FastPinType<SWITCH_OFF>::TYPE _switch_off;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;	
};

// Define vectors we need in the example
REGISTER_PCI_ISR_METHOD(PCINT_ON, SwitchHandler, &SwitchHandler::on_switch_on_change, SWITCH_ON)
REGISTER_PCI_ISR_METHOD(PCINT_OFF, SwitchHandler, &SwitchHandler::on_switch_off_change, SWITCH_OFF)

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	SwitchHandler switch_handler;
	register_handler(switch_handler);
	interrupt::PCIType<SWITCH_ON>::TYPE pci_on;
	interrupt::PCIType<SWITCH_OFF>::TYPE pci_off;
	
	pci_on.enable_pin<SWITCH_ON>();
	pci_off.enable_pin<SWITCH_OFF>();
	pci_on.enable();
	pci_off.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
