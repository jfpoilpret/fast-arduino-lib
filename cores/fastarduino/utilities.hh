#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/sfr_defs.h>

//TODO check if templates are really needed for each asm code

template<uint8_t IOREG, uint8_t BIT>
inline void set_ioreg_bit()
{
	asm volatile("SBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG), [BIT] "I" (BIT));
}

template<uint8_t IOREG, uint8_t BIT>
inline void clear_ioreg_bit()
{
	asm volatile("CBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG), [BIT] "I" (BIT));
}

template<uint8_t IOREG, uint8_t BIT>
inline bool ioreg_bit_value()
{
	bool result = false;
	asm volatile(
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
	asm volatile("OUT %[IOREG], %[VALUE]\n\t"::[IOREG] "I" (IOREG), [VALUE] "r" (value));
}

template<uint8_t IOREG>
inline uint8_t get_ioreg_byte()
{
	uint8_t value = 0;
	asm volatile("IN %[VALUE], %[IOREG]\n\t":[VALUE] "+r" (value):[IOREG] "I" (IOREG));
	return value;
}

#endif	/* UTILITIES_HH */

