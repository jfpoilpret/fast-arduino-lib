/*
 * Pin Change Interrupt test sample.
 * Uses PCI driven switch input to light Arduino LED (D13)
 */

#include <avr/interrupt.h>

#include <fastarduino/IO.hh>
#include <fastarduino/INT.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH = Board::DigitalPin::D2;
constexpr const Board::ExternalInterruptPin INT_SWITCH = Board::ExternalInterruptPin::EXT0;
// Define vectors we need in the example
USE_INT0()

#elif defined (ARDUINO_MEGA)

#elif defined (BREADBOARD_ATTINYX4)

#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler: public INTHandler
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
	INT<INT_SWITCH> int0{InterruptTrigger::ANY_CHANGE, &handler};
	int0.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
