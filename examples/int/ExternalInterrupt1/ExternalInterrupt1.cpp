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
 * - on Arduino LEONARDO:
 *   - D3 (INT0) branch a push button connected to ground
 *   - D13 (PC7) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D21 (INT0) branch a push button connected to ground
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D10 (INT0, PB0) branch a push button connected to ground
 *   - D7 (PA7) LED connected to ground through a resistor
 */

#include <fastarduino/gpio.h>
#include <fastarduino/int.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const board::DigitalPin SWITCH = board::ExternalInterruptPin::D2_PD2_EXT0;
#elif defined (ARDUINO_LEONARDO)
constexpr const board::DigitalPin SWITCH = board::ExternalInterruptPin::D3_PD0_EXT0;
#elif defined (ARDUINO_MEGA)
constexpr const board::DigitalPin SWITCH = board::ExternalInterruptPin::D21_PD0_EXT0;
#elif defined (BREADBOARD_ATTINYX4)
constexpr const board::DigitalPin SWITCH = board::ExternalInterruptPin::D10_PB2_EXT0;
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
	gpio::FastPinType<SWITCH>::TYPE _switch;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;	
};

// Define vectors we need in the example
REGISTER_INT_ISR_METHOD(0, SWITCH, PinChangeHandler, &PinChangeHandler::on_pin_change)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::INTSignal<SWITCH> int0{interrupt::InterruptTrigger::ANY_CHANGE};
	int0.enable();

	// Event Loop
	while (true)
	{
#if defined(BREADBOARD_ATTINYX4)
		// Not sure why, but INT0 ANY_CHANGE does not seem to wake up MCU in POWER_SAVE mode, 
		// although that works well with UNO and MEGA...
		power::Power::sleep(board::SleepMode::IDLE);
#else
		power::Power::sleep(board::SleepMode::POWER_DOWN);
#endif
	}
}
