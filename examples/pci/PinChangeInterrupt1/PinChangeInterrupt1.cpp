/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/PCI.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D14;
constexpr const Board::InterruptPin PCI_SWITCH = Board::InterruptPin::PCI14;
constexpr const Board::PCIPort PCI_PORT = Board::PCIPort::PCI1;
// Define vectors we need in the example
USE_PCI1()

#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D53;
constexpr const Board::InterruptPin PCI_SWITCH = Board::InterruptPin::PCI0;
constexpr const Board::PCIPort PCI_PORT = Board::PCIPort::PCI0;
// Define vectors we need in the example
USE_PCI0()

//#elif defined (BREADBOARD_ATTINYX4)

#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler: public PCIHandler
{
public:
	PinChangeHandler()
	:	_switch{SWITCH, PinMode::INPUT_PULLUP},
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
	PCI<PCI_PORT> pci{&handler};
	
	pci.enable_pin(PCI_SWITCH);
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
