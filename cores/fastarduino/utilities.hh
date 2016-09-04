#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Board.hh"

//FIXME replace with __attribute__((unused))
#ifndef UNUSED
#define UNUSED(arg) ((void)(arg))
#endif

#ifndef INLINE
#define INLINE __attribute__((always_inline))
#endif

inline uint8_t _lock() INLINE;
inline uint8_t _lock()
{
	uint8_t key = SREG;
	asm volatile("cli" ::: "memory");
	return key;
}

inline void _unlock(uint8_t* key) INLINE;
inline void _unlock(uint8_t* key)
{
  SREG = *key;
  asm volatile("" ::: "memory");
}

#define synchronized for (uint8_t __key __attribute__((__cleanup__(_unlock))) = _lock(), i = 1; i != 0; i--)

//TODO Add optimized versions for IOREG registers: set_ioreg_mask, clear_ioreg_mask
inline void set_mask(REGISTER REG, uint8_t MASK) INLINE;
inline void set_mask(REGISTER REG, uint8_t MASK)
{
	((volatile uint8_t&) REG) |= MASK;
}

inline void clear_mask(REGISTER REG, uint8_t MASK) INLINE;
inline void clear_mask(REGISTER REG, uint8_t MASK)
{
	((volatile uint8_t&) REG) &= ~MASK;
}

inline void set_ioreg_bit(REGISTER IOREG, uint8_t BIT) INLINE;
inline void set_ioreg_bit(REGISTER IOREG, uint8_t BIT)
{
	asm volatile("SBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG.io_addr()), [BIT] "I" (BIT));
}

inline void clear_ioreg_bit(REGISTER IOREG, uint8_t BIT) INLINE;
inline void clear_ioreg_bit(REGISTER IOREG, uint8_t BIT)
{
	asm volatile("CBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG.io_addr()), [BIT] "I" (BIT));
}

inline bool ioreg_bit_value(REGISTER IOREG, uint8_t BIT) INLINE;
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

inline void set_ioreg_byte(REGISTER IOREG, uint8_t value) INLINE;
inline void set_ioreg_byte(REGISTER IOREG, uint8_t value)
{
	asm volatile("OUT %[IOREG], %[VALUE]\n\t"::[IOREG] "I" (IOREG.io_addr()), [VALUE] "r" (value));
}

inline uint8_t get_ioreg_byte(REGISTER IOREG) INLINE;
inline uint8_t get_ioreg_byte(REGISTER IOREG)
{
	uint8_t value = 0;
	asm volatile("IN %[VALUE], %[IOREG]\n\t":[VALUE] "+r" (value):[IOREG] "I" (IOREG.io_addr()));
	return value;
}

#endif	/* UTILITIES_HH */
