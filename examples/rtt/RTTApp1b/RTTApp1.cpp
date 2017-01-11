/*
 * Real Time Timer example. Take #1
 * This program shows usage of FastArduino Timer-based RTT (Real Time Timer) support.
 * It checks RTT with all available timers of the target board.
 * For each available timer on the target platform, the program blinks a LED 5 times with a period of 10 seconds.
 * 
 * Wiring:
 * - on Arduino UNO and Arduino MEGA:
 *   - no wiring needed as the program uses default LED on D13
 * - on ATmega328P based boards:
 *   - D13 (PB5) connected to a LED through a 330Ohm resistor then linked to GND
 * - on ATtinyX4 based boards:
 *   - D7 (LED, PA7) connected to a LED through a 330Ohm resistor then linked to GND
 */

#include <avr/interrupt.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>

USE_TIMERS()

const constexpr uint32_t BLINK_DELAY = 10000;

template<Board::Timer TIMER>
void check_timer()
{
	typename FastPinType<Board::DigitalPin::LED>::TYPE led{PinMode::OUTPUT, false};
	RTT<TIMER> rtt;
	rtt.begin();
	// Event Loop
	for (uint8_t i = 0; i < 5; ++i)
	{
		led.toggle();
		rtt.delay(BLINK_DELAY);
	}
	rtt.end();
}

int main()
{
	// Enable interrupts at startup time
	sei();

#if defined (BREADBOARD_ATTINYX4)
	check_timer<Board::Timer::TIMER0>();
	check_timer<Board::Timer::TIMER1>();
#elif defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
	check_timer<Board::Timer::TIMER0>();
	check_timer<Board::Timer::TIMER1>();
	check_timer<Board::Timer::TIMER2>();
#elif defined (ARDUINO_MEGA)
	check_timer<Board::Timer::TIMER0>();
	check_timer<Board::Timer::TIMER1>();
	check_timer<Board::Timer::TIMER2>();
	check_timer<Board::Timer::TIMER3>();
	check_timer<Board::Timer::TIMER4>();
	check_timer<Board::Timer::TIMER5>();
#endif
}
