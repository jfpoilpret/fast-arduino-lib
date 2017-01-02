/*
 * Potentiometer value reading example.
 * This program shows usage of FastArduino AnalogInput API.
 * It reads and converts the analog level on a pin and displays it as a level (in binary) in a range of 8 LEDs.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - A0: connected to the wiper of a 10K pot or trimmer, which terminals are connected between Vcc and Gnd
 *   - D0-D7 (port D) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on Arduino MEGA:
 *   - TODO
 *   - D22-D29 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 * - on ATtinyX4 based boards:
 *   - TODO
 *   - D0-D7 (port A) branch 8 LED (in series with 330 Ohm resistors to limit current) connected to ground
 */

#include <avr/interrupt.h>
#include <fastarduino/time.hh>
#include <fastarduino/AnalogInput.hh>
#include <fastarduino/FastIO.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const Board::AnalogPin POT = Board::AnalogPin::A0;
static constexpr const Board::Port LED_PORT = Board::Port::PORT_D;
static constexpr const uint8_t LED_MASK = 0xFF;
#elif defined (ARDUINO_MEGA)
static constexpr const Board::AnalogPin POT = Board::AnalogPin::A0;
static constexpr const Board::Port LED_PORT = Board::Port::PORT_A;
static constexpr const uint8_t LED_MASK = 0xFF;
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const Board::AnalogPin POT = Board::AnalogPin::A7;
static constexpr const Board::Port LED_PORT = Board::Port::PORT_A;
static constexpr const uint8_t LED_MASK = 0x7F;
#else
#error "Current target is not yet supported!"
#endif

using ANALOG_INPUT = AnalogInput<POT, Board::AnalogReference::AVCC, uint8_t, Board::AnalogClock::MAX_FREQ_200KHz>;

int main()
{
	// Enable interrupts at startup time
	sei();
	
	FastMaskedPort<LED_PORT> leds{LED_MASK, 0xFF};
	// Declare Analog input
	ANALOG_INPUT pot;

	// Loop of samplings
	while (true)
	{
		ANALOG_INPUT::TYPE value = pot.sample() >> 1;
		leds.set_PORT(value);
		Time::delay_ms(1000);
	}
	return 0;
}
