#ifndef IO_HH
#define	IO_HH

#include <fastarduino/Board.hh>
#include <fastarduino/utilities.hh>

class AbstractPort
{
public:
	AbstractPort(volatile uint8_t* PIN) __attribute__((always_inline)) : _PIN(PIN) {}
	
protected:
	volatile uint8_t* PIN() __attribute__((always_inline))
	{
		return _PIN;
	}
	volatile uint8_t* DDR() __attribute__((always_inline))
	{
		return _PIN + 1;
	}
	volatile uint8_t* PORT() __attribute__((always_inline))
	{
		return _PIN + 2;
	}
	
private:
	volatile uint8_t* const _PIN;
};

// This class maps to a PORT and handles it all 8 bits at a time
// SRAM size is 2 bytes
class IOPort: public AbstractPort
{
public:
	IOPort(volatile uint8_t* PIN) __attribute__((always_inline)) : AbstractPort(PIN) {}
	IOPort(volatile uint8_t* PIN, uint8_t ddr, uint8_t port = 0) __attribute__((always_inline)) : AbstractPort(PIN)
	{
		set_DDR(ddr);
		set_PORT(port);
	}
	void set_PORT(uint8_t port) __attribute__((always_inline))
	{
		*PORT() = port;
	}
	uint8_t get_PORT() __attribute__((always_inline))
	{
		return *PORT();
	}
	void set_DDR(uint8_t ddr) __attribute__((always_inline))
	{
		*DDR() = ddr;
	}
	uint8_t get_DDR() __attribute__((always_inline))
	{
		return *DDR();
	}
	void set_PIN(uint8_t pin) __attribute__((always_inline))
	{
		*PIN() = pin;
	}
	uint8_t get_PIN() __attribute__((always_inline))
	{
		return *PIN();
	}
};

// This class maps to a portion of a PORT (according to provided mask) and handles all its bits at a time
// SRAM size is 3 bytes
class MaskedPort: public AbstractPort
{
public:
	MaskedPort(volatile uint8_t* PIN, uint8_t mask) __attribute__((always_inline))
	: AbstractPort(PIN), _MASK(mask) {}
	MaskedPort(volatile uint8_t* PIN, uint8_t mask, uint8_t ddr, uint8_t port = 0) __attribute__((always_inline)) 
	: AbstractPort(PIN), _MASK(mask)
	{
		set_DDR(ddr);
		set_PORT(port);
	}
	void set_PORT(uint8_t port) __attribute__((always_inline))
	{
		MASK_VALUE(*PORT(), port);
	}
	uint8_t get_PORT() __attribute__((always_inline))
	{
		return VALUE(*PORT());
	}
	void set_DDR(uint8_t ddr) __attribute__((always_inline))
	{
		MASK_VALUE(*DDR(), ddr);
	}
	uint8_t get_DDR() __attribute__((always_inline))
	{
		return VALUE(*DDR());
	}
	void set_PIN(uint8_t pin) __attribute__((always_inline))
	{
		MASK_VALUE(*PIN(), pin);
	}
	uint8_t get_PIN() __attribute__((always_inline))
	{
		return *PIN() & MASK();
	}
	
protected:
	uint8_t MASK() __attribute__((always_inline))
	{
		return _MASK;
	}
	void MASK_VALUE(volatile uint8_t& reg, uint8_t value) __attribute__((always_inline))
	{
		reg = (reg & ~MASK()) | (value & MASK());
	}
	uint8_t VALUE(uint8_t reg) __attribute__((always_inline))
	{
		return reg & MASK();
	}
	
private:
	const uint8_t _MASK;
};

enum class PinMode
{
	INPUT,
	INPUT_PULLUP,
	OUTPUT,
};

// This class maps to a specific pin
// SRAM size supposed is 3 bytes
class IOPin: public AbstractPort
{
public:
	IOPin(Board::DigitalPin DPIN, PinMode mode, bool value = false) __attribute__((always_inline))
	: AbstractPort(Board::PIN(DPIN)), _BIT(1 << Board::BIT(DPIN))
	{
		pin_mode(mode, value);
	}
	void pin_mode(PinMode mode, bool value = false) __attribute__((always_inline))
	{
		if (mode == PinMode::OUTPUT)
			*DDR() |= BIT();
		else
			*DDR() ^= ~BIT();
		if (value || mode == PinMode::INPUT_PULLUP)
			*PORT() |= BIT();
		else
			*PORT() &= ~BIT();
	}
	void set() __attribute__((always_inline))
	{
		*PORT() |= BIT();
	}
	void clear() __attribute__((always_inline))
	{
		*PORT() &= ~BIT();
	}
	void toggle() __attribute__((always_inline))
	{
		*PIN() |= BIT();
	}
	bool value() __attribute__((always_inline))
	{
		return *PIN() & BIT();
	}
	
protected:
	uint8_t BIT() __attribute__((always_inline))
	{
		return _BIT;
	}
	
private:
	const uint8_t _BIT;
};

#endif	/* IO_HH */
