#include <fastarduino/gpio.h>
#include <fastarduino/power.h>
#include <fastarduino/watchdog.h>

// Define vectors we need in the example
REGISTER_WATCHDOG_ISR_EMPTY()

int main() __attribute__((OS_main));
int main()
{
	board::init();
	sei();

	gpio::FAST_PIN<board::DigitalPin::LED> led{gpio::PinMode::OUTPUT};

	watchdog::WatchdogSignal watchdog;
	watchdog.begin(watchdog::TimeOut::TO_500ms);
	
	while (true)
	{
		led.toggle();
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
