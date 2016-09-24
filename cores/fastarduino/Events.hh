#ifndef EVENTS_HH
#define	EVENTS_HH

#include <stddef.h>
#include "Queue.hh"
#include "LinkedList.hh"

namespace Events
{
	namespace Type
	{
		// Library-defined event types
		const uint8_t NO_EVENT = 0;
		const uint8_t WDT_TIMER = 1;

		// User-defined events start here (in range [128-255]))
		const uint8_t USER_EVENT = 128;
	};

	class Event
	{
	public:
		Event(uint8_t type = Type::NO_EVENT, uint16_t value = 0) INLINE
			: _type{type}, _value{value} {}
		uint8_t type() const INLINE
		{
			return _type;
		}
		uint16_t value() const INLINE
		{
			return _value;
		}

	private:
		uint8_t _type;
		uint16_t _value;
	};

	class EventHandler;

	// Dispatcher should be used only from non-interrupt code
	class Dispatcher: public LinkedList<EventHandler>
	{
	public:
		void dispatch(const Event& event);
	};

	class EventHandler: public Link<EventHandler>
	{
	public:
		uint8_t type() const INLINE
		{
			return _type;
		}

		virtual void on_event(const Event& event) = 0;
		
	protected:
		EventHandler(uint8_t type = Type::NO_EVENT) INLINE
			: _type{type} {}
		
		uint8_t _type;
		//TODO is this really needed?
		friend class Dispatcher;
	};
};
#endif	/* EVENTS_HH */
