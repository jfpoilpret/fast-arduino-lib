#ifndef SIPO_HH
#define SIPO_HH

#include <fastarduino/FastIO.hh>

//TODO Improve template arguments: T should not be here but directly for output() method
//TODO We can then specialize output for uint8_t, uint16_t...
template<Board::DigitalPin CLOCK, Board::DigitalPin LATCH, Board::DigitalPin DATA>
class SIPO
{
public:
	static constexpr const Board::Port PORT = FastPinType<CLOCK>::PORT;
	static constexpr const uint8_t DDR_MASK =
		FastPinType<CLOCK>::MASK | FastPinType<LATCH>::MASK | FastPinType<DATA>::MASK;
	static constexpr const uint8_t PORT_MASK = FastPinType<LATCH>::MASK;
	
	SIPO():_clock{}, _latch{}, _data{}
	{
		static_assert(	PORT == FastPinType<LATCH>::PORT && PORT == FastPinType<DATA>::PORT,
						"CLOCK, LATCH and DATA pins must belong to the same PORT");
	}
	
	inline void init()
	{
		_clock.set_mode(PinMode::OUTPUT, false);
		_latch.set_mode(PinMode::OUTPUT, true);
		_data.set_mode(PinMode::OUTPUT, false);
	}
	
	template<typename T>
	void output(T data)
	{
		T mask = (T(1)) << (sizeof(T) * 8 - 1);
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

