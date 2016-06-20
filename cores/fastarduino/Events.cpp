#include "Events.hh"

namespace Events
{
	class HandlerCaller
	{
	public:
		HandlerCaller(const Events::Event& event) __attribute__((always_inline)) : _event{event} {}
		void operator()(Events::AbstractHandler& handler) __attribute__((always_inline))
		{
			if (handler.type() == _event.type())
				handler.handle(_event);
		}
	private:
		const Events::Event _event;
	};

	void Dispatcher::dispatch(const Event& event)
	{
		traverse(HandlerCaller(event));
	}
};
