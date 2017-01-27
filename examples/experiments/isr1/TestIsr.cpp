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

#define REGISTER_ISR_METHOD_(HANDLER, CALLBACK,...)	\
HandlerHolder< HANDLER >::ArgsHodler< __VA_ARGS__ >::CallbackHolder< CALLBACK >::handle

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

	REGISTER_ISR_METHOD_(TestHandler, &TestHandler::act0)();
	REGISTER_ISR_METHOD_(TestHandler, &TestHandler::act1, uint32_t)(1000UL);
	REGISTER_ISR_METHOD_(TestHandler, &TestHandler::act2, bool, uint32_t)(true, 1000UL);
}

int main() __attribute__((OS_main));
int main()
{
	sei();
	test();
	while (true) ;
}
