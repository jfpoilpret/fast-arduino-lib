/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/INT.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH = Board::ExternalInterruptPin::D2;
// Define vectors we need in the example
USE_EMPTY_INT0()
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH = Board::ExternalInterruptPin::D21;
// Define vectors we need in the example
USE_EMPTY_INT0()
#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin SWITCH = Board::ExternalInterruptPin::D10;
// Define vectors we need in the example
USE_EMPTY_INT0()
#else
#error "Current target is not yet supported!"
#endif

int main()
{
	// Enable interrupts at startup time
	sei();
	
	IOPin button{SWITCH, PinMode::INPUT_PULLUP};
	IOPin led{Board::DigitalPin::LED, PinMode::OUTPUT};	
	INTSignal<SWITCH> int0{InterruptTrigger::ANY_CHANGE};
	int0.enable();

	// Event Loop
	while (true)
	{
		if (button.value())
			led.clear();
		else
			led.set();
#if defined(BREADBOARD_ATTINYX4)
		// Not sure why, but INT0 ANY_CHANGE does not seem to wake up MCU in POWER_SAVE mode, 
		// although that works well with UNO and MEGA...
		Power::sleep(Board::SleepMode::IDLE);
#else
		Power::sleep(Board::SleepMode::POWER_DOWN);
#endif
	}
}
