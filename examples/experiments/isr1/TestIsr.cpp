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

static const uint8_t SIZE = 64;
static char buffer[SIZE];

using QUEUE = containers::Queue<char, char>;
static QUEUE queue{buffer};

static void callback()
{
	queue.push_('z');
}

// Use an ISR to check the size of push_() with registers save/restore
REGISTER_INT_ISR_FUNCTION(INT_NUM, INT_PIN, callback)

// int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();

	// Use delay_ms() as marker in generated code, in order to properly isolate code sections
	time::delay_ms(1000);

	while (true)
	{
		queue.push('a');
		time::delay_us(1000);

		char c;
		queue.peek(c);
		time::delay_us(1000);

		queue.pull(c);
		time::delay_us(1000);

		if (queue.items())
			time::delay_us(10000);
		if (queue.free())
			time::delay_us(10000);
		if (queue.empty())
			time::delay_us(10000);
		queue.clear();
		time::delay_us(1000);
	}
}
