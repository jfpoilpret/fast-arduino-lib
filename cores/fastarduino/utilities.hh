#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Board.hh"

#ifndef UNUSED
#define UNUSED(arg) ((void)(arg))
#endif

//class ClearInterrupt
//{
//public:
//	ClearInterrupt() __attribute__((always_inline)) : _sreg(SREG)
//	{
//		cli();
//	}
//	~ClearInterrupt() __attribute__((always_inline))
//	{
//		SREG = _sreg;
//	}
//
//private:
//	const uint8_t _sreg;
//};

inline uint8_t _lock() __attribute__((always_inline));
inline uint8_t _lock()
{
	uint8_t key = SREG;
	asm volatile("cli" ::: "memory");
	return key;
}

inline void _unlock(uint8_t* key) __attribute__((always_inline));
inline void _unlock(uint8_t* key)
{
  SREG = *key;
  asm volatile("" ::: "memory");
}

#define synchronized for (uint8_t __key __attribute__((__cleanup__(_unlock))) = _lock(), i = 1; i != 0; i--)

//TODO Check if templates are absolutely needed here (as this reduces the utilization scope of those methods)
template<const REGISTER& REG, uint8_t MASK>
inline void set_mask()
{
	((volatile uint8_t&) REG) |= MASK;
}

template<REGISTER& REG, uint8_t MASK>
inline void clear_mask()
{
	((volatile uint8_t&) REG) &= ~MASK;
}

template<const REGISTER& REG>
inline void set_mask(uint8_t mask)
{
	((volatile uint8_t&) REG) |= mask;
}

template<const REGISTER& REG>
inline void clear_mask(uint8_t mask)
{
	((volatile uint8_t&) REG) &= ~mask;
}

inline void set_ioreg_bit(REGISTER IOREG, uint8_t BIT) __attribute__((always_inline));
inline void set_ioreg_bit(REGISTER IOREG, uint8_t BIT)
{
	asm volatile("SBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG.io_addr()), [BIT] "I" (BIT));
}

inline void clear_ioreg_bit(REGISTER IOREG, uint8_t BIT) __attribute__((always_inline));
inline void clear_ioreg_bit(REGISTER IOREG, uint8_t BIT)
{
	asm volatile("CBI %[IOREG], %[BIT] \n\t"::[IOREG] "I" (IOREG.io_addr()), [BIT] "I" (BIT));
}

inline bool ioreg_bit_value(REGISTER IOREG, uint8_t BIT) __attribute__((always_inline));
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

inline void set_ioreg_byte(REGISTER IOREG, uint8_t value) __attribute__((always_inline));
inline void set_ioreg_byte(REGISTER IOREG, uint8_t value)
{
	asm volatile("OUT %[IOREG], %[VALUE]\n\t"::[IOREG] "I" (IOREG.io_addr()), [VALUE] "r" (value));
}

inline uint8_t get_ioreg_byte(REGISTER IOREG) __attribute__((always_inline));
inline uint8_t get_ioreg_byte(REGISTER IOREG)
{
	uint8_t value = 0;
	asm volatile("IN %[VALUE], %[IOREG]\n\t":[VALUE] "+r" (value):[IOREG] "I" (IOREG.io_addr()));
	return value;
}

#endif	/* UTILITIES_HH */
