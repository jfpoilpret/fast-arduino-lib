/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * It does not do anything interesting as far as hardware is concerned.
 * It is just try-and-throw-away code.
 */


// Define vectors we need in the example
#include <fastarduino/queue.h>
#include <fastarduino/time.h>
#include <fastarduino/int.h>
#include <fastarduino/boards/board.h>

// Board-specifics are here only to check INT ISR
#define INT_NUM 0
static const board::DigitalPin INT_PIN = board::ExternalInterruptPin::D2_PD2_EXT0;

class Callback
{
public:
	Callback()
	{
		interrupt::register_handler(*this);
	}

	void callback()
	{
	}
};

#define INT_VECTOR(NUM) INT ## NUM ## _vect

ISR(INT_VECTOR(INT_NUM))
{
	static_assert(board_traits::DigitalPin_trait<INT_PIN>::IS_INT, "PIN must be an INT pin.");
	static_assert(board_traits::ExternalInterruptPin_trait<INT_PIN>::INT == INT_NUM, "PIN INT number must match INT_NUM");
	
	interrupt::HandlerHolder<Callback>::ArgsHolder<void>::CallbackHolder<&Callback::callback>::handle();
}

int main() __attribute__((OS_main));
int main()
{
	board::init();

	Callback callback;

	// Enable interrupts at startup time
	sei();

	while (true)
	{
	}
}
