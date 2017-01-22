/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/utilities.hh>

#define MAKE_ISR(VECTOR, HOLDER)	\
ISR(VECTOR)							\
{									\
	HOLDER :: _handler->callback();	\
}

template<typename Handler>
class HandlerHolder
{
private:
	static Handler* _handler;
	
	friend void TIMER0_COMPA_vect();
	template<typename Handler> friend void register_handler(Handler&);
};

template<typename Handler>
void register_handler(Handler& handler)
{
	HandlerHolder<Handler>::_handler = &handler;
}

class MyHandler
{
public:
	void callback();
};

int main()
{
	return 0;
}
