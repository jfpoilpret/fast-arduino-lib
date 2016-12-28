#ifndef SIPO_HH
#define SIPO_HH

#include <fastarduino/FastIO.hh>

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
		uint8_t* pdata = (uint8_t*) data;
		_latch.clear();
		for (uint8_t i = 0 ; i < sizeof(T); ++i)
			_output(pdata[i]);
		_latch.set();
	}
	
	// Specialized output for most common types
	inline void output(uint8_t data) INLINE
	{
		_latch.clear();
		_output(data);
		_latch.set();
	}
	
	inline void output(uint16_t data) INLINE
	{
		_latch.clear();
		_output(data >> 8);
		_output(data);
		_latch.set();
	}
	
private:
	void _output(uint8_t data)
	{
		uint8_t mask = 0x80;
		do
		{
			if (data & mask) _data.set(); else _data.clear();
			_clock.set();
			mask >>= 1;
			_clock.clear();
		}
		while (mask);
	}

	typename FastPinType<CLOCK>::TYPE _clock;
	typename FastPinType<LATCH>::TYPE _latch;
	typename FastPinType<DATA>::TYPE _data;
};

#endif /* SIPO_HH */

