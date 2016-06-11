/*
 * This program is the Hello World of Arduino: blink LED on D13.
 * So far this is just an active loop without any advanced features such as watchdog or timed tasks.
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <fastarduino/FastPin.hh>

static FastPin<Board::LED> PinLED{PinMode::OUTPUT};

int main()
{
	sei();
	while (true)
	{
		PinLED.toggle();
		_delay_ms(200.0);
		_delay_ms(200.0);
	}
	return 0;
}
