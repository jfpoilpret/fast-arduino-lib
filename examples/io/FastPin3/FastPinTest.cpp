/*
 * This program is the Hello World of Arduino: blink LED on D13.
 * It checks FastArduino FastPort support.
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

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const Board::Port LED_PORT = Board::Port::PORT_D;
#elif defined (ARDUINO_MEGA)
static constexpr const Board::Port LED_PORT = Board::Port::PORT_A;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::Port LED_PORT = Board::Port::PORT_A;
#else
#error "Current target is not yet supported!"
#endif

int main()
{
	// Enable interrupts at startup time
	sei();

	// Set Port D direction to all outputs
	FastPort<LED_PORT> ledPort;
	ledPort.set_DDR(0xFF);
	uint8_t value = 0;
	// Loop of the LED chaser
	while (true)
	{
		if (value == 0)
			value = 0x01;
		else
			value <<= 1;
		ledPort.set_PORT(value);
		
		_delay_ms(250.0);
	}
	return 0;
}
