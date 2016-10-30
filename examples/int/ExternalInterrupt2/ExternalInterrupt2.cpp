/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/INT.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D2;
constexpr const Board::ExternalInterruptPin INT_SWITCH = Board::ExternalInterruptPin::EXT0;
// Define vectors we need in the example
USE_EMPTY_INT0()

#elif defined (ARDUINO_MEGA)

#elif defined (BREADBOARD_ATTINYX4)

#else
#error "Current target is not yet supported!"
#endif

int main()
{
	// Enable interrupts at startup time
	sei();
	
	IOPin button{SWITCH, PinMode::INPUT_PULLUP};
	IOPin led{Board::DigitalPin::LED, PinMode::OUTPUT};	
	INTSignal<INT_SWITCH> int0{InterruptTrigger::ANY_CHANGE};
	int0.enable();

	// Event Loop
	while (true)
	{
		if (button.value())
			led.clear();
		else
			led.set();
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
