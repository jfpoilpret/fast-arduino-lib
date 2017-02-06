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
 * Pin External Interrupt example. Take #1
 * This program shows usage of External Interrupt Pin FastArduino support to light a LED when a button is pushed, and
 * switch it off when another button is pushed.
 * This sample uses a handler called by INT vector.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D2 (INT0, PD2) branch a push button connected to ground
 *   - D3 (INT1, PD3) branch a push button connected to ground
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA: TODO
 *   - D21 (INT0) branch a push button connected to ground
 *   - D20 (INT1) branch a push button connected to ground
 *   - D13 LED connected to ground through a resistor
 */

#include <avr/interrupt.h>

#include <fastarduino/fast_io.h>
#include <fastarduino/int.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH_ON = Board::ExternalInterruptPin::D2;
constexpr const Board::DigitalPin SWITCH_OFF = Board::ExternalInterruptPin::D3;
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH_ON = Board::ExternalInterruptPin::D21;
constexpr const Board::DigitalPin SWITCH_OFF = Board::ExternalInterruptPin::D20;
#else
#error "Current target is not yet supported!"
#endif

class SwitchHandler
{
public:
	SwitchHandler()
	:	_switch_on{PinMode::INPUT_PULLUP},
		_switch_off{PinMode::INPUT_PULLUP},
		_led{PinMode::OUTPUT}
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
	FastPinType<SWITCH_ON>::TYPE _switch_on;
	FastPinType<SWITCH_OFF>::TYPE _switch_off;
	FastPinType<Board::DigitalPin::LED>::TYPE _led;	
};

// Define vectors we need in the example
REGISTER_INT_ISR_METHOD(0, SwitchHandler, &SwitchHandler::on_switch_on_change)
REGISTER_INT_ISR_METHOD(1, SwitchHandler, &SwitchHandler::on_switch_off_change)

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	SwitchHandler switch_handler;
	register_handler(switch_handler);
	INTSignal<SWITCH_ON> int0{InterruptTrigger::ANY_CHANGE};
	INTSignal<SWITCH_OFF> int1{InterruptTrigger::ANY_CHANGE};
	int0.enable();
	int1.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
