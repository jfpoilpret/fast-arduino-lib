#ifndef SIPO_HH
#define SIPO_HH

#include <fastarduino/FastIO.hh>

template<Board::DigitalPin CLOCK, Board::DigitalPin LATCH, Board::DigitalPin DATA, typename T = uint8_t>
class SIPO
{
public:
	SIPO():_clock{PinMode::OUTPUT, false}, _latch{PinMode::OUTPUT, true}, _data{PinMode::OUTPUT, false} {}
	void output(T data)
	{
		T mask = 1 << (sizeof(T) * 8 - 1);
		_latch.clear();
		do
		{
			if (data & mask) _data.set(); else _data.clear();
			_clock.set();
			mask >>= 1;
			_clock.clear();
		}
		while (mask);
		_latch.set();
	}
	
private:
	typename FastPinType<CLOCK>::TYPE _clock;
	typename FastPinType<LATCH>::TYPE _latch;
	typename FastPinType<DATA>::TYPE _data;
};

#endif /* SIPO_HH */

