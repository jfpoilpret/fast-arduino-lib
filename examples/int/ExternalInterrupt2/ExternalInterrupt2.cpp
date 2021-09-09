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
 * Pin External Interrupt example. Take #2
 * This program shows usage of External Interrupt Pin FastArduino support to 
 * light a LED when a button is pushed.
 * This sample uses INT0 vector as a mere signal (no handler called) to awaken MCU 
 * from sleep mode, hence the button state is checked at wakeup time. This approach 
 * allows code size reduction by more than 50 bytes.
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
 * - on ATtinyX5 based boards:
 *   - D2 (INT0, PB2) branch a push button connected to ground
 *   - D0 (PB0) LED connected to ground through a resistor
 * - on ATmega644 based boards:
 *   - D26 (INT0, PD2) branch a push button connected to ground
 *   - D8 (PB0) LED connected to ground through a resistor
 */

#include <fastarduino/gpio.h>
#include <fastarduino/int.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
constexpr const board::ExternalInterruptPin SWITCH = board::ExternalInterruptPin::D2_PD2_EXT0;
// Define vectors we need in the example
REGISTER_INT_ISR_EMPTY(0, SWITCH)
#elif defined (ARDUINO_LEONARDO)
constexpr const board::ExternalInterruptPin SWITCH = board::ExternalInterruptPin::D3_PD0_EXT0;
REGISTER_INT_ISR_EMPTY(0, SWITCH)
#elif defined (ARDUINO_MEGA)
constexpr const board::ExternalInterruptPin SWITCH = board::ExternalInterruptPin::D21_PD0_EXT0;
// Define vectors we need in the example
REGISTER_INT_ISR_EMPTY(0, SWITCH)
#elif defined (BREADBOARD_ATTINYX4)
constexpr const board::ExternalInterruptPin SWITCH = board::ExternalInterruptPin::D10_PB2_EXT0;
// Define vectors we need in the example
REGISTER_INT_ISR_EMPTY(0, SWITCH)
#elif defined (BREADBOARD_ATTINYX5)
constexpr const board::ExternalInterruptPin SWITCH = board::ExternalInterruptPin::D2_PB2_EXT0;
// Define vectors we need in the example
REGISTER_INT_ISR_EMPTY(0, SWITCH)
#elif defined (BREADBOARD_ATMEGA644P)
constexpr const board::ExternalInterruptPin SWITCH = board::ExternalInterruptPin::D26_PD2_EXT0;
// Define vectors we need in the example
REGISTER_INT_ISR_EMPTY(0, SWITCH)
#else
#error "Current target is not yet supported!"
#endif

int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	gpio::FAST_EXT_PIN<SWITCH> button{gpio::PinMode::INPUT_PULLUP};
	gpio::FAST_PIN<board::DigitalPin::LED> led{gpio::PinMode::OUTPUT};	
	interrupt::INTSignal<SWITCH> int0{interrupt::InterruptTrigger::ANY_CHANGE};
	int0.enable();

	// Event Loop
	while (true)
	{
		if (button.value())
			led.clear();
		else
			led.set();
#if defined(BREADBOARD_ATTINYX4) || defined(BREADBOARD_ATTINYX5)
		// Not sure why, but INT0 ANY_CHANGE does not seem to wake up MCU in POWER_SAVE mode, 
		// although that works well with UNO and MEGA...
		power::Power::sleep(board::SleepMode::IDLE);
#else
		power::Power::sleep(board::SleepMode::POWER_DOWN);
#endif
	}
}
