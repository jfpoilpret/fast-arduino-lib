/*
 * Pin Change Interrupt test sample.
 * Takes PCI input from 3 switches and lights one of 3 LEDs (1 LED per button).
 * Also toggles a 4th LED on each PCI interrupt.
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/PCI.hh>
#include <fastarduino/power.hh>

// Define vectors we need in the example
USE_PCI1()

//TODO example 3 with IOPort instead of IOPin (more size efficient?))
class PinChangeHandler: public PCIHandler
{
public:
	PinChangeHandler()
	:_switches
	{
		IOPin{Board::DigitalPin::D14, PinMode::INPUT_PULLUP}, 
		IOPin{Board::DigitalPin::D16, PinMode::INPUT_PULLUP}, 
		IOPin{Board::DigitalPin::D17, PinMode::INPUT_PULLUP}
	},
	_leds
	{
		IOPin{Board::DigitalPin::D1, PinMode::OUTPUT}, 
		IOPin{Board::DigitalPin::D3, PinMode::OUTPUT}, 
		IOPin{Board::DigitalPin::D5, PinMode::OUTPUT}, 
		IOPin{Board::DigitalPin::D7, PinMode::OUTPUT}
	}
	{
	}
	
	virtual bool pin_change()
	{
		for (uint8_t i = 0; i < 3; ++i)
		{
			if (_switches[i].value())
				_leds[i].clear();
			else
				_leds[i].set();
		}
		_leds[3].toggle();
		return true;
	}
	
private:
	IOPin _switches[3];
	IOPin _leds[4];	
};

int main()
{
	// Enable interrupts at startup time
	sei();

	PinChangeHandler handler;
	PCI<Board::PCIPort::PCI1> pci{&handler};
	
	pci.enable_pin(Board::InterruptPin::PCI14);
	pci.enable_pin(Board::InterruptPin::PCI16);
	pci.enable_pin(Board::InterruptPin::PCI17);
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
