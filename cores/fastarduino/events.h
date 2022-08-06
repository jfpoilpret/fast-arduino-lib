//   Copyright 2016-2022 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/// @cond api

/**
 * @file
 * Support for events management.
 */
#ifndef EVENTS_HH
#define EVENTS_HH

#include <stddef.h>
#include "queue.h"
#include "linked_list.h"

/**
 * Defines all API to handle events within FastArduino programs.
 * Some of FastArduino API use this API for event support.
 * 
 * In FastArduino, Events are handled mainly through an event queue, created
 * by your program and passed to event API:
 * @code
 * // Define the Event type we need (no additional value)
 * using namespace events;
 * using EVENT = Event<void>;
 * // Define the event queue we will use in the program
 * EVENT buffer[32];
 * containers::Queue<EVENT> event_queue{buffer};
 * 
 * int main()
 * {
 *     ... initialization code here ...
 *     // Main event loop
 *     while (true)
 *     {
 *         EVENT event = containers::pull(event_queue);
 *         // Handle event here
 *     }
 * }
 * @endcode
 * Actually, the code above does not show FastArduino Event handling API but only the 
 * `Event` type.
 * 
 * Event support provides some API to more easily handle events once pulled from the 
 * event queue:
 * @code
 * // Define an event handler
 * class MyHandler: public EventHandler<EVENT> {...};
 * 
 * int main()
 * {
 *     // Instantiate event handlers (one per type of event)
 *     MyHandler handler;
 * 
 *     // Create Dispatcher and register each handler
 *     Dispatcher<EVENT> dispatcher;
 *     dispatcher.insert(handler);
 * 
 *     // Main event loop
 *     while (true)
 *     {
 *         EVENT event = containers::pull(event_queue);
 *         dispatcher.dispatch(event);
 *     }
 * }
 * @endcode
 * 
 */
namespace events
{
	/**
	 * Defines pre-defined types of events gegenarted by FastArduino API.
	 * The type of an event is coded as an unsigned byte.
	 * 
	 * All numbers from 0 to 127 are reserved for fastArduino library and should
	 * not be used for user-defined custom event types.
	 * 
	 * @sa Event
	 */
	namespace Type
	{
		/**
		 * Special event type attached to no event at all. This may be used by
		 * methods returning an event without waiting for one to be available.
		 */
		const uint8_t NO_EVENT = 0;

		/**
		 * Type of events generated by `watchdog::Watchdog` for each watchdog 
		 * timeout interrupt.
		 * @sa watchdog::Watchdog
		 */
		const uint8_t WDT_TIMER = 1;

		/**
		 * Type of events generated by `timer::RTTEventCallback` whenever elapsed
		 * `RTT::millis()` reaches a multiple of `PERIOD_MS`.
		 * @sa timer::RTTEventCallback
		 */
		const uint8_t RTT_TIMER = 2;

		/**
		 * The first ordinal event type that you may use for your own custom events.
		 * You would generally define all your custom event types as constant:
		 * @code
		 * static const uint8_t MY_EVENT_TYPE = events::Type::USER_EVENT;
		 * static const uint8_t MY_OTHER_EVENT_TYPE = events::Type::USER_EVENT + 1;
		 * @endcode
		 * You can define up to 127 event types on your own, from 128 to 255.
		 */
		const uint8_t USER_EVENT = 128;
	};

	/**
	 * A standard Event as managed by FastArduino event API. By default an event just holds
	 * a type as defined in `events::Type`.
	 * It may also hold a value of any type @p T, which you may use for any purpose you see fit.
	 * 
	 * Please note that calue type @p T will have an impact on the byte size of each event, hence
	 * the size of your event queue. You should then select the smallest type that you need,
	 * and even `void` if you do not need any additional value for your events.
	 * 
	 * All FastArduino API that produce events do not use any value for events, only their type, 
	 * hence they will work with ANY `Event<T>` you may define.
	 * 
	 * @tparam T the type of value held by all events; this type must have public 
	 * default and copy constructors. T shall not be a reference type.
	 * 
	 * @sa events::Type
	 */
	template<typename T> class Event
	{
	public:
		Event(const Event<T>&) = default;
		Event<T>& operator=(const Event<T>&) = default;

		/**
		 * The type of additional event value, as defined in template paraneter @p T.
		 */
		using TYPE = T;

		/**
		 * Create a new event with the given @p type and the given @p value.
		 * @param type the type of this event, `Type::NO_EVENT` by default.
		 * @param value the value of this event, `T{}` by default; for an `Event<void>`, 
		 * this argument shall not be provided or a compile error will occur.
		 */
		explicit Event(uint8_t type = Type::NO_EVENT, T value = T{}) INLINE : type_{type}, value_{value} {}

		/**
		 * The type of this event.
		 */
		uint8_t type() const INLINE
		{
			return type_;
		}

		/**
		 * The associated value of this event.
		 * For an `Event<void>`, this method does not exist and shall not be called, otherwise
		 * a compile error will occur.
		 */
		T value() const INLINE
		{
			return value_;
		}

	private:
		uint8_t type_;
		T value_;
	};

