/*
 * Pin Change Interrupt test sample.
 * Takes PCI input from 3 switches and lights one of 3 LEDs (1 LED per button).
 * Also toggles a 4th LED on each PCI interrupt.
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/PCI.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH1 = Board::DigitalPin::D14;
constexpr const Board::DigitalPin SWITCH2 = Board::DigitalPin::D16;
constexpr const Board::DigitalPin SWITCH3 = Board::DigitalPin::D17;
constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D1;
constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D3;
constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D5;
constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D7;
constexpr const Board::PCIPort PCI_PORT = Board::PCIPort::PCI1;
// Define vectors we need in the example
USE_PCI1()

#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH1 = Board::DigitalPin::D53;
constexpr const Board::DigitalPin SWITCH2 = Board::DigitalPin::D52;
constexpr const Board::DigitalPin SWITCH3 = Board::DigitalPin::D51;
constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D22;
constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D23;
constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D24;
constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D25;
constexpr const Board::PCIPort PCI_PORT = Board::PCIPort::PCI0;
// Define vectors we need in the example
USE_PCI0()

#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin SWITCH1 = Board::DigitalPin::D8;
constexpr const Board::DigitalPin SWITCH2 = Board::DigitalPin::D9;
constexpr const Board::DigitalPin SWITCH3 = Board::DigitalPin::D10;
constexpr const Board::DigitalPin LED1 = Board::DigitalPin::D0;
constexpr const Board::DigitalPin LED2 = Board::DigitalPin::D1;
constexpr const Board::DigitalPin LED3 = Board::DigitalPin::D2;
constexpr const Board::DigitalPin LED4 = Board::DigitalPin::D3;
constexpr const Board::PCIPort PCI_PORT = Board::PCIPort::PCI1;
// Define vectors we need in the example
USE_PCI1()

#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler: public PCIHandler
{
public:
	PinChangeHandler()
	:_switches
	{
		IOPin{SWITCH1, PinMode::INPUT_PULLUP}, 
		IOPin{SWITCH2, PinMode::INPUT_PULLUP}, 
		IOPin{SWITCH3, PinMode::INPUT_PULLUP}
	},
	_leds
	{
		IOPin{LED1, PinMode::OUTPUT}, 
		IOPin{LED2, PinMode::OUTPUT}, 
		IOPin{LED3, PinMode::OUTPUT}, 
		IOPin{LED4, PinMode::OUTPUT}
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
	PCI<PCI_PORT> pci{&handler};
	
	pci.enable_pin((Board::InterruptPin) SWITCH1);
	pci.enable_pin((Board::InterruptPin) SWITCH2);
	pci.enable_pin((Board::InterruptPin) SWITCH3);
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
