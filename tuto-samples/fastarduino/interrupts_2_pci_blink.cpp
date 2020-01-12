#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

constexpr const board::InterruptPin SWITCH = board::InterruptPin::D14_PC0_PCI1;
#define PCI_NUM 1

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
	gpio::FAST_INT_PIN<SWITCH> _switch;
	gpio::FAST_PIN<board::DigitalPin::LED> _led;	
};

// Define vectors we need in the example
REGISTER_PCI_ISR_METHOD(PCI_NUM, PinChangeHandler, &PinChangeHandler::on_pin_change, SWITCH)

int main() __attribute__((OS_main));
int main()
{
	board::init();
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::PCI_SIGNAL<SWITCH> pci;
	
	pci.enable_pin<SWITCH>();
	pci.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
