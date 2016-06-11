/*
 * This program is just here to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <fastarduino/FastPin.hh>

static FastPort<_SFR_IO_ADDR(PINB)> PortB;

static FastPin<Board::D0> PinD0{PinMode::INPUT};
static FastPin<Board::D1> PinD1{PinMode::INPUT_PULLUP};
static FastPin<Board::D2> PinD2{PinMode::OUTPUT};

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
