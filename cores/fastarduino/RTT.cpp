#include "RTT.hh"
#include "time.hh"

RTT* RTT::_singleton = 0;

void RTT::_begin8(
	volatile uint8_t& TCCRA, volatile uint8_t& TCCRB, 
	volatile uint8_t& OCRA, volatile uint8_t& TCNT, volatile uint8_t& TIMSK)
{
	synchronized
	{
		// Use a timer with 1 ms interrupts
		// OCnA & OCnB disconnected, CTC (Clear Timer on Compare match)
		TCCRA = _BV(WGM01);
		// Don't force output compare (FOCA & FOCB), Clock Select clk/64 (CS = 3)
		TCCRB = _BV(CS00) | _BV(CS01);
		// Set timer counter compare match (when value reached, 1ms has elapsed)
		OCRA = (F_CPU / 64 / 1000) - 1;
		// Reset timer counter
		TCNT = 0;
		// Set timer interrupt mode (set interrupt on OCRnA compare match)
		TIMSK = _BV(OCIE0A);
		//TODO do we need to set delay here? This is not done by watchdog (why?)
		//TODO we should also ensure we will restore the right delay?
		// Change default delay function
//		Time::delay = RTT::delay;
	}
}

void RTT::_begin16(
	volatile uint8_t& TCCRA, volatile uint8_t& TCCRB, 
	volatile uint16_t& OCRA, volatile uint16_t& TCNT, volatile uint8_t& TIMSK)
{
	synchronized
	{
		// Use a timer with 1 ms interrupts
		// OCnA & OCnB disconnected, CTC (Clear Timer on Compare match)
		TCCRA = 0;
		// Don't force output compare (FOCA & FOCB), mode 2, Clock Select clk/1 (CS = 1)
		TCCRB = _BV(WGM12) | _BV(CS10);
		// Set timer counter compare match (when value reached, 1ms has elapsed)
		OCRA = (F_CPU / 1000) - 1;
		// Reset timer counter
		TCNT = 0;
		// Set timer interrupt mode (set interrupt on OCRnA compare match)
		TIMSK = _BV(OCIE1A);
		//TODO do we need to set delay here? This is not done by watchdog (why?)
		//TODO we should also ensure we will restore the right delay?
		// Change default delay function
//		Time::delay = RTT::delay;
	}
}

void RTT::_end(volatile uint8_t& TCCRB, volatile uint8_t& TIMSK)
{
	synchronized
	{
		// Stop timer
		TCCRB = 0;
		// Clear timer interrupt mode (set interrupt on OCRnA compare match)
		TIMSK = 0;
		// Restore default delay function
//		Time::delay = Time::default_delay;
	}
}

void RTT::delay(uint32_t ms) const
{
	uint32_t end = millis() + ms + 1;
	while (millis() < end)
		Time::yield();
}
