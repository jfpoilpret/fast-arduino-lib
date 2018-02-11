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

using QUEUE = containers::Queue<char, char>;

class Callback
{
public:
	Callback(QUEUE& queue):queue_{queue}
	{
		interrupt::register_handler(*this);
	}

	void callback()
	{
		queue_.push_('z');
	}

private:
	QUEUE& queue_;
};

// Use an ISR to check the size of push_() with registers save/restore
REGISTER_INT_ISR_METHOD(INT_NUM, INT_PIN, Callback, &Callback::callback)

int main() __attribute__((OS_main));
int main()
{
	board::init();

	char buffer[SIZE];
	QUEUE queue{buffer};
	Callback callback{queue};

	// Enable interrupts at startup time
	sei();

	while (true)
	{
		queue.push('a');
		// Use delay_us() as a marker in generated code, in order to properly isolate code sections
		time::delay_us(1000);

		char c;
		queue.peek(c);
		time::delay_us(1000);

		queue.pull(c);
		time::delay_us(1000);

		if (queue.items())
			time::delay_us(1000);
		if (queue.free())
			time::delay_us(1000);
		if (queue.empty())
			time::delay_us(1000);
		queue.clear();
		time::delay_us(1000);
	}
}
