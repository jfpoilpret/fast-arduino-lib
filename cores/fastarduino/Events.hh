#ifndef EVENTS_HH
#define	EVENTS_HH

#include <stddef.h>
#include "Queue.hh"
#include "LinkedList.hh"

namespace Event
{
	enum Type
	{
		NO_EVENT = 0,
		WDT_TIMER = 1
	} __attribute__((packed));

	class Event
	{
	public:
		Event(Type type = NO_EVENT, uint16_t value = 0) __attribute__((always_inline)) : _type{type}, _value{value} {}
		Type type() const __attribute__((always_inline))
		{
			return _type;
		}
		uint16_t value() const __attribute__((always_inline))
		{
			return _value;
		}

	private:
		const Type _type;
		const uint16_t _value;
	};

	class Handler;

	// Dispatcher should be used only from non-interrupt code
	class Dispatcher: public LinkedList<Handler>
	{
	public:
		void dispatch(const Event& event);
	};

	class Handler: public Link<Handler>
	{
	public:
		Handler(Type type) __attribute__((always_inline)) : _type{type}, _next{0} {}
		virtual void on_event(const Event& event) = 0;
		
	private:
		const Type _type;
		Handler* _next;
		
		friend class Dispatcher;
	};
	
	//TODO define different kinds of Handlers here
};
#endif	/* EVENTS_HH */
