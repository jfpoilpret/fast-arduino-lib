#include <fastarduino/gpio.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

constexpr const board::DigitalPin SWITCH = board::InterruptPin::D14_PC0_PCI1;
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
	gpio::FastPinType<SWITCH>::TYPE _switch;
	gpio::FastPinType<board::DigitalPin::LED>::TYPE _led;	
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
	interrupt::PCIType<SWITCH>::TYPE pci;
	
	pci.enable_pin<SWITCH>();
	pci.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
