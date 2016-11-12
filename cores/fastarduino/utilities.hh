#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/io.h>
#include <avr/interrupt.h>

//#include "Board.hh"

#ifndef UNUSED
#define UNUSED __attribute__((unused))
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

#define synchronized \
_Pragma ("GCC diagnostic ignored \"-Wreturn-type\"") \
for (uint8_t __key __attribute__((__cleanup__(_unlock))) = _lock(), i = 1; i != 0; i--)

class REGISTER
{
public:
	constexpr REGISTER():ADDR(0) {}
	constexpr REGISTER(const REGISTER& rhs):ADDR(rhs.ADDR) {}
	constexpr REGISTER(uint8_t ADDR):ADDR(ADDR) {}
	uint8_t io_addr() const
	{
		return ADDR - __SFR_OFFSET;
	}
	uint8_t mem_addr() const
	{
		return ADDR;
	}
	operator volatile uint8_t& () const
	{
		return *((volatile uint8_t*) (uint16_t) ADDR);
	}
	operator volatile uint16_t& () const
	{
		return *((volatile uint16_t*) (uint16_t) ADDR);
	}
	//TODO small enhancement: use operators () instead
	void set(uint8_t value) const
	{
		*((volatile uint8_t*) (uint16_t) ADDR) = value;
	}
	uint8_t get() const
	{
		return *((volatile uint8_t*) (uint16_t) ADDR);
	}

private:	
	uint8_t ADDR;
};

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

inline void set_bit_field(REGISTER REG, uint8_t MASK, uint8_t VALUE) INLINE;
inline void set_bit_field(REGISTER REG, uint8_t MASK, uint8_t VALUE)
{
	volatile uint8_t& ref = (volatile uint8_t&) REG;
	ref = (ref & ~MASK) | (VALUE & MASK);
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
	//TODO check if optimization is possible in actual usage of this function
	// E.g. in code below, extra mov r15,r30 is not absolutely necessary (r30 could be directly used)
	// 2ae:   e0 e0           ldi     r30, 0x00       ; 0
	// 2b0:   4f 99           sbic    0x09, 7 ; 9
	// 2b2:   e1 e0           ldi     r30, 0x01       ; 1
	// 2b4:   fe 2e           mov     r15, r30
	bool result;
	asm volatile(
		"LDI %[RESULT], 0\n\t"			// Clear result value by default
		"SBIC %[IOREG], %[BIT] \n\t"
		"LDI %[RESULT], 1\n\t"			// Bit is set, set result value accordingly
		:[RESULT] "=&d" (result)
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
