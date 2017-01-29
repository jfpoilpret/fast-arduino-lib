#ifndef WATCHDOG_HH
#define	WATCHDOG_HH

#include <avr/interrupt.h>
#include <avr/wdt.h>
#include "Events.hh"
#include "Board.hh"

//TODO Remove singleton and use standard ISR registration procedure with macro
#define REGISTER_WATCHDOG_CLOCK_ISR_METHOD()	\
REGISTER_ISR_METHOD_(WDT_vect, Watchdog, &Watchdog::on_tick)

#define REGISTER_WATCHDOG_ISR_METHOD(HANDLER, CALLBACK)	\
REGISTER_ISR_METHOD_(WDT_vect, HANDLER, CALLBACK)

#define REGISTER_WATCHDOG_ISR_FUNCTION(CALLBACK)	\
REGISTER_ISR_FUNCTION_(WDT_vect, CALLBACK)

#define REGISTER_INT_ISR_EMPTY() EMPTY_INTERRUPT(WDT_vect);

class WatchdogSignal
{
public:
	enum class TimeOut: uint8_t
	{
		TO_16ms = 0,
		TO_32ms,
		TO_64ms,
		TO_125ms,
		TO_250ms,
		TO_500ms,
		TO_1s,
		TO_2s,
		TO_4s,
		TO_8s
	};
	
	void begin(TimeOut timeout = TimeOut::TO_16ms);
	void end()
	{
		synchronized
		{
			WDTCSR = _BV(WDCE) | _BV(WDE);
			WDTCSR = 0;
		}
	}
	
protected:
	inline void _begin(uint8_t config) INLINE
	{
		wdt_reset();
		MCUSR |= 1 << WDRF;
		WDTCSR = _BV(WDCE) | _BV(WDE);
		WDTCSR = config;
	}
};

class Watchdog: public WatchdogSignal
{
public:
	Watchdog(Queue<Events::Event>& event_queue)
		:_millis{0}, _millis_per_tick{0}, _event_queue(event_queue) {}
	Watchdog(const Watchdog&) = delete;
	
	void register_watchdog_handler()
	{
		register_handler(*this);
	}
	
	void begin(TimeOut timeout = TimeOut::TO_16ms);
	
	uint32_t millis() const
	{
		synchronized return _millis;
	}
	void delay(uint32_t ms);
	
	void on_tick()
	{
		_millis += _millis_per_tick;
		_event_queue.push(Events::Event{Events::Type::WDT_TIMER});
	}
	
private:
	volatile uint32_t _millis;
	uint16_t _millis_per_tick;
	Queue<Events::Event>& _event_queue;
};

#endif	/* WATCHDOG_HH */
