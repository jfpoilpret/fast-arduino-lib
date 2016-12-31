/*
 * This program is just here to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/FastIO.hh>

static FastPort<Board::Port::PORT_B> PortB;

static FastPinType<Board::DigitalPin::D0>::TYPE PinD0{PinMode::INPUT};
static FastPinType<Board::DigitalPin::D1>::TYPE PinD1{PinMode::INPUT_PULLUP};
static FastPinType<Board::DigitalPin::D2>::TYPE PinD2{PinMode::OUTPUT};

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
	f();
	return 0;
}
