/*
 * Pin External Interrupt example. Take #1
 * This program shows usage of External Interrupt Pin FastArduino support to light a LED when a button is pushed, and
 * switch it off when another button is pushed.
 * This sample uses a handler called by INT vector.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D2 (INT0, PD2) branch a push button connected to ground
 *   - D3 (INT1, PD3) branch a push button connected to ground
 *   - D13 (PB5) LED connected to ground through a resistor
 * - on Arduino MEGA: TODO
 *   - D21 (INT0) branch a push button connected to ground
 *   - D13 (PB7) LED connected to ground through a resistor
 * Uses PCI driven switch input to light Arduino LED (D13, )
 */

#include <avr/interrupt.h>

#include <fastarduino/FastIO.hh>
#include <fastarduino/INT.hh>
#include <fastarduino/power.hh>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
constexpr const Board::DigitalPin SWITCH_ON = Board::ExternalInterruptPin::D2;
constexpr const Board::DigitalPin SWITCH_OFF = Board::ExternalInterruptPin::D3;
// Define vectors we need in the example
USE_INTS(0, 1)
#elif defined (ARDUINO_MEGA)
constexpr const Board::DigitalPin SWITCH_ON = Board::ExternalInterruptPin::D21;
constexpr const Board::DigitalPin SWITCH_OFF = Board::ExternalInterruptPin::D20;
// Define vectors we need in the example
USE_INTS(0, 1)
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
		if (_switch.value())
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
		if (_switch.value())
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
	SwitchOffHandler switchoff_handler;
	INT<SWITCH_ON> int0{InterruptTrigger::ANY_CHANGE, &switchon_handler};
	INT<SWITCH_OFF> int1{InterruptTrigger::ANY_CHANGE, &switchoff_handler};
	int0.enable();
	int1.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
