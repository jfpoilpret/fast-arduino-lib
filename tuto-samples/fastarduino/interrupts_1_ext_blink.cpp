#include <fastarduino/gpio.h>
#include <fastarduino/int.h>
#include <fastarduino/power.h>

constexpr const board::ExternalInterruptPin SWITCH = board::ExternalInterruptPin::D2_PD2_EXT0;

class PinChangeHandler
{
public:
	PinChangeHandler():_switch{gpio::PinMode::INPUT_PULLUP}, _led{gpio::PinMode::OUTPUT} {}
	
	void on_pin_change()
	{
		if (_switch.value())
			_led.clear();
		else
			_led.set();
	}
	
private:
	gpio::FAST_EXT_PIN<SWITCH> _switch;
	gpio::FAST_PIN<board::DigitalPin::LED> _led;	
};

// Define vectors we need in the example
REGISTER_INT_ISR_METHOD(0, SWITCH, PinChangeHandler, &PinChangeHandler::on_pin_change)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::INTSignal<SWITCH> int0{interrupt::InterruptTrigger::ANY_CHANGE};
	int0.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
