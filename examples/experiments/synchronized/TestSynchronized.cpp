/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * It does not do anything interesting as far as hardware is concerned.
 * It is just try-and-throw-away code.
 */


// Define vectors we need in the example
#include <fastarduino/boards/board.h>
#include <fastarduino/interrupts.h>
#include <fastarduino/utilities.h>

// Utility class to start/end synchronization or do nothing at all
template<bool NOTHING = false> class DisableInterrupts
{
public:
	DisableInterrupts() : sreg_{SREG}
	{
		cli();
	}
	~DisableInterrupts()
	{
		SREG = sreg_;
	}

private:
	const uint8_t sreg_;
};
template<> class DisableInterrupts<true>
{
public:
	DisableInterrupts() = default;
};

volatile int x;

void f()
{
	synchronized x = x * 2 + 1;
}

void g()
{
	UNUSED auto auto_cli = DisableInterrupts<false>{};
	x = x * 4 + 3;
}

void h()
{
	UNUSED auto auto_cli = DisableInterrupts<true>{};
	x = x * 8 + 7;
}

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	x = 10;
	NOP();
	f();
	NOP();
	g();
	NOP();
	h();
	NOP();
}
