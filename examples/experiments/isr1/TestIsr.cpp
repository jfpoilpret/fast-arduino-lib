/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <stdint.h>

#include <avr/interrupt.h>

// Utilities to handle ISR callbacks
#define REGISTER_ISR_METHOD_(VECTOR, HANDLER, CALLBACK)		\
ISR(VECTOR)													\
{															\
	HandlerCallbackHolder< HANDLER , CALLBACK >::handle();	\
}

template<typename Handler> void register_handler(Handler&);
template<typename Handler>
class HandlerHolder
{
public:
	static Handler* handler()
	{
		return _handler;
	}
private:
	static Handler* _handler;
	friend void register_handler<Handler>(Handler&);
};

template<typename Handler>
Handler* HandlerHolder<Handler>::_handler = 0;

template<typename Handler, void(Handler::*Callback)()>
class HandlerCallbackHolder: public HandlerHolder<Handler>
{
public:
	static void handle()
	{
		Handler* handler_instance = HandlerHolder<Handler>::handler();
//		FIX_BASE_POINTER(handler_instance);
		return (handler_instance->*Callback)();
	}
};

template<typename Handler>
void register_handler(Handler& handler)
{
	HandlerHolder<Handler>::_handler = &handler;
}

class MyHandler
{
public:
	MyHandler() {}
	void callback()
	{
		++_count;
	}
	uint16_t _count;
};

REGISTER_ISR_METHOD_(INT0_vect, MyHandler, &MyHandler::callback)

int main() __attribute__((OS_main));
int main()
{
	sei();
	MyHandler my_handler;
	register_handler(my_handler);
	while (true) ;
}
