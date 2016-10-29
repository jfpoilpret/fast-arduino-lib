#ifndef IO_HH
#define	IO_HH

#include "Board.hh"
#include "utilities.hh"
#include "iocommons.hh"

class AbstractPort
{
public:
	AbstractPort() INLINE {}
	AbstractPort(REGISTER PIN) INLINE : _PIN{PIN} {}
	
protected:
	REGISTER PIN() INLINE
	{
		return _PIN;
	}
	REGISTER DDR() INLINE
	{
		return REGISTER(_PIN.mem_addr() + 1);
	}
	REGISTER PORT() INLINE
	{
		return REGISTER(_PIN.mem_addr() + 2);
	}
	
private:
	REGISTER _PIN;
};

// This class maps to a PORT and handles it all 8 bits at a time
// SRAM size is 2 bytes
class IOPort: public AbstractPort
{
public:
	IOPort() INLINE {}
	IOPort(REGISTER PIN) INLINE : AbstractPort{PIN} {}
	IOPort(REGISTER PIN, uint8_t ddr, uint8_t port = 0) INLINE : AbstractPort{PIN}
	{
		set_DDR(ddr);
		set_PORT(port);
	}
	
	void set_PORT(uint8_t port) INLINE
	{
		PORT().set(port);
	}
	uint8_t get_PORT() INLINE
	{
		return PORT().get();
	}
	void set_DDR(uint8_t ddr) INLINE
	{
		DDR().set(ddr);
	}
	uint8_t get_DDR() INLINE
	{
		return DDR().get();
	}
	void set_PIN(uint8_t pin) INLINE
	{
		PIN().set(pin);
	}
	uint8_t get_PIN() INLINE
	{
		return PIN().get();
	}
};

// This class maps to a portion of a PORT (according to provided mask) and handles all its bits at a time
// SRAM size is 3 bytes
class IOMaskedPort: public AbstractPort
{
public:
	IOMaskedPort() INLINE {}
	IOMaskedPort(REGISTER PIN, uint8_t mask = 0) INLINE
	: AbstractPort(PIN), _MASK(mask) {}
	IOMaskedPort(REGISTER PIN, uint8_t mask, uint8_t ddr, uint8_t port = 0) INLINE 
	: AbstractPort(PIN), _MASK(mask)
	{
		set_DDR(ddr);
		set_PORT(port);
	}
	void set_PORT(uint8_t port) INLINE
	{
		MASK_VALUE(PORT(), port);
	}
	uint8_t get_PORT() INLINE
	{
		return VALUE(PORT());
	}
	void set_DDR(uint8_t ddr) INLINE
	{
		MASK_VALUE(DDR(), ddr);
	}
	uint8_t get_DDR() INLINE
	{
		return VALUE(DDR());
	}
	void set_PIN(uint8_t pin) INLINE
	{
		PIN().set(pin & MASK());
	}
	uint8_t get_PIN() INLINE
	{
		return PIN().get() & MASK();
	}
	
protected:
	uint8_t MASK() INLINE
	{
		return _MASK;
	}
	void MASK_VALUE(volatile uint8_t& reg, uint8_t value) INLINE
	{
		reg = (reg & ~MASK()) | (value & MASK());
	}
	uint8_t VALUE(uint8_t reg) INLINE
	{
		return reg & MASK();
	}
	
private:
	uint8_t _MASK;
};

// This class maps to a specific pin
// SRAM size supposed is 3 bytes
class IOPin: public AbstractPort
{
public:
	IOPin() INLINE : _BIT{0} {}
	IOPin(Board::DigitalPin DPIN, PinMode mode, bool value = false) INLINE
	: AbstractPort{Board::PIN_REG(DPIN)}, _BIT(_BV(Board::BIT(DPIN)))
	{
		pin_mode(mode, value);
	}
	void pin_mode(PinMode mode, bool value = false) INLINE
	{
		if (mode == PinMode::OUTPUT)
			set_mask(DDR(), BIT());
		else
			clear_mask(DDR(), BIT());
		if (value || mode == PinMode::INPUT_PULLUP)
			set_mask(PORT(), BIT());
		else
			clear_mask(PORT(), BIT());
	}
	void set() INLINE
	{
		set_mask(PORT(), BIT());
	}
	void clear() INLINE
	{
		clear_mask(PORT(), BIT());
	}
	void toggle() INLINE
	{
		PIN().set(BIT());
	}
	bool value() INLINE
	{
		return PIN().get() & BIT();
	}
	
protected:
	uint8_t BIT() INLINE
	{
		return _BIT;
	}
	
private:
	uint8_t _BIT;
};

#endif	/* IO_HH */
