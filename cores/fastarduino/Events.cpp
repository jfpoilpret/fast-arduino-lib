#include "Events.hh"

class HandlerCaller
{
public:
	HandlerCaller(const Event::Event& event) __attribute__((always_inline)) : _event{event} {}
	void operator()(Event::Handler& handler) __attribute__((always_inline))
	{
		handler.on_event(_event);
	}
private:
	const Event::Event& _event;
};

void Event::Dispatcher::dispatch(const Event& event)
{
	traverse(HandlerCaller(event));
}
