/*
 * This program is the Hello World of Arduino: blink LED on D13.
 * It checks FastArduino FastPin support.
 * So far this is just an active loop with a active wait delay.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D7 (PA7) LED connected to ground through a resistor
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <fastarduino/FastIO.hh>

int main()
{
	sei();
	FastPinType<Board::DigitalPin::LED>::TYPE PinLED{PinMode::OUTPUT};
	while (true)
	{
		PinLED.toggle();
		_delay_ms(400.0);
	}
	return 0;
}
