/*
 * This program is just for personal experiments here on AVR features and C++ stuff
 * to check compilation and link of FastArduino port and pin API.
 * It does not do anything interesting as far as hardware is concerned.
 */

#include <stdint.h>
#include <avr/io.h>

class REGISTER
{
public:
	constexpr REGISTER():ADDR(0) {}
	constexpr REGISTER(const REGISTER& rhs):ADDR(rhs.ADDR) {}
	constexpr REGISTER(uint16_t ADDR):ADDR(ADDR) {}
	
	void operator =(uint8_t value) const
	{
		*((volatile uint8_t*) ADDR) = value;
	}
	void operator |=(uint8_t value) const
	{
		*((volatile uint8_t*) ADDR) |= value;
	}
	void operator &=(uint8_t value) const
	{
		*((volatile uint8_t*) ADDR) &= value;
	}
	void operator ^=(uint8_t value) const
	{
		*((volatile uint8_t*) ADDR) ^= value;
	}
	uint8_t operator ~() const
	{
		return ~(*((volatile uint8_t*) ADDR));
	}
	operator uint8_t() const
	{
		return *((volatile uint8_t*) ADDR);
	}

private:	
	uint16_t ADDR;
};

constexpr const REGISTER REG_EMPTY{};
constexpr const REGISTER REG_PORTB{(uint16_t)&PORTB};
constexpr const REGISTER REG_PORTB2 = REG_PORTB;

int main() __attribute__((OS_main));
int main()
{
	REG_PORTB = 0xFF;				// ldi + out
	REG_PORTB &= 0x0F;				// in + andi + out
	REG_PORTB |= 0x80;				// sbi
	REG_PORTB &= ~0x08;				// cbi
	REG_PORTB ^= 0xFF;				// in + com
	REG_PORTB ^= 0x23;				// in + ldi + eor + pout
	uint8_t value = REG_PORTB;		// in
	while (value != 123)			// cpi + breq
	{
		value = ~REG_PORTB;			// in + com

		value = REG_PORTB | value;	// in + or
		value = REG_PORTB & value;	// in + and
	}
}
