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

	//TODO would it be possible/useful to make Event a template on `value_` type?
	// In this case, we should also specialize template for T = void (no value needed)
	class Event
	{
	public:
		Event(uint8_t type = Type::NO_EVENT, uint16_t value = 0) INLINE : type_{type}, value_{value}
		{
		}
		uint8_t type() const INLINE
		{
			return type_;
		}
		uint16_t value() const INLINE
		{
			return value_;
		}

	private:
		uint8_t type_;
		uint16_t value_;
	};

	class EventHandler;

	// Dispatcher should be used only from non-interrupt code
	class Dispatcher : public containers::LinkedList<EventHandler>
	{
	public:
		void dispatch(const Event& event);
	};

	class EventHandler : public containers::Link<EventHandler>
	{
	public:
		uint8_t type() const INLINE
		{
			return type_;
		}

		virtual void on_event(const Event& event) = 0;

	protected:
		EventHandler(uint8_t type = Type::NO_EVENT) INLINE : type_{type}
		{
		}

		uint8_t type_;
		//TODO is this really needed?
		friend class Dispatcher;
	};
};
#endif /* EVENTS_HH */
