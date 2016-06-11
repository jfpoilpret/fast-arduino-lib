#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/sfr_defs.h>

template<uint8_t IOREG, uint8_t BIT>
inline void set_ioreg_bit()
{
	asm("SBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG), [BIT] "I" (BIT));
}

template<uint8_t IOREG, uint8_t BIT>
inline void clear_ioreg_bit()
{
	asm("CBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG), [BIT] "I" (BIT));
}

template<uint8_t IOREG, uint8_t BIT>
inline bool ioreg_bit_value()
{
	bool result = false;
	asm(
		"SBIC %[IOREG], %[BIT] \n\t"
		"LDI %[RESULT], 1\n\t"			// Bit is set, set result value accordingly
		:[RESULT] "+r" (result)
		:[IOREG] "I" (IOREG), [BIT] "I" (BIT)
	);
	return result;
}

template<uint8_t IOREG>
inline void set_ioreg_byte(uint8_t value)
{
	asm("OUT %[IOREG], %[VALUE]\n\t"::[IOREG] "I" (IOREG), [VALUE] "r" (value));
}

template<uint8_t IOREG>
inline uint8_t get_ioreg_byte()
{
	uint8_t value = 0;
	asm("IN %[VALUE], %[IOREG]\n\t":[VALUE] "+r" (value):[IOREG] "I" (IOREG));
	return value;
}

#endif	/* UTILITIES_HH */

