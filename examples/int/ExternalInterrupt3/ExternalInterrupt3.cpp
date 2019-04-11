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

#include <fastarduino/gpio.h>
#include <fastarduino/int.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
constexpr const board::DigitalPin SWITCH_ON = board::ExternalInterruptPin::D2_PD2_EXT0;
constexpr const board::DigitalPin SWITCH_OFF = board::ExternalInterruptPin::D3_PD3_EXT1;
#elif defined (ARDUINO_LEONARDO)
constexpr const board::DigitalPin SWITCH_ON = board::ExternalInterruptPin::D3_PD0_EXT0;
constexpr const board::DigitalPin SWITCH_OFF = board::ExternalInterruptPin::D2_PD1_EXT1;
#elif defined (ARDUINO_MEGA)
constexpr const board::DigitalPin SWITCH_ON = board::ExternalInterruptPin::D21_PD0_EXT0;
constexpr const board::DigitalPin SWITCH_OFF = board::ExternalInterruptPin::D20_PD1_EXT1;
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
REGISTER_INT_ISR_METHOD(0, SWITCH_ON, SwitchHandler, &SwitchHandler::on_switch_on_change)
REGISTER_INT_ISR_METHOD(1, SWITCH_OFF, SwitchHandler, &SwitchHandler::on_switch_off_change)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	SwitchHandler switch_handler;
	interrupt::register_handler(switch_handler);
	interrupt::INTSignal<SWITCH_ON> int0{interrupt::InterruptTrigger::ANY_CHANGE};
	interrupt::INTSignal<SWITCH_OFF> int1{interrupt::InterruptTrigger::ANY_CHANGE};
	int0.enable();
	int1.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