	/// @cond notdocumented
	template<> class Event<void>
	{
	public:
		Event(const Event<void>&) = default;
		Event<void>& operator=(const Event<void>&) = default;

		using TYPE = void;

		explicit Event(uint8_t type = Type::NO_EVENT) INLINE : type_{type} {}
		uint8_t type() const INLINE
		{
			return type_;
		}

	private:
		uint8_t type_;
	};
	/// @endcond

	/// @cond notdocumented
	template<typename T> struct Event_trait
	{
		static constexpr const bool IS_EVENT = false;
	};
	// Note: the following line compiled with GCC 7.4 but does not with 9.2 and 10.2
	// template<> template<typename T> struct Event_trait<Event<T>>
	// Replaced with this line to make GCC 9.2 happy
	template<typename T> struct Event_trait<Event<T>>
	{
		static constexpr const bool IS_EVENT = true;
	};
	/// @endcond

	/// @cond notdocumented
	template<typename EVENT> class EventHandler;
	/// @endcond

	/**
	 * Utility to dispatch an event to a list of `EventHandler`s that are registered for
	 * its type.
	 * You first create an `EventHandler` subclass, intantiate it and register it to 
	 * a `Dispatcher`; for this, you use `containers::LinkedList` API which `Dispatcher`
	 * derives from.
	 * 
	 * NOTE: you should never call any `Dispatcher` method from an ISR because these methods
	 * may last too long for an ISR.
	 * 
	 * @tparam EVENT the `events::Event<T>` handled
	 * 
	 * @sa containers::LinkedList
	 * @sa EventHandler
	 * @sa Event::type()
	 */
	template<typename EVENT> class Dispatcher : public containers::LinkedList<EventHandler<EVENT>>
	{
		static_assert(Event_trait<EVENT>::IS_EVENT, "EVENT type must be an events::Event<T>");

	public:
		Dispatcher() = default;
		Dispatcher(const Dispatcher<EVENT>&) = delete;
		Dispatcher<EVENT>& operator=(const Dispatcher<EVENT>&) = delete;

		/**
		 * Dispatch the given @p event to the right `EventHandler`, based on the event type.
		 * Note that if several registered `EventHandler`s match this @p event type,
		 * then they will all be called with that event.
		 * 
		 * @param event the event to dispatch to the right event handler
		 * @sa EventHandler::on_event()
		 */
		void dispatch(const EVENT& event)
		{
			this->traverse(HandlerCaller(event));
		}

	private:
		class HandlerCaller
		{
		public:
			explicit HandlerCaller(const EVENT& event) INLINE : event_{event} {}
			bool operator()(EventHandler<EVENT>& handler) INLINE
			{
				if (handler.type() == event_.type()) handler.on_event(event_);
				return false;
			}

		private:
			const EVENT event_;
		};
	};

	/**
	 * Abstract event handler, used by `Dispatcher` to get called back when an 
	 * event of the expected type is dispatched.
	 * 
	 * @tparam EVENT the `events::Event<T>` dispatched
	 * 
	 * @sa Dispatcher::dispatch()
	 */
	template<typename EVENT> class EventHandler : public containers::Link<EventHandler<EVENT>>
	{
		static_assert(Event_trait<EVENT>::IS_EVENT, "EVENT type must be an events::Event<T>");

	public:
		/**
		 * The type of event that this handler accepts and can act upon.
		 */
		uint8_t type() const INLINE
		{
			return type_;
		}

	protected:
		EventHandler(const EventHandler<EVENT>&) = default;
		EventHandler<EVENT>& operator=(const EventHandler<EVENT>&) = default;
		
		/**
		 * This pure virtual method is called by `Dispatcher::dispatch()` when
		 * @p event.type() matches the type supported by this `EventHandler`.
		 * @param event the event to handle
		 */
		virtual void on_event(const EVENT& event) = 0;

		/**
		 * Create an Event Handler for given @p type of event.
		 * @param type the evnt type handled by this handler
		 */
		explicit EventHandler(uint8_t type = Type::NO_EVENT) INLINE : type_{type} {}

	private:
		uint8_t type_;
		friend class Dispatcher<EVENT>;
	};
};
#endif /* EVENTS_HH */
/// @endcond
