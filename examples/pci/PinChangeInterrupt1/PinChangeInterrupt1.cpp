/*
 * Pin Change Interrupt example. Take #1
 * This program shows usage of Pin Change Interrupt (PCI) FastArduino support to light a LED when a button is pushed.
 * This sample uses a handler called by PCINT vector.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D14 (PCINT8, PC0, ADC0) branch a push button connected to ground
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA:
 *   - D53 (PCINT0, PB0) branch a push button connected to ground
 *   - D13 (PB7) LED connected to ground through a resistor
 * - on ATtinyX4 based boards:
 *   - D8 (PCINT8, PB0) branch a push button connected to ground
 *   - D7 (PA7) LED connected to ground through a resistor
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/PCI.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D14;
// Define vectors we need in the example
USE_PCI1()

#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D53;
// Define vectors we need in the example
USE_PCI0()

#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D8;
// Define vectors we need in the example
USE_PCI1()

#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler: public ExternalInterruptHandler
{
public:
	PinChangeHandler()
	:	_switch{SWITCH, PinMode::INPUT_PULLUP},
		_led{Board::DigitalPin::LED, PinMode::OUTPUT}
	{}
	
	virtual bool on_pin_change() override
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
	PCIType<SWITCH>::TYPE pci{&handler};
	
	pci.enable_pin<SWITCH>();
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
