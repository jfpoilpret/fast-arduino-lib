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
		Event(uint8_t type = Type::NO_EVENT, uint16_t value = 0) __attribute__((always_inline))
			: _type{type}, _value{value} {}
		uint8_t type() const __attribute__((always_inline))
		{
			return _type;
		}
		uint16_t value() const __attribute__((always_inline))
		{
			return _value;
		}

	private:
		uint8_t _type;
		uint16_t _value;
	};

	class AbstractHandler;

	// Dispatcher should be used only from non-interrupt code
	class Dispatcher: public LinkedList<AbstractHandler>
	{
	public:
		void dispatch(const Event& event);
	};

	//TODO enable broader filter of events for handlers?
	//TODO Let handlers decide if they forward events or not?
	// AbstractHandler used on more specific handlers types below
	// This class should normally never be used directly by developers
	class AbstractHandler: public Link<AbstractHandler>
	{
	public:
		//TODO make it private? (only Dispatcher should call it)
		void handle(const Event& event) __attribute__((always_inline))
		{
			_f(_env, event);
		}
		uint8_t type() const __attribute__((always_inline))
		{
			return _type;
		}

	private:
		typedef void (*F)(void* env, const Event& event);
		AbstractHandler(uint8_t type = Type::NO_EVENT, void* env = 0, F f = 0) __attribute__((always_inline))
			: _type{type}, _f{f}, _env{env} {}
		
		uint8_t _type;
		F _f;
		void* _env;
		
		friend class VirtualHandler;
		template<typename FUNCTOR>
		friend class FunctorHandler;
	};
	
	// Derive this class to define event handlers based on a virtual method.
	class VirtualHandler: public AbstractHandler
	{
	protected:
		VirtualHandler(uint8_t type = Type::NO_EVENT) __attribute__((always_inline))
			: AbstractHandler{type, this, apply} {}
		virtual void on_event(const Event& event) = 0;
		
	private:
		static void apply(void* env, const Event& event) __attribute__((always_inline))
		{
			((VirtualHandler*) env)->on_event(event);
		}
	};

	// Instantiate this template with a Functor when a functor is applicable.
	// FUNCTOR must be a class defining:
	// void operator()(const Event&);
	// This approach generally gives smaller code and data than VirtualHandler approach
	template<typename FUNCTOR>
	class FunctorHandler: public AbstractHandler
	{
	public:
		FunctorHandler() __attribute__((always_inline)) : AbstractHandler{} {}
		FunctorHandler(uint8_t type, FUNCTOR f) __attribute__((always_inline))
			: AbstractHandler{type, this, apply}, _f{f} {}
	private:
		static void apply(void* env, const Event& event) __attribute__((always_inline))
		{
			((FunctorHandler<FUNCTOR>*) env)->_f(event);
		}
		FUNCTOR _f;
	};
};
#endif	/* EVENTS_HH */
