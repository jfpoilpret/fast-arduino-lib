/*
 * FastIO LED chaser.
 * This program shows usage of FastArduino FastIO API with masked ports.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D0-D7 (port D) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on Arduino MEGA:
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
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

static const uint8_t NUM_LEDS = 8;

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();
	// Prepare ports to write to LEDs
	FastMaskedPort<LED_PORT> pins[NUM_LEDS];
	for (uint8_t i = 0; i < NUM_LEDS; ++i)
		pins[i] = FastMaskedPort<LED_PORT>{_BV(i), 0xFF};
	
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
