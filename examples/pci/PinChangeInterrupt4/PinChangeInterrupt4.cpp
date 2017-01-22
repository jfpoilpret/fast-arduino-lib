/*
 * Pin Change Interrupt example. Take #1
 * This program shows usage of Pin Change Interrupt (PCI) FastArduino support to light a LED when a button is pushed.
 * This sample uses a handler called by PCINT vector.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D14 (PCINT8, PC0, ADC0) branch a push button connected to ground
 *   - D13 (PB5) LED connected to ground through a resistor
 */

#include <avr/interrupt.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/PCI.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH_ON = Board::DigitalPin::D14;
constexpr const Board::DigitalPin SWITCH_OFF = Board::DigitalPin::D8;
#else
#error "Current target is not yet supported!"
#endif

class SwitchHandler
{
public:
	SwitchHandler()
	:	_switch_on{PinMode::INPUT_PULLUP},
		_switch_off{PinMode::INPUT_PULLUP},
		_led{PinMode::OUTPUT}
	{}
	
	void on_switch_on_change()
	{
		if (!_switch_on.value())
			_led.set();
	}
	
	void on_switch_off_change()
	{
		if (!_switch_off.value())
			_led.clear();
	}
	
private:
	FastPinType<SWITCH_ON>::TYPE _switch_on;
	FastPinType<SWITCH_OFF>::TYPE _switch_off;
	FastPinType<Board::DigitalPin::LED>::TYPE _led;	
};

// Define vectors we need in the example
#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
REGISTER_PCI_ISR_METHOD(1, SwitchHandler, &SwitchHandler::on_switch_on_change)
REGISTER_PCI_ISR_METHOD(0, SwitchHandler, &SwitchHandler::on_switch_off_change)
#endif

int main()
{
	// Enable interrupts at startup time
	sei();
	
	SwitchHandler switch_handler;
	register_handler(switch_handler);
	PCIType<SWITCH_ON>::TYPE pci_on;
	PCIType<SWITCH_OFF>::TYPE pci_off;
	
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
