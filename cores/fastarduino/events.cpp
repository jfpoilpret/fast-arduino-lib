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

//FIXME have to pull that code back into events.h because template-based!
namespace events
{
	// template<typename T>
	// class HandlerCaller
	// {
	// public:
	// 	HandlerCaller(const Event<T>& event) INLINE : event_{event}
	// 	{
	// 	}
	// 	bool operator()(EventHandler<T>& handler) INLINE
	// 	{
	// 		if (handler.type() == event_.type()) handler.on_event(event_);
	// 		return false;
	// 	}

	// private:
	// 	const Event<T> event_;
	// };

	// template<typename T>
	// void Dispatcher<T>::dispatch(const Event<T>& event)
	// {
	// 	traverse(HandlerCaller<T>(event));
	// }
};
