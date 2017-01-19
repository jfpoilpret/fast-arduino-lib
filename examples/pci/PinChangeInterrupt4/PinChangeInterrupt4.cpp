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

#include <fastarduino/FastIO.hh>
#include <fastarduino/PCI.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH_ON = Board::DigitalPin::D14;
constexpr const Board::DigitalPin SWITCH_OFF = Board::DigitalPin::D8;
// Define vectors we need in the example
USE_PCIS(0, 1)

#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH_ON = Board::DigitalPin::D53;
//constexpr const Board::DigitalPin SWITCH_OFF = Board::DigitalPin::D8;
// Define vectors we need in the example
USE_PCIS(0)

#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin SWITCH_ON = Board::DigitalPin::D8;
//constexpr const Board::DigitalPin SWITCH_OFF = Board::DigitalPin::D8;
// Define vectors we need in the example
USE_PCIS(1)

#else
#error "Current target is not yet supported!"
#endif

class SwitchOnHandler: public ExternalInterruptHandler
{
public:
	SwitchOnHandler()
	:	_switch{PinMode::INPUT_PULLUP},
		_led{PinMode::OUTPUT}
	{}
	
	virtual bool on_pin_change() override
	{
		if (!_switch.value())
			_led.set();
		return true;
	}
	
private:
	FastPinType<SWITCH_ON>::TYPE _switch;
	FastPinType<Board::DigitalPin::LED>::TYPE _led;	
};

class SwitchOffHandler: public ExternalInterruptHandler
{
public:
	SwitchOffHandler()
	:	_switch{PinMode::INPUT_PULLUP},
		_led{PinMode::OUTPUT}
	{}
	
	virtual bool on_pin_change() override
	{
		if (!_switch.value())
			_led.clear();
		return true;
	}
	
private:
	FastPinType<SWITCH_OFF>::TYPE _switch;
	FastPinType<Board::DigitalPin::LED>::TYPE _led;	
};

int main()
{
	// Enable interrupts at startup time
	sei();
	
	SwitchOnHandler switchon_handler;
	PCIType<SWITCH_ON>::TYPE pci_on{&switchon_handler};
	SwitchOffHandler switchoff_handler;
	PCIType<SWITCH_OFF>::TYPE pci_off{&switchoff_handler};
	
	pci_on.enable_pin<SWITCH_ON>();
	pci_off.enable_pin<SWITCH_OFF>();
	pci_on.enable();
	pci_off.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
