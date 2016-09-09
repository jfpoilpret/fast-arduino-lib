/*
 * This program is the Hello World of Arduino: blink LED on D13.
 * So far this is just an active loop without any advanced features such as watchdog or timed tasks.
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <fastarduino/IO.hh>

int main()
{
	sei();
	IOPin PinLED{Board::DigitalPin::LED, PinMode::OUTPUT};
	while (true)
	{
		PinLED.toggle();
		_delay_ms(400.0);
	}
	return 0;
}
