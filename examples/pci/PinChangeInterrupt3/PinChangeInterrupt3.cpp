//   Copyright 2016-2017 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

/*
 * Pin Change Interrupt example. Multiple PCI.
 * This program shows usage of Pin Change Interrupt (PCI) FastArduino support to light LEDs when buttons are pushed.
 * This sample uses a handler called by PCINT vector.
 * Concretely, the example takes PCI input from 3 switches and lights one of 3 LEDs (1 LED per button).
 * It also toggles a 4th LED on each PCI interrupt.
 * 
 * This is similar to PinChangeInterrupt2 except it used IOPort instead of IOPin,
 * which is more size-efficient.
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

#include <fastarduino/fast_io.h>
#include <fastarduino/pci.h>
#include <fastarduino/power.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P)
static constexpr const uint8_t LED1 = _BV(board::BIT<board::DigitalPin::D1_PD1>());
static constexpr const uint8_t LED2 = _BV(board::BIT<board::DigitalPin::D3_PD3>());
static constexpr const uint8_t LED3 = _BV(board::BIT<board::DigitalPin::D5_PD5>());
static constexpr const uint8_t LED4 = _BV(board::BIT<board::DigitalPin::D7_PD7>());
static constexpr const board::Port LED_PORT = board::Port::PORT_D;
static constexpr const board::DigitalPin SWITCH1 = board::InterruptPin::D14_PC0_PCI1;
static constexpr const board::DigitalPin SWITCH2 = board::InterruptPin::D16_PC2_PCI1;
static constexpr const board::DigitalPin SWITCH3 = board::InterruptPin::D17_PC3_PCI1;
static constexpr const uint8_t SW1 = _BV(board::BIT<SWITCH1>());
static constexpr const uint8_t SW2 = _BV(board::BIT<SWITCH2>());
static constexpr const uint8_t SW3 = _BV(board::BIT<SWITCH3>());
static constexpr const board::Port SWITCH_PORT = board::Port::PORT_C;
#define PCI_NUM 1
#elif defined (ARDUINO_MEGA)
static constexpr const uint8_t LED1 = _BV(board::BIT<board::DigitalPin::D22_PA0>());
static constexpr const uint8_t LED2 = _BV(board::BIT<board::DigitalPin::D23_PA1>());
static constexpr const uint8_t LED3 = _BV(board::BIT<board::DigitalPin::D24_PA2>());
static constexpr const uint8_t LED4 = _BV(board::BIT<board::DigitalPin::D25_PA3>());
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
static constexpr const board::DigitalPin SWITCH1 = board::InterruptPin::D53_PB0_PCI0;
static constexpr const board::DigitalPin SWITCH2 = board::InterruptPin::D52_PB1_PCI0;
static constexpr const board::DigitalPin SWITCH3 = board::InterruptPin::D51_PB2_PCI0;
static constexpr const uint8_t SW1 = _BV(board::BIT<SWITCH1>());
static constexpr const uint8_t SW2 = _BV(board::BIT<SWITCH2>());
static constexpr const uint8_t SW3 = _BV(board::BIT<SWITCH3>());
static constexpr const board::Port SWITCH_PORT = board::Port::PORT_B;
#define PCI_NUM 0
#elif defined (BREADBOARD_ATTINYX4)
static constexpr const uint8_t LED1 = _BV(board::BIT<board::DigitalPin::D0_PA0>());
static constexpr const uint8_t LED2 = _BV(board::BIT<board::DigitalPin::D1_PA1>());
static constexpr const uint8_t LED3 = _BV(board::BIT<board::DigitalPin::D2_PA2>());
static constexpr const uint8_t LED4 = _BV(board::BIT<board::DigitalPin::D3_PA3>());
static constexpr const board::Port LED_PORT = board::Port::PORT_A;
static constexpr const board::DigitalPin SWITCH1 = board::InterruptPin::D8_PB0_PCI1;
static constexpr const board::DigitalPin SWITCH2 = board::InterruptPin::D9_PB1_PCI1;
static constexpr const board::DigitalPin SWITCH3 = board::InterruptPin::D10_PB2_PCI1;
static constexpr const uint8_t SW1 = _BV(board::BIT<SWITCH1>());
static constexpr const uint8_t SW2 = _BV(board::BIT<SWITCH2>());
static constexpr const uint8_t SW3 = _BV(board::BIT<SWITCH3>());
static constexpr const board::Port SWITCH_PORT = board::Port::PORT_B;
#define PCI_NUM 1
#else
#error "Current target is not yet supported!"
#endif

class PinChangeHandler
{
public:
	PinChangeHandler():_switches{0x00, 0xFF}, _leds{0xFF} {}
	
	void on_pin_change()
	{
		uint8_t switches = _switches.get_PIN();
		uint8_t leds = (_leds.get_PIN() & LED4) ^ LED4;
		if (!(switches & SW1)) leds |= LED1;
		if (!(switches & SW2)) leds |= LED2;
		if (!(switches & SW3)) leds |= LED3;
		_leds.set_PORT(leds);
	}
	
private:
	gpio::FastPort<SWITCH_PORT> _switches;
	gpio::FastPort<LED_PORT> _leds;	
};

// Define vectors we need in the example
REGISTER_PCI_ISR_METHOD(PCI_NUM, PinChangeHandler, &PinChangeHandler::on_pin_change, SWITCH1)

int main() __attribute__((OS_main));
int main()
{
	// Enable interrupts at startup time
	sei();

	PinChangeHandler handler;
	interrupt::register_handler(handler);
	interrupt::PCIType<SWITCH1>::TYPE pci;
	
	pci.enable_pins(SW1 | SW2 | SW3);
	pci.enable();

	// Event Loop
	while (true)
	{
		power::Power::sleep(board::SleepMode::POWER_DOWN);
	}
}
