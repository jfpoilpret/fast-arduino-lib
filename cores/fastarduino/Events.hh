#ifndef EVENTS_HH
#define	EVENTS_HH

#include <stddef.h>

namespace Event
{
	enum Type
	{
		WDT_TIMER
	} __attribute__((packed));

	class Event
	{
	public:
		Event(Type type, uint16_t value = 0) __attribute__((always_inline)) : _type{type}, _value{value} {}
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
	template<typename T> class Queue;
	
	//TODO avoid template at this level
	// Create a Queue also without a template
	class Dispatcher
	{
	public:
		Dispatcher(Queue<Event>& queue) __attribute__((always_inline)) : _queue{queue}, _handler{0} {};
		bool push(const Event& event);
		void add_handler(Handler* handler);
		void remove_handler(Handler* handler);
		bool dispatch();

	private:
		// Events Queue
		Queue<Event>& _queue;
		// Handlers linked list
		Handler* _handler;
	};

	class Handler
	{
	public:
		Handler(Type type) __attribute__((always_inline)) : _type{type}, _next{0} {}
		virtual bool on_event(const Event& event) = 0;
		
	private:
		const Type _type;
		Handler* _next;
		
		friend class Dispatcher;
	};
	
	//TODO define different kinds of Handlers here
};
#endif	/* EVENTS_HH */
