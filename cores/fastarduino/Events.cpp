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

#include "events.h"

namespace Events
{
	class HandlerCaller
	{
	public:
		HandlerCaller(const Event& event) INLINE : _event{event} {}
		bool operator()(EventHandler& handler) INLINE
		{
			if (handler.type() == _event.type())
				handler.on_event(_event);
			return false;
		}
	private:
		const Event _event;
	};

	void Dispatcher::dispatch(const Event& event)
	{
		traverse(HandlerCaller(event));
	}
};
