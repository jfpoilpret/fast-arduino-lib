/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

//#include <avr/interrupt.h>
//#include <avr/io.h>
#include <fastarduino/FastIO.hh>

#define MAKE_ISR(VECTOR, HANDLER)		\
ISR(VECTOR)								\
{										\
	HandlerHolder< HANDLER >::handle();	\
}

template<typename Handler> void register_handler(Handler&);

template<typename Handler>
class HandlerHolder
{
public:
	static void handle()
	{
		return _handler->callback();
	}
private:
	static Handler* _handler;
	friend void register_handler<Handler>(Handler&);
};

template<typename Handler>
Handler* HandlerHolder<Handler>::_handler = 0;

template<typename Handler>
void register_handler(Handler& handler)
{
	HandlerHolder<Handler>::_handler = &handler;
}

class MyHandler
{
public:
	MyHandler(): _led{PinMode::OUTPUT} {}
	void callback()
	{
		_led.toggle();
	}
private:
	FastPinType<Board::DigitalPin::LED>::TYPE _led;
};

MAKE_ISR(INT0_vect, MyHandler)

int main()
{
	sei();
	MyHandler my_handler;
	register_handler(my_handler);
	while (true) ;
}
