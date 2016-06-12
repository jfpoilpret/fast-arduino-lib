#ifndef FASTPIN_HH
#define	FASTPIN_HH

#include <fastarduino/Board.hh>
#include <fastarduino/utilities.hh>

//TODO similar class with MASK to handle only a part of the pins in the port
//TODO rename classes and possibly add namespace

class AbstractPort
{
public:
	AbstractPort(uint8_t PIN ) __attribute__((always_inline)) : _PIN(PIN) {}
	
protected:
	volatile uint8_t* PIN() __attribute__((always_inline))
	{
		return (volatile uint8_t*) (uint16_t) _PIN;
	}
	volatile uint8_t* DDR() __attribute__((always_inline))
	{
		return (volatile uint8_t*) (uint16_t) (_PIN + 1);
	}
	volatile uint8_t* PORT() __attribute__((always_inline))
	{
		return (volatile uint8_t*) (uint16_t) (_PIN + 2);
	}
	
private:
	const uint8_t _PIN;
};

// This class maps to a PORT and handles it all 8 bits at a time
// SRAM size supposed is 1 byte
class FastPort: public AbstractPort
{
public:
	FastPort(uint8_t PIN ) __attribute__((always_inline)) : AbstractPort(PIN) {}
	FastPort(uint8_t PIN, uint8_t ddr, uint8_t port = 0) __attribute__((always_inline)) : AbstractPort(PIN)
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

enum class PinMode
{
	INPUT,
	INPUT_PULLUP,
	OUTPUT,
};

// This class maps to a specific pin
// SRAM size supposed is 2 bytes
class FastPin: public AbstractPort
{
public:
	FastPin(Board::DigitalPin DPIN, PinMode mode, bool value = false) __attribute__((always_inline))
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
	
	// All methods starting with _ are synchronized, ie they are interrupt-safe
	void _pin_mode(PinMode mode, bool value = false) __attribute__((always_inline))
	{
		ClearInterrupt clint;
		pin_mode(mode, value);
	}
	void _set() __attribute__((always_inline))
	{
		ClearInterrupt clint;
		set();
	}
	void _clear() __attribute__((always_inline))
	{
		ClearInterrupt clint;
		clear();
	}
	void _toggle() __attribute__((always_inline))
	{
		ClearInterrupt clint;
		toggle();
	}
	bool _value() __attribute__((always_inline))
	{
		ClearInterrupt clint;
		return value();
	}

protected:
	uint8_t BIT() __attribute__((always_inline))
	{
		return _BIT;
	}
	
private:
	const uint8_t _BIT;
};

#endif	/* FASTPIN_HH */
