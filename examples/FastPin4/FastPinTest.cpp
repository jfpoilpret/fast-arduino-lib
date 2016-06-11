/*
 * LED chaser, with input settings
 * This program shows usage of FastArduino port API to handle several output at a time, plus individual input pins.
 * On Arduino, you should branch LED (in series with 330 Ohm resistors to limit current) on the following pins:
 * - D0-D7
 * i.e. all pins mapped to AVR ATmega328 PORT D
 * Then, you should branch 5 switches (I use DIP switches which are convenient on breadboard):
 * - one side to A0-A3 (LED pattern) or D8 (chase direction)
 * - the other side to GND (we use internal pullup resistors for inputs)
 * 
 * TODO more switches (2) to control speed
 */

#include <avr/interrupt.h>
#include <util/delay.h>
#include <fastarduino/FastPin.hh>

static inline uint8_t calculate_pattern(uint16_t model, uint8_t shift)
{
	//FIXME this does not work when eg pattern is 0xF
	return (model << shift) >> 4;
}

static inline uint16_t calculate_model(uint8_t pattern)
{
	//TODO change model to use pattern as the number of bits to light?
	uint16_t model = (1 << (pattern + 1)) - 1;
	return model | (model << 8);
//	return pattern | (pattern << 8);
}

int main()
{
	// Enable interrupts at startup time
	sei();
	// Prepare ports to read settings and write to LEDs
	FastPort<_SFR_IO_ADDR(PINC)> PortC{0x00, 0x1F};
	FastPort<_SFR_IO_ADDR(PIND)> PortD{0xFF};
	
	// Loop of the LED chaser
	while (true)
	{
		volatile uint8_t settings = PortC.get_PIN();
		uint16_t model = calculate_model(settings & 0x07);
		bool direction = settings & 0x10;
		for (uint8_t i = 0; i < 8; ++i)
		{
			PortD.set_PORT(calculate_pattern(model, (direction ? i : 7 - i)));
			_delay_ms(250.0);
		}
	}
	return 0;
}
