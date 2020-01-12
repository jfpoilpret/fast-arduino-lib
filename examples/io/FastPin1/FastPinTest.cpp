//   Copyright 2016-2019 Jean-Francois Poilpret
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
 * This program is just here to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/gpio.h>

#if defined(ARDUINO_UNO) || defined(BREADBOARD_ATMEGA328P) || defined(ARDUINO_NANO)
static gpio::FastPort<board::Port::PORT_B> PortB;
static gpio::FAST_PIN<board::DigitalPin::D0_PD0> PinD0{gpio::PinMode::INPUT};
static gpio::FAST_PIN<board::DigitalPin::D1_PD1> PinD1{gpio::PinMode::INPUT_PULLUP};
static gpio::FAST_PIN<board::DigitalPin::D2_PD2> PinD2{gpio::PinMode::OUTPUT};
#elif defined(ARDUINO_LEONARDO)
static gpio::FastPort<board::Port::PORT_B> PortB;
static gpio::FAST_PIN<board::DigitalPin::D0_PD2> PinD0{gpio::PinMode::INPUT};
static gpio::FAST_PIN<board::DigitalPin::D1_PD3> PinD1{gpio::PinMode::INPUT_PULLUP};
static gpio::FAST_PIN<board::DigitalPin::D2_PD1> PinD2{gpio::PinMode::OUTPUT};
#elif defined (ARDUINO_MEGA)
static gpio::FastPort<board::Port::PORT_B> PortB;
static gpio::FAST_PIN<board::DigitalPin::D0_PE0> PinD0{gpio::PinMode::INPUT};
static gpio::FAST_PIN<board::DigitalPin::D1_PE1> PinD1{gpio::PinMode::INPUT_PULLUP};
static gpio::FAST_PIN<board::DigitalPin::D2_PE4> PinD2{gpio::PinMode::OUTPUT};
#elif defined (BREADBOARD_ATTINYX4)
static gpio::FastPort<board::Port::PORT_B> PortB;
static gpio::FAST_PIN<board::DigitalPin::D0_PA0> PinD0{gpio::PinMode::INPUT};
static gpio::FAST_PIN<board::DigitalPin::D1_PA1> PinD1{gpio::PinMode::INPUT_PULLUP};
static gpio::FAST_PIN<board::DigitalPin::D2_PA2> PinD2{gpio::PinMode::OUTPUT};
#elif defined (BREADBOARD_ATTINYX5)
static gpio::FastPort<board::Port::PORT_B> PortB;
static gpio::FAST_PIN<board::DigitalPin::D0_PB0> PinD0{gpio::PinMode::INPUT};
static gpio::FAST_PIN<board::DigitalPin::D1_PB1> PinD1{gpio::PinMode::INPUT_PULLUP};
static gpio::FAST_PIN<board::DigitalPin::D2_PB2> PinD2{gpio::PinMode::OUTPUT};
#else
#error "Current target is not yet supported!"
#endif

bool f()
{
	PortB.set_DDR(0xFF);
	PortB.set_PORT(0x0);
	bool d0 = PinD0.value();
	bool d1 = PinD1.value();
	if (d0 && d1)
		PinD2.set();
	else
		PinD2.clear();
	PinD2.toggle();
	return (d0 || d1);
}

int main()
{
	board::init();
	f();
	return 0;
}
