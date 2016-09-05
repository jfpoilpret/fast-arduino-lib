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

static constexpr const uint8_t LED1 = _BV(Board::BIT(Board::DigitalPin::D1));
static constexpr const uint8_t LED2 = _BV(Board::BIT(Board::DigitalPin::D3));
static constexpr const uint8_t LED3 = _BV(Board::BIT(Board::DigitalPin::D5));
static constexpr const uint8_t LED4 = _BV(Board::BIT(Board::DigitalPin::D7));

static constexpr const uint8_t SW1 = _BV(Board::BIT(Board::DigitalPin::D14));
static constexpr const uint8_t SW2 = _BV(Board::BIT(Board::DigitalPin::D16));
static constexpr const uint8_t SW3 = _BV(Board::BIT(Board::DigitalPin::D17));

class PinChangeHandler: public PCIHandler
{
public:
	PinChangeHandler():_switches{Board::PORT_C, 0x00, 0xFF}, _leds{Board::PORT_D, 0xFF} {}
	
	virtual bool pin_change()
	{
		uint8_t switches = _switches.get_PIN();
		uint8_t leds = (_leds.get_PIN() & LED4) ^ LED4;
		if (!(switches & SW1)) leds |= LED1;
		if (!(switches & SW2)) leds |= LED2;
		if (!(switches & SW3)) leds |= LED3;
		_leds.set_PORT(leds);
		return true;
	}
	
private:
	IOPort _switches;
	IOPort _leds;	
};

int main()
{
	// Enable interrupts at startup time
	sei();

	PinChangeHandler handler;
	PCI<Board::PCIPort::PCI1> pci{&handler};
	
	pci.enable_pins(SW1 | SW2 | SW3);
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
