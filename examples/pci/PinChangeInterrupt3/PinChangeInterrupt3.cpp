/*
 * Pin Change Interrupt test sample.
 * Takes PCI input from 3 switches and lights one of 3 LEDs (1 LED per button).
 * Also toggles a 4th LED on each PCI interrupt.
 * This is similar to PinChangeInterrupt2 except it used IOPort instead of IOPin,
 * which is more size-efficient.
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/PCI.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const uint8_t LED1 = _BV(Board::BIT(Board::DigitalPin::D1));
static constexpr const uint8_t LED2 = _BV(Board::BIT(Board::DigitalPin::D3));
static constexpr const uint8_t LED3 = _BV(Board::BIT(Board::DigitalPin::D5));
static constexpr const uint8_t LED4 = _BV(Board::BIT(Board::DigitalPin::D7));
static constexpr const REGISTER LED_PORT = Board::PORT_D;
static constexpr const uint8_t SW1 = _BV(Board::BIT(Board::DigitalPin::D14));
static constexpr const uint8_t SW2 = _BV(Board::BIT(Board::DigitalPin::D16));
static constexpr const uint8_t SW3 = _BV(Board::BIT(Board::DigitalPin::D17));
static constexpr const REGISTER SWITCH_PORT = Board::PORT_C;
static constexpr const Board::PCIPort PCI_PORT = Board::PCIPort::PCI1;
// Define vectors we need in the example
USE_PCI1()

#elif defined (ARDUINO_MEGA)
static constexpr const uint8_t LED1 = _BV(Board::BIT(Board::DigitalPin::D22));
static constexpr const uint8_t LED2 = _BV(Board::BIT(Board::DigitalPin::D23));
static constexpr const uint8_t LED3 = _BV(Board::BIT(Board::DigitalPin::D24));
static constexpr const uint8_t LED4 = _BV(Board::BIT(Board::DigitalPin::D25));
static constexpr const REGISTER LED_PORT = Board::PORT_A;
static constexpr const uint8_t SW1 = _BV(Board::BIT(Board::DigitalPin::D53));
static constexpr const uint8_t SW2 = _BV(Board::BIT(Board::DigitalPin::D52));
static constexpr const uint8_t SW3 = _BV(Board::BIT(Board::DigitalPin::D51));
static constexpr const REGISTER SWITCH_PORT = Board::PORT_B;
static constexpr const Board::PCIPort PCI_PORT = Board::PCIPort::PCI0;
// Define vectors we need in the example
USE_PCI0()

#elif defined (BREADBOARD_ATTINYX4)
static constexpr const uint8_t LED1 = _BV(Board::BIT(Board::DigitalPin::D0));
static constexpr const uint8_t LED2 = _BV(Board::BIT(Board::DigitalPin::D1));
static constexpr const uint8_t LED3 = _BV(Board::BIT(Board::DigitalPin::D2));
static constexpr const uint8_t LED4 = _BV(Board::BIT(Board::DigitalPin::D3));
static constexpr const REGISTER LED_PORT = Board::PORT_A;
static constexpr const uint8_t SW1 = _BV(Board::BIT(Board::DigitalPin::D8));
static constexpr const uint8_t SW2 = _BV(Board::BIT(Board::DigitalPin::D9));
static constexpr const uint8_t SW3 = _BV(Board::BIT(Board::DigitalPin::D10));
static constexpr const REGISTER SWITCH_PORT = Board::PORT_B;
static constexpr const Board::PCIPort PCI_PORT = Board::PCIPort::PCI1;
// Define vectors we need in the example
USE_PCI1()

#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler: public ExternalInterruptHandler
{
public:
	PinChangeHandler():_switches{SWITCH_PORT, 0x00, 0xFF}, _leds{LED_PORT, 0xFF} {}
	
	virtual bool on_pin_change() override
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
	PCI<PCI_PORT> pci{&handler};
	
	pci.enable_pins(SW1 | SW2 | SW3);
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
