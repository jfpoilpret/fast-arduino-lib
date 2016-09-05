/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/PCI.hh>
#include <fastarduino/power.hh>

// Define vectors we need in the example
USE_PCI1()

class PinChangeHandler: public PCIHandler
{
public:
	PinChangeHandler()
	:	_switch{Board::DigitalPin::D14, PinMode::INPUT_PULLUP},
		_led{Board::DigitalPin::LED, PinMode::OUTPUT}
	{}
	
	virtual bool pin_change()
	{
		if (_switch.value())
			_led.clear();
		else
			_led.set();
		return true;
	}
	
private:
	IOPin _switch;
	IOPin _led;	
};

int main()
{
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	PCI<Board::PCIPort::PCI1> pci{&handler};
	
	pci.enable_pin(Board::InterruptPin::PCI14);
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
