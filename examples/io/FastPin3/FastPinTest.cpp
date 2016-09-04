/*
 * LED chaser
 * This program shows usage of FastArduino port API to handle several output pins at a time.
 * On Arduino, you should branch LED (in series with 330 Ohm resistors to limit current) on the following pins:
 * - D0-D7
 * i.e. all pins mapped to AVR ATmega328 PORT D
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <fastarduino/IO.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const REGISTER LED_PORT = Board::PORT_D;
#elif defined (ARDUINO_MEGA)
constexpr const REGISTER LED_PORT = Board::PORT_A;
#elif defined (BREADBOARD_ATTINYX4)
constexpr const REGISTER LED_PORT = Board::PORT_A;
#else
#error "Current target is not yet supported!"
#endif

int main()
{
	// Enable interrupts at startup time
	sei();

	// Set Port D direction to all outputs
	IOPort ledPort{LED_PORT};
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
