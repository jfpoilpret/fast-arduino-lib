#include "Events.hh"

//TODO should it be inlined in header?
class HandlerCaller
{
public:
	HandlerCaller(const Event::Event& event) __attribute__((always_inline)) : _event{event} {}
	void operator()(Event::AbstractHandler& handler) __attribute__((always_inline))
	{
		if (handler.type() == _event.type())
			handler.handle(_event);
	}
private:
	const Event::Event _event;
};

void Event::Dispatcher::dispatch(const Event& event)
{
	traverse(HandlerCaller(event));
}
