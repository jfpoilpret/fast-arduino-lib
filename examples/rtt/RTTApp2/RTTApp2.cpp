/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/power.hh>
#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
// Define vectors we need in the example
//USE_RTT_TIMER0()
//USE_RTT_TIMER1()
USE_RTT_TIMER2()
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

	// NB in this sleep mode, delay takes about 1.5 times the specified time, and works only with Timer2
	// because other timers cannot wake uf MCU from that sleep mode, as per specification.
	// The additional 0.5x are due to the wake-up time at every interrupt (every ms)
	Power::set_default_mode(Board::SleepMode::POWER_SAVE);
	
	FastPin<Board::DigitalPin::LED> led{PinMode::OUTPUT, false};
//	RTT<Board::Timer::TIMER0> rtt;
//	RTT<Board::Timer::TIMER1> rtt;
	RTT<Board::Timer::TIMER2> rtt;

	rtt.begin();
	// Event Loop
	while (true)
	{
		led.toggle();
		rtt.delay(BLINK_DELAY);
	}
}
