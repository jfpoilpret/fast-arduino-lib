/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <stdint.h>

#include <avr/interrupt.h>

// General experiments here
template<typename Handler, typename... Args>
struct HandlerHolder
{
	using Holder = HandlerHolder<Handler, Args...>;
	static Handler* _handler;
	
	template<void (Handler::*Callback)(Args...)>
	struct CallbackHolder
	{
		static void handle(Args... args)
		{
			(Holder::_handler->*Callback)(args...);
		}
	};
};

template<typename Handler, typename... Args>
Handler* HandlerHolder<Handler, Args...>::_handler = 0;

struct TestHandler
{
	void act0() {}
	void act1(uint32_t) {}
	void act2(bool, uint32_t) {}
};

//static HandlerHolder<TestHandler, &TestHandler::act0> handler0;
//static HandlerHolder<TestHandler> handler0;
//static HandlerHolder<TestHandler, uint32_t> handler1;
//static HandlerHolder<TestHandler, bool, uint32_t> handler2;

static void test()
{
	TestHandler handler;
	HandlerHolder<TestHandler>::_handler = &handler;
	HandlerHolder<TestHandler>::CallbackHolder<&TestHandler::act0>::handle();

	HandlerHolder<TestHandler, uint32_t>::_handler = &handler;
	HandlerHolder<TestHandler, uint32_t>::CallbackHolder<&TestHandler::act1>::handle(1000UL);

	HandlerHolder<TestHandler, bool, uint32_t>::_handler = &handler;
	HandlerHolder<TestHandler, bool, uint32_t>::CallbackHolder<&TestHandler::act2>::handle(true, 1000UL);
}




int main() __attribute__((OS_main));
int main()
{
	sei();
	test();
	while (true) ;
}
