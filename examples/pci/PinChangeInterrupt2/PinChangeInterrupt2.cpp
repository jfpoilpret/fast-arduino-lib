//   Copyright 2016-2023 Jean-Francois Poilpret
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
 * This program shows usage of Pin Change Interrupt (PCI) FastArduino support to
 * light LEDs when buttons are pushed.
 * This sample uses a handler called by PCINT vector.
 * Concretely, the example takes PCI input from 3 switches and lights one of 3 
 * LEDs (1 LED per button).
 * It also toggles a 4th LED on each PCI interrupt.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D1, D3, D5, D7 (port D) branch 4 LED (in series with 330 Ohm resistors to 
 *     limit current) connected to ground
 *   - D14, D16, D17 (port C, ADC0, ADC2, ADC3) branch 3 buttons connected to ground
 * - on Arduino LEONARDO:
 *   - D0-D3 (port D) branch 4 LED (in series with 330 Ohm resistors to limit 
 *     current) connected to ground
 *   - D8-D10 (port B) branch 3 buttons connected to ground
 * - on Arduino MEGA:
 *   - D22-D25 (port A) branch 4 LED (in series with 330 Ohm resistors to limit 
 *     current) connected to ground
 *   - D53-D51 (port B) branch 3 buttons connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D3 (port A) branch 4 LED (in series with 330 Ohm resistors to limit 
 *     current) connected to ground
 *   - D8-D10 (port B) branch 3 buttons connected to ground
 * - on ATmega644 based boards:
 *   - D16-D19 (port C) branch 4 LED (in series with 330 Ohm resistors to limit 
 *     current) connected to ground
 *   - D0-D2 (port A) branch 3 buttons connected to ground
 */

#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
constexpr const board::InterruptPin SWITCH1 = board::InterruptPin::D14_PC0_PCI1;
constexpr const board::InterruptPin SWITCH2 = board::InterruptPin::D16_PC2_PCI1;
constexpr const board::InterruptPin SWITCH3 = board::InterruptPin::D17_PC3_PCI1;
constexpr const board::DigitalPin LED1 = board::DigitalPin::D1_PD1;
constexpr const board::DigitalPin LED2 = board::DigitalPin::D3_PD3;
constexpr const board::DigitalPin LED3 = board::DigitalPin::D5_PD5;
constexpr const board::DigitalPin LED4 = board::DigitalPin::D7_PD7;
#define PCI_NUM 1
#elif defined (ARDUINO_LEONARDO)
constexpr const board::InterruptPin SWITCH1 = board::InterruptPin::D8_PB4_PCI0;
constexpr const board::InterruptPin SWITCH2 = board::InterruptPin::D9_PB5_PCI0;
constexpr const board::InterruptPin SWITCH3 = board::InterruptPin::D10_PB6_PCI0;
constexpr const board::DigitalPin LED1 = board::DigitalPin::D0_PD2;
constexpr const board::DigitalPin LED2 = board::DigitalPin::D1_PD3;
constexpr const board::DigitalPin LED3 = board::DigitalPin::D2_PD1;
constexpr const board::DigitalPin LED4 = board::DigitalPin::D3_PD0;
#define PCI_NUM 0
#elif defined (ARDUINO_MEGA)
constexpr const board::InterruptPin SWITCH1 = board::InterruptPin::D53_PB0_PCI0;
constexpr const board::InterruptPin SWITCH2 = board::InterruptPin::D52_PB1_PCI0;
constexpr const board::InterruptPin SWITCH3 = board::InterruptPin::D51_PB2_PCI0;
constexpr const board::DigitalPin LED1 = board::DigitalPin::D22_PA0;
constexpr const board::DigitalPin LED2 = board::DigitalPin::D23_PA1;
constexpr const board::DigitalPin LED3 = board::DigitalPin::D24_PA2;
constexpr const board::DigitalPin LED4 = board::DigitalPin::D25_PA3;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATTINYX4)
constexpr const board::InterruptPin SWITCH1 = board::InterruptPin::D8_PB0_PCI1;
constexpr const board::InterruptPin SWITCH2 = board::InterruptPin::D9_PB1_PCI1;
constexpr const board::InterruptPin SWITCH3 = board::InterruptPin::D10_PB2_PCI1;
constexpr const board::DigitalPin LED1 = board::DigitalPin::D0_PA0;
constexpr const board::DigitalPin LED2 = board::DigitalPin::D1_PA1;
constexpr const board::DigitalPin LED3 = board::DigitalPin::D2_PA2;
constexpr const board::DigitalPin LED4 = board::DigitalPin::D3_PA3;
#define PCI_NUM 1
#elif defined (BREADBOARD_ATMEGAXX4P)
constexpr const board::InterruptPin SWITCH1 = board::InterruptPin::D0_PA0_PCI0;
constexpr const board::InterruptPin SWITCH2 = board::InterruptPin::D1_PA1_PCI0;
constexpr const board::InterruptPin SWITCH3 = board::InterruptPin::D2_PA2_PCI0;
constexpr const board::DigitalPin LED1 = board::DigitalPin::D16_PC0;
constexpr const board::DigitalPin LED2 = board::DigitalPin::D17_PC1;
constexpr const board::DigitalPin LED3 = board::DigitalPin::D18_PC2;
constexpr const board::DigitalPin LED4 = board::DigitalPin::D19_PC3;
#define PCI_NUM 0
#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler
{
public:
	PinChangeHandler()
	:	_switch1{gpio::PinMode::INPUT_PULLUP},
		_switch2{gpio::PinMode::INPUT_PULLUP},
		_switch3{gpio::PinMode::INPUT_PULLUP},
		_led1{gpio::PinMode::OUTPUT},
		_led2{gpio::PinMode::OUTPUT},
		_led3{gpio::PinMode::OUTPUT},
		_led4{gpio::PinMode::OUTPUT}
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
	gpio::FAST_INT_PIN<SWITCH1> _switch1;
	gpio::FAST_INT_PIN<SWITCH2> _switch2;
	gpio::FAST_INT_PIN<SWITCH3> _switch3;
	gpio::FAST_PIN<LED1> _led1;
	gpio::FAST_PIN<LED2> _led2;
	gpio::FAST_PIN<LED3> _led3;
	gpio::FAST_PIN<LED4> _led4;
};

// Define vectors we need in the example
REGISTER_PCI_ISR_METHOD(PCI_NUM, PinChangeHandler, &PinChangeHandler::on_pin_change, SWITCH1, SWITCH3, SWITCH3)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::PCI_SIGNAL<SWITCH1> pci;
	
	pci.enable_pin<SWITCH1>();
	pci.enable_pin<SWITCH2>();
	pci.enable_pin<SWITCH3>();
	pci.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
