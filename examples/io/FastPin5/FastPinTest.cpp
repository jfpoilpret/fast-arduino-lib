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
#include <fastarduino/IO.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const REGISTER LED_PORT = Board::PORT_D;
#elif defined (ARDUINO_MEGA)
static constexpr const REGISTER LED_PORT = Board::PORT_A;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const REGISTER LED_PORT = Board::PORT_A;
#else
#error "Current target is not yet supported!"
#endif

static const uint8_t NUM_LEDS = 8;

int main()
{
	// Enable interrupts at startup time
	sei();
	// Prepare ports to write to LEDs
	IOMaskedPort pins[NUM_LEDS];
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
	{
		pins[i] = IOMaskedPort{LED_PORT, _BV(i), 0xFF};
	}
	
	// Loop of the LED chaser
	while (true)
	{
		for (uint8_t i = 0; i < 8; ++i)
		{
			pins[i].set_PORT(0xFF);
			_delay_ms(250.0);
			pins[i].set_PORT(0x00);
			_delay_ms(250.0);
		}
	}
	return 0;
}
