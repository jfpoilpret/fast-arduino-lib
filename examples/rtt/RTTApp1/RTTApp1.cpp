/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
// Define vectors we need in the example
USE_RTT_TIMER0()
#elif defined (ARDUINO_MEGA)
// Define vectors we need in the example

#elif defined (BREADBOARD_ATTINYX4)
// Define vectors we need in the example

#else
#error "Current target is not yet supported!"
#endif

const constexpr uint32_t BLINK_DELAY = 10000;

int main()
{
	// Enable interrupts at startup time
	sei();

	FastPin<Board::DigitalPin::LED> led{PinMode::OUTPUT, false};
	RTT8<Board::Timer8::TIMER0> rtt;

	rtt.begin();
	// Event Loop
	while (true)
	{
		led.toggle();
		rtt.delay(BLINK_DELAY);
	}
}
