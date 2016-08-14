#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Board.hh"

#ifndef UNUSED
#define UNUSED(arg) ((void)(arg))
#endif

class ClearInterrupt
{
public:
	ClearInterrupt() __attribute__((always_inline)) : _sreg(SREG)
	{
		cli();
	}
	~ClearInterrupt() __attribute__((always_inline))
	{
		SREG = _sreg;
	}

private:
	const uint8_t _sreg;
};

//inline void set_bit(volatile uint8_t* MEM, uint8_t BIT)
//{
//	asm volatile("SBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG), [BIT] "I" (BIT));
//}

inline void set_ioreg_bit(REGISTER IOREG, uint8_t BIT)
{
	asm volatile("SBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG.io_addr()), [BIT] "I" (BIT));
}

inline void clear_ioreg_bit(REGISTER IOREG, uint8_t BIT)
{
	asm volatile("CBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG.io_addr()), [BIT] "I" (BIT));
}

inline bool ioreg_bit_value(REGISTER IOREG, uint8_t BIT)
{
	bool result = false;
	asm volatile(
		"SBIC %[IOREG], %[BIT] \n\t"
		"LDI %[RESULT], 1\n\t"			// Bit is set, set result value accordingly
		:[RESULT] "+r" (result)
		:[IOREG] "I" (IOREG.io_addr()), [BIT] "I" (BIT)
	);
	return result;
}

inline void set_ioreg_byte(REGISTER IOREG, uint8_t value)
{
	asm volatile("OUT %[IOREG], %[VALUE]\n\t"::[IOREG] "I" (IOREG.io_addr()), [VALUE] "r" (value));
}

inline uint8_t get_ioreg_byte(REGISTER IOREG)
{
	uint8_t value = 0;
	asm volatile("IN %[VALUE], %[IOREG]\n\t":[VALUE] "+r" (value):[IOREG] "I" (IOREG.io_addr()));
	return value;
}

#endif	/* UTILITIES_HH */
