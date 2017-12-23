//   Copyright 2016-2017 Jean-Francois Poilpret
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

#ifndef EVENTS_HH
#define EVENTS_HH

#include <stddef.h>
#include "queue.h"
#include "linked_list.h"

namespace events
{
	namespace Type
	{
		// Library-defined event types
		const uint8_t NO_EVENT = 0;
		const uint8_t WDT_TIMER = 1;
		const uint8_t RTT_TIMER = 2;

		// User-defined events start here (in range [128-255]))
		const uint8_t USER_EVENT = 128;
	};

	// T must have public default and copy constructors
	template<typename T>
	class Event
	{
	public:
		using TYPE = T;
		
		Event(uint8_t type = Type::NO_EVENT, T value = T{}) INLINE : type_{type}, value_{value}
		{
		}
		uint8_t type() const INLINE
		{
			return type_;
		}
		T value() const INLINE
		{
			return value_;
		}

	private:
		uint8_t type_;
		T value_;
	};

	template<>
	class Event<void>
	{
	public:
		using TYPE = void;
		
		Event(uint8_t type = Type::NO_EVENT) INLINE : type_{type}
		{
		}
		uint8_t type() const INLINE
		{
			return type_;
		}

	private:
		uint8_t type_;
	};

	template<typename T> struct Event_trait
	{
		static constexpr const bool IS_EVENT = false;
	};
	template<>
	template<typename T> struct Event_trait<Event<T>>
	{
		static constexpr const bool IS_EVENT = true;
	};

	template<typename EVENT>
	class EventHandler;

	// Dispatcher should be used only from non-interrupt code
	template<typename EVENT>
	class Dispatcher : public containers::LinkedList<EventHandler<EVENT>>
	{
		static_assert(Event_trait<EVENT>::IS_EVENT, "EVENT type must be an events::Event<T>");

	public:
		void dispatch(const EVENT& event)
		{
			this->traverse(HandlerCaller(event));
		}
	
	private:
		class HandlerCaller
		{
		public:
			HandlerCaller(const EVENT& event) INLINE : event_{event}
			{
			}
			bool operator()(EventHandler<EVENT>& handler) INLINE
			{
				if (handler.type() == event_.type()) handler.on_event(event_);
				return false;
			}

		private:
			const EVENT event_;
		};
	};

	template<typename EVENT>
	class EventHandler : public containers::Link<EventHandler<EVENT>>
	{
		static_assert(Event_trait<EVENT>::IS_EVENT, "EVENT type must be an events::Event<T>");
		
	public:
		uint8_t type() const INLINE
		{
			return type_;
		}

		virtual void on_event(const EVENT& event) = 0;

	protected:
		EventHandler(uint8_t type = Type::NO_EVENT) INLINE : type_{type}
		{
		}

		uint8_t type_;
		//TODO is this really needed?
		friend class Dispatcher<EVENT>;
	};
};
#endif /* EVENTS_HH */
