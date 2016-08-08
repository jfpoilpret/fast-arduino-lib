/*
 * Pin Change Interrupt test sample.
 */

#include <avr/interrupt.h>
#include <util/delay.h>

#include <fastarduino/IO.hh>
#include <fastarduino/PCI.hh>

// Define vectors we need in the example
USE_PCI1()

class PinChangeHandler
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
	
	void operator() ()
	{
		for (uint8_t i = 0; i < 3; ++i)
		{
			if (_switches[i].value())
				_leds[i].clear();
			else
				_leds[i].set();
		}
		_leds[3].toggle();
	}
	
private:
	IOPin _switches[3];
	IOPin _leds[4];	
};

int main()
{
	// Enable interrupts at startup time
	sei();

	FunctorPCIHandler<PinChangeHandler> handler{PinChangeHandler{}};
	PCI<Board::PCIPort::PCI1> pci{handler};
	
	pci.enable(Board::InterruptPin::PCI14);
	pci.enable(Board::InterruptPin::PCI16);
	pci.enable(Board::InterruptPin::PCI17);
	pci.enable();

	// Event Loop
	while (true)
	{
		_delay_ms(10000.0);
		
	}
}
