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
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D14;
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D53;
#elif defined (BREADBOARD_ATTINYX4)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D8;
#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler
{
public:
	PinChangeHandler()
	:	_switch{PinMode::INPUT_PULLUP},
		_led{PinMode::OUTPUT}
	{}
	
	void on_pin_change()
	{
		if (_switch.value())
			_led.clear();
		else
			_led.set();
	}
	
private:
	FastPinType<SWITCH>::TYPE _switch;
	FastPinType<Board::DigitalPin::LED>::TYPE _led;	
};

// Define vectors we need in the example
#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
REGISTER_PCI_ISR_METHOD(1, PinChangeHandler, &PinChangeHandler::on_pin_change)
#elif defined (ARDUINO_MEGA)
REGISTER_PCI_ISR_METHOD(0, PinChangeHandler, &PinChangeHandler::on_pin_change)
#elif defined (BREADBOARD_ATTINYX4)
REGISTER_PCI_ISR_METHOD(1, PinChangeHandler, &PinChangeHandler::on_pin_change)
#endif

int main()
{
	// Enable interrupts at startup time
	sei();
	
	PinChangeHandler handler;
	register_handler(handler);
	PCIType<SWITCH>::TYPE pci;
	
	pci.enable_pin<SWITCH>();
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
