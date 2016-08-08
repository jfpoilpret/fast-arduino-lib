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

class PinChangeHandler
{
public:
	PinChangeHandler():_switches{Board::PORT_C, 0x00, 0xFF}, _leds{Board::PORT_D, 0xFF} {}
	
	void operator() ()
	{
		uint8_t switches = _switches.get_PIN();
		uint8_t leds = (_leds.get_PIN() & 0x80) ^ 0x80;
		if (!(switches & 0x01)) leds |= 0x02;
		if (!(switches & 0x04)) leds |= 0x08;
		if (!(switches & 0x08)) leds |= 0x20;
		_leds.set_PORT(leds);
	}
	
private:
	IOPort _switches;
	IOPort _leds;	
};

int main()
{
	// Enable interrupts at startup time
	sei();

	FunctorPCIHandler<PinChangeHandler> handler{PinChangeHandler{}};
	PCI<Board::PCIPort::PCI1> pci{handler};
	
	pci.enable(0x01 | 0x04 | 0x08);
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
