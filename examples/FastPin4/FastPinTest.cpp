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

int main()
{
	// Enable interrupts at startup time
	sei();
	// Read settings for pattern
	FastPort<_SFR_IO_ADDR(PINC)> PortC{0x00, 0x0F};
//	PortC.set_DDR(0x00);
//	PortC.set_PORT(0x0F);
	
	uint8_t pattern = PortC.get_PIN() & 0x0F;
	// Read settings for direction
	FastPin<Board::D8> PinDirection{PinMode::INPUT_PULLUP};
	bool direction = PinDirection.value();

	// Set Port D direction to all outputs
	FastPort<_SFR_IO_ADDR(PIND)> PortD{0xFF};
//	PortD.set_DDR(0xFF);
	
	//DEBUG show pattern and direction on LEDs during 5 seconds
	PortD.set_PORT((pattern << 4) | direction);
	for (uint8_t i = 0; i < 20; i++)
		_delay_ms(250.0);
	
	uint8_t value = 0;
	// Loop of the LED chaser
	//FIXME improve pattern rotation
	while (true)
	{
		if (value == 0)
			value = (direction ? 0x80 : 0x01);
//			value = pattern;
		else if (direction)
			value >>= 1;
		else
			value <<= 1;
//		PortD.set_PORT(pattern);
		PortD.set_PORT(value);
		_delay_ms(250.0);
//		PortD.set_PORT(0);
//		_delay_ms(250.0);
	}
	return 0;
}
