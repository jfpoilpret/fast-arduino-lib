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
 * This program shows usage of External Interrupt Pin FastArduino support to light a LED when a button is pushed.
 * This sample uses a handler called by INT vector.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D2 (INT0, PD2) branch a push button connected to ground
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D21 (INT0) branch a push button connected to ground
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D10 (INT0, PB0) branch a push button connected to ground
 *   - D7 (PA7) LED connected to ground through a resistor
 */

#include <avr/interrupt.h>

#include <fastarduino/fast_io.h>
#include <fastarduino/int.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH = Board::ExternalInterruptPin::D2_EXT0;
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH = Board::ExternalInterruptPin::D21_EXT0;
#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin SWITCH = Board::ExternalInterruptPin::D10_EXT0;
#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler
{
public:
	PinChangeHandler()
	:	_switch{PinMode::INPUT_PULLUP},
		_led{PinMode::OUTPUT}
	{}
	
	void on_pin_change()
	{
		if (_switch.value())
			_led.clear();
		else
			_led.set();
	}
	
private:
	FastPinType<SWITCH>::TYPE _switch;
	FastPinType<Board::DigitalPin::LED>::TYPE _led;	
};

// Define vectors we need in the example
REGISTER_INT_ISR_METHOD(0, PinChangeHandler, &PinChangeHandler::on_pin_change)

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	register_handler(handler);
	INTSignal<SWITCH> int0{InterruptTrigger::ANY_CHANGE};
	int0.enable();

	// Event Loop
	while (true)
	{
#if defined(BREADBOARD_ATTINYX4)
		// Not sure why, but INT0 ANY_CHANGE does not seem to wake up MCU in POWER_SAVE mode, 
		// although that works well with UNO and MEGA...
		Power::sleep(Board::SleepMode::IDLE);
#else
		Power::sleep(Board::SleepMode::POWER_DOWN);
#endif
	}
}
