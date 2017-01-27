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

#define CALL_HANDLER(HANDLER, CALLBACK,...)	\
HandlerHolder< HANDLER >::ArgsHodler< __VA_ARGS__ >::CallbackHolder< CALLBACK >::handle

#define CALLBACK_HANDLER(HANDLER, CALLBACK,...)	\
CALL_HANDLER(HANDLER, CALLBACK, ##__VA_ARGS__)

struct TestHandler
{
	TestHandler(): _count(0) {}
	
	void act0() {++_count;}
	void act1(uint32_t arg) {_count += arg;}
	void act2(bool inc, uint32_t arg) {if (inc) _count += arg; else _count -= arg;}
	
	uint16_t _count;
};

static void test()
{
	CALL_HANDLER(TestHandler, &TestHandler::act0)();
	CALL_HANDLER(TestHandler, &TestHandler::act1, uint32_t)(1000UL);
	CALL_HANDLER(TestHandler, &TestHandler::act2, bool, uint32_t)(false, 500UL);
	CALLBACK_HANDLER(TestHandler, &TestHandler::act0)();
	CALLBACK_HANDLER(TestHandler, &TestHandler::act1, uint32_t)(2000UL);
	CALLBACK_HANDLER(TestHandler, &TestHandler::act2, bool, uint32_t)(true, 100UL);
}

int main() __attribute__((OS_main));
int main()
{
	sei();
	
	TestHandler handler;
	register_handler(handler);

	while (handler._count < 10000) test();
}
