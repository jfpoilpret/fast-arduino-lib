/*
 * LED chaser, with input settings
 * This program shows usage of FastArduino port API to handle several output at a time, plus individual input pins.
 * On Arduino, you should branch LED (in series with 330 Ohm resistors to limit current) on the following pins:
 * - D0-D7
 * i.e. all pins mapped to AVR ATmega328 PORT D
 * Then, you should branch 4 switches (I use DIP switches which are convenient on breadboard):
 * - one side to A0-A2 (number of simultaneously lit LED) and A3 (chase direction)
 * - the other side to GND (we use internal pullup resistors for inputs)
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <fastarduino/FastPin.hh>

static inline uint8_t shift_pattern(uint8_t pattern, uint8_t shift)
{
	uint16_t result = (pattern << shift);
	return result | (result >> 8);
}

static inline uint8_t calculate_pattern(uint8_t num_bits)
{
	uint16_t pattern = (1 << (num_bits + 1)) - 1;
	return pattern;
}

int main()
{
	// Enable interrupts at startup time
	sei();
	// Prepare ports to read settings and write to LEDs
	IOPort PortC{Board::PORT_C, 0x00, 0x0F};
	IOPort PortD{Board::PORT_D, 0xFF};
	
	// Loop of the LED chaser
	while (true)
	{
		// Read settings everytime a LED chasing loop is about to start
		uint8_t settings = PortC.get_PIN();
		uint8_t pattern = calculate_pattern(settings & 0x07);
		bool direction = settings & 0x08;
		for (uint8_t i = 0; i < 8; ++i)
		{
			PortD.set_PORT(shift_pattern(pattern, (direction ? i : 7 - i)));
			_delay_ms(250.0);
		}
	}
	return 0;
}
