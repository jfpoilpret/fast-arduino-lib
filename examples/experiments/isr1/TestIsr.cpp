/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <stdint.h>
#include <avr/io.h>

template<typename T>
class REGISTER
{
public:
	constexpr REGISTER():ADDR(0) {}
	constexpr REGISTER(const REGISTER<T>& rhs):ADDR(rhs.ADDR) {}
	constexpr REGISTER(uint16_t ADDR):ADDR(ADDR) {}
	
	void operator =(T value) const
	{
		*((volatile T*) ADDR) = value;
	}
	void operator |=(T value) const
	{
		*((volatile T*) ADDR) |= value;
	}
	void operator &=(T value) const
	{
		*((volatile T*) ADDR) &= value;
	}
	void operator ^=(T value) const
	{
		*((volatile T*) ADDR) ^= value;
	}
	uint8_t operator ~() const
	{
		return ~(*((volatile T*) ADDR));
	}
	operator T() const
	{
		return *((volatile T*) ADDR);
	}

private:	
	uint16_t ADDR;
};

using REG8 = REGISTER<uint8_t>;
using REG16 = REGISTER<uint16_t>;

constexpr const REG8 REG_EMPTY{};
constexpr const REG8 REG_PORTB{(uint16_t)&PORTB};
constexpr const REG8 REG_PORTB2 = REG_PORTB;

constexpr const REG16 REG16_EMPTY{};
constexpr const REG16 REG16_TCNT{(uint16_t)&TCNT1};
constexpr const REG16 REG16_TCNT2 = REG16_TCNT;

int main() __attribute__((OS_main));
int main()
{
	REG_PORTB = 0xFF;				// ldi + out
	REG_PORTB &= 0x0F;				// in + andi + out
	REG_PORTB |= 0x80;				// sbi
	REG_PORTB &= ~0x08;				// cbi
	REG_PORTB ^= 0xFF;				// in + com
	REG_PORTB ^= 0x23;				// in + ldi + eor + pout
	uint8_t value8 = REG_PORTB;		// in
	while (value8 != 123)			// cpi + breq
	{
		value8 = ~REG_PORTB;			// in + com

		value8 = REG_PORTB | value8;	// in + or
		value8 = REG_PORTB & value8;	// in + and
	}
	
	REG16_TCNT = 0xFFFF;
	REG16_TCNT &= 0x0F0F;
	REG16_TCNT |= 0x8000;
	REG16_TCNT &= ~0x0800;
	REG16_TCNT ^= 0xFFFF;
	REG16_TCNT ^= 0x2323;
	uint16_t value16 = REG16_TCNT;
	while (value16 != 15000)
	{
		value16 = ~REG16_TCNT;
		
		value16 = REG16_TCNT | value16;
		value16 = REG16_TCNT & value16;
	}
}
