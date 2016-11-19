#ifndef FASTIO_HH
#define	FASTIO_HH

#include "utilities.hh"
#include "Board.hh"
#include <fastarduino/iocommons.hh>

// This class maps to a PORT and handles it all 8 bits at a time
// SRAM size supposed to be 0
//TODO replace uint8_t with enum class Port (board-specific definition)
template<uint8_t PIN>
class FastPort
{
public:
	FastPort() {}
	FastPort(uint8_t ddr, uint8_t port = 0) INLINE
	{
		set_DDR(ddr);
		set_PORT(port);
	}
	void set_PORT(uint8_t port) INLINE
	{
		set_ioreg_byte(_PORT, port);
	}
	uint8_t get_PORT() INLINE
	{
		return get_ioreg_byte(_PORT);
	}
	void set_DDR(uint8_t ddr) INLINE
	{
		set_ioreg_byte(_DDR, ddr);
	}
	uint8_t get_DDR() INLINE
	{
		return get_ioreg_byte(_DDR);
	}
	void set_PIN(uint8_t pin) INLINE
	{
		set_ioreg_byte(_PIN, pin);
	}
	uint8_t get_PIN() INLINE
	{
		return get_ioreg_byte(_PIN);
	}

private:
	static const constexpr uint8_t _PIN = PIN;
	static const constexpr uint8_t _DDR = _PIN + 1;
	static const constexpr uint8_t _PORT = _PIN + 2;
};

// This class maps to a specific pin
// SRAM size supposed to be 0
//TODO add method to change PinMode (should not be fixed at ctor time only)
template<Board::DigitalPin DPIN>
class FastPin
{
public:
	FastPin(PinMode mode, bool value = false) INLINE
	{
		if (mode == PinMode::OUTPUT)
			set_ioreg_bit(_DDR, _BIT);
		else
			clear_ioreg_bit(_DDR, _BIT);
		if (value || mode == PinMode::INPUT_PULLUP)
			set_ioreg_bit(_PORT, _BIT);
		else
			clear_ioreg_bit(_PORT, _BIT);
	}
	void set() INLINE
	{
		set_ioreg_bit(_PORT, _BIT);
	}
	void clear() INLINE
	{
		clear_ioreg_bit(_PORT, _BIT);
	}
	void toggle() INLINE
	{
		set_ioreg_bit(_PIN, _BIT);
	}
	bool value() INLINE
	{
		return ioreg_bit_value(_PIN, _BIT);
	}
	
private:
	static const constexpr REGISTER _PIN = Board::PIN_REG(DPIN);
	static const constexpr REGISTER _DDR = Board::DDR_REG(DPIN);
	static const constexpr REGISTER _PORT = Board::PORT_REG(DPIN);
	static const constexpr uint8_t _BIT = Board::BIT(DPIN);
};


#endif	/* FASTIO_HH */

