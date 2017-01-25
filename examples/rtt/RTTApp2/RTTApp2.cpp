/*
 * Real Time Timer example. Take #2
 * This program shows usage of FastArduino Timer-based RTT (Real Time Timer) support.
 * It checks RTT with the timer that can awake from sleep mode, for the target board.
 * The program blinks a LED at a period of 10 seconds, forever.
 * 
 * This examples differs from RTTApp1 in that it sleeps to POWER_SAVE mode, thus reducing energy consumption.
 * 
 * Wiring:
 * - on Arduino UNO and Arduino MEGA:
 *   - no wiring needed as the program uses default LED on D13
 * - on ATmega328P based boards:
 *   - D13 (PB5) connected to a LED through a 330Ohm resistor then linked to GND
 * - on ATtinyX4 based boards:
 *   - D7 (LED, PA7) connected to a LED through a 330Ohm resistor then linked to GND
 */

#include <avr/interrupt.h>

#include <fastarduino/power.hh>
#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>

// Define vectors we need in the example
REGISTER_RTT_ISR(0)

const constexpr uint32_t BLINK_DELAY = 10000;

int main()
{
	// Enable interrupts at startup time
	sei();

	// NB in this sleep mode, delay takes about 1.5 times the specified time, and works only with Timer2
	// because other timers cannot wake up MCU from that sleep mode, as per specification.
	// The additional 0.5x are due to the wake-up time at every interrupt (every ms)
//	Power::set_default_mode(Board::SleepMode::POWER_SAVE);
	
	typename FastPinType<Board::DigitalPin::LED>::TYPE led{PinMode::OUTPUT, false};
	RTT<Board::Timer::TIMER0> rtt;
	rtt.register_rtt_handler();

	rtt.begin();
	// Event Loop
	while (true)
	{
		led.toggle();
		rtt.delay(BLINK_DELAY);
	}
}
