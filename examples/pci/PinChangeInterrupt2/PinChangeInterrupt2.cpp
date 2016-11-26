/*
 * Pin Change Interrupt example. Multiple PCI.
 * This program shows usage of Pin Change Interrupt (PCI) FastArduino support to light LEDs when buttons are pushed.
 * This sample uses a handler called by PCINT vector.
 * Concretely, the example takes PCI input from 3 switches and lights one of 3 LEDs (1 LED per button).
 * It also toggles a 4th LED on each PCI interrupt.
 * 
 * Wiring:
 * - on ATmega328P based boards (including Arduino UNO):
 *   - D1, D3, D5, D7 (port D) branch 4 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D14, D16, D17 (port C, ADC0, ADC2, ADC3) branch 3 buttons connected to ground
 * - on Arduino MEGA:
 *   - D22-D25 (port A) branch 4 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D53-D51 (port B) branch 3 buttons connected to ground
 * - on ATtinyX4 based boards:
 *   - D0-D3 (port A) branch 4 LED (in series with 330 Ohm resistors to limit current) connected to ground
 *   - D8-D10 (port B) branch 3 buttons connected to ground
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
// Define vectors we need in the example
USE_PCI1()

#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler: public ExternalInterruptHandler
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
	
	virtual bool on_pin_change() override
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
	PCIType<SWITCH1>::TYPE pci{&handler};
	
	pci.enable_pin<SWITCH1>();
	pci.enable_pin<SWITCH2>();
	pci.enable_pin<SWITCH3>();
	pci.enable();

	// Event Loop
	while (true)
	{
		Power::sleep(Board::SleepMode::POWER_DOWN);
	}
}
