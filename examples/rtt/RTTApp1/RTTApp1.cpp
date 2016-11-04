/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/RTT.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
// Define vectors we need in the example
USE_RTT_TIMER0()
USE_RTT_TIMER1()
USE_RTT_TIMER2()
#elif defined (ARDUINO_MEGA)
// Define vectors we need in the example
USE_RTT_TIMER0()
USE_RTT_TIMER1()
USE_RTT_TIMER2()
USE_RTT_TIMER3()
USE_RTT_TIMER4()
USE_RTT_TIMER5()
#elif defined (BREADBOARD_ATTINYX4)
// Define vectors we need in the example
USE_RTT_TIMER0()
USE_RTT_TIMER1()
#else
#error "Current target is not yet supported!"
#endif

const constexpr uint32_t BLINK_DELAY = 10000;

template<Board::Timer TIMER>
void check_timer()
{
	FastPin<Board::DigitalPin::LED> led{PinMode::OUTPUT, false};
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
