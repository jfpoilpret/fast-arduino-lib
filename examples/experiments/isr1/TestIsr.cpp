/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <stdint.h>

#include <avr/interrupt.h>

// General experiments here
template<typename Handler>
struct HandlerHolder
{
	using Holder = HandlerHolder<Handler>;
	static Handler* _handler;

	template<typename... Args>
	struct ArgsHodler
	{
		template<void (Handler::*Callback)(Args...)>
		struct CallbackHolder
		{
			static void handle(Args... args)
			{
				(Holder::_handler->*Callback)(args...);
			}
		};
	};
};

template<typename Handler>
Handler* HandlerHolder<Handler>::_handler = 0;

template<typename Handler>
void register_handler(Handler& handler)
{
	HandlerHolder<Handler>::_handler = &handler;
}

struct TestHandler
{
	void act0() {}
	void act1(uint32_t) {}
	void act2(bool, uint32_t) {}
};

static void test()
{
	TestHandler handler;
	register_handler(handler);
//	HandlerHolder<TestHandler>::_handler = &handler;

	HandlerHolder<TestHandler>::ArgsHodler<>::CallbackHolder<&TestHandler::act0>::handle();
	HandlerHolder<TestHandler>::ArgsHodler<uint32_t>::CallbackHolder<&TestHandler::act1>::handle(1000UL);
	HandlerHolder<TestHandler>::ArgsHodler<bool, uint32_t>::CallbackHolder<&TestHandler::act2>::handle(true, 1000UL);
}




int main() __attribute__((OS_main));
int main()
{
	sei();
	test();
	while (true) ;
}
