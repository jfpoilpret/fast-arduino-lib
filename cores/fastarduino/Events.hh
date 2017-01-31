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
		const uint8_t RTT_TIMER = 2;

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
