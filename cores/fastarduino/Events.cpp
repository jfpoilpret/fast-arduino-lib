#include "Events.hh"

namespace Events
{
	class HandlerCaller
	{
	public:
		HandlerCaller(const Event& event) INLINE : _event{event} {}
		bool operator()(AbstractHandler& handler) INLINE
		{
			if (handler.type() == _event.type())
				handler.handle(_event);
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
