// Important copyright notice: 
// Some parts of this file were copied from Mikael Patel's Cosa library, which copyright appears below.
// Some parts of this file directly derive from https://tty1.net/blog/2008/avr-gcc-optimisations_en.html
// Other parts are under Copyright (c) 2016, Jean-Francois Poilpret

/**
 * @file Cosa/Types.h
 * @version 1.0
 *
 * @section License
 * Copyright (C) 2012-2015, Mikael Patel
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * @section Description
 * Common literals, data types and syntax abstractions.
 *
 * This file is part of the Arduino Che Cosa project.
 */
#ifndef UTILITIES_HH
#define	UTILITIES_HH

#include <avr/io.h>
#include <avr/interrupt.h>

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

// Macro found on https://tty1.net/blog/2008/avr-gcc-optimisations_en.html
// This allows processing pointers to SRAM data be performed directly from Y, Z registers
// this may optimize code size on some circumstances
#define FIX_BASE_POINTER(_ptr) __asm__ __volatile__("" : "=b" (_ptr) : "0" (_ptr))

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

constexpr uint16_t as_uint16_t(uint8_t high, uint8_t low)
{
	return (high << 8) | low;
}

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

// Useful macro to iterate
// NOTE: these macros have been inspired by several readings:
// http://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments
// http://stackoverflow.com/questions/1872220/is-it-possible-to-iterate-over-arguments-in-variadic-macros
// https://github.com/pfultz2/Cloak/wiki/C-Preprocessor-tricks,-tips,-and-idioms

#define EMPTY(...)
#define CAT(X, Y) X##Y
#define COMMA() ,
#define STRINGIFY(X, ...) #X
#define ID(X, ...) X

#define FE_0_(M, DATA, FIRST, SEP, LAST) 
#define FE_1_(M, DATA, FIRST, SEP, LAST, X1) FIRST() M(X1, DATA) LAST()
#define FE_2_(M, DATA, FIRST, SEP, LAST, X1, X2) FIRST() M(X1, DATA) SEP() M(X2, DATA) LAST()
#define FE_3_(M, DATA, FIRST, SEP, LAST, X1, X2, X3) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) LAST()
#define FE_4_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4) FIRST()  M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) LAST()
#define FE_5_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) LAST()
#define FE_6_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5, X6) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) SEP() M(X6, DATA) LAST()
#define FE_7_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5, X6, X7) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) SEP() M(X6, DATA) SEP() M(X7, DATA) LAST()
#define FE_8_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5, X6, X7, X8) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) SEP() M(X6, DATA) SEP() M(X7, DATA) SEP() M(X8, DATA) LAST()
#define FE_9_(M, DATA, FIRST, SEP, LAST, X1, X2, X3, X4, X5, X6, X7, X8, X9) FIRST() M(X1, DATA) SEP() M(X2, DATA) SEP() M(X3, DATA) SEP() M(X4, DATA) SEP() M(X5, DATA) SEP() M(X6, DATA) SEP() M(X7, DATA) SEP() M(X8, DATA) SEP() M(X9, DATA) LAST()

#define GET_MACRO_9_(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, NAME,...) NAME 

// FOR_EACH executes M macro for each argument passed to it, returns empty if passed an empty list.
// Number of arguments in variadic list must be between 0 and 9
#define FOR_EACH(M, DATA, ...) GET_MACRO_9_(unused, ##__VA_ARGS__, FE_9_, FE_8_, FE_7_, FE_6_, FE_5_, FE_4_, FE_3_, FE_2_, FE_1_, FE_0_)(M, DATA, EMPTY, EMPTY, EMPTY, ## __VA_ARGS__)
// FOR_EACH_SEP executes M macro for each argument passed to it, separates each transformed value with SEP(),
// prepends FIRST() and appends LAST() if result list is not empty; returns empty if passed an empty list
// Number of arguments in variadic list must be between 0 and 9
#define FOR_EACH_SEP(M, DATA, FIRST, SEP, LAST, ...) GET_MACRO_9_(unused, ##__VA_ARGS__, FE_9_, FE_8_, FE_7_, FE_6_, FE_5_, FE_4_, FE_3_, FE_2_, FE_1_, FE_0_)(M, DATA, FIRST, SEP, LAST, ##__VA_ARGS__)

#endif	/* UTILITIES_HH */
