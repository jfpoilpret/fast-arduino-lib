//   Copyright 2016-2017 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef FASTIO_HH
#define	FASTIO_HH

#include "utilities.h"
#include "boards/board_traits.h"

/**
 * Defines all API to manipulate digital input/output pins.
 */
namespace gpio
{
	/**
	 * Defines the configurable mode of a digital IO pin.
	 */
	enum class PinMode: uint8_t
	{
		/** Digital pin is configured as high-impedance (open drain) input. */
		INPUT,
		/** Digital pin is configured as input with an internal pullup resistor. */
		INPUT_PULLUP,
		/** Digital pin is configured as output. */
		OUTPUT,
	};

	/// @cond experimental
	// Experimental API, maybe removed if judged bad
	template<board::Port PORT, uint8_t BIT> class FastPin;
	class SlowPin
	{
	public:
		void set()
		{
			PORT_ |= BIT_;
		}
		void clear()
		{
			PORT_ &= ~BIT_;
		}
		void toggle()
		{
			PIN_ |= BIT_;
		}
		bool value()
		{
			return PIN_ & BIT_;
		}
		void set_mode(PinMode mode, bool value)
		{
			if (mode == PinMode::OUTPUT)
				DDR_ |= BIT_;
			else
				DDR_ &= ~BIT_;
			if (value || mode == PinMode::INPUT_PULLUP)
				PORT_ |= BIT_;
			else
				PORT_ &= ~BIT_;
		}
		
	private:
		SlowPin(volatile uint8_t& DDR, volatile uint8_t& PIN, volatile uint8_t& PORT, uint8_t BIT)
			:DDR_{DDR}, PIN_{PIN}, PORT_{PORT}, BIT_{BIT} {}
		
		volatile uint8_t& DDR_;
		volatile uint8_t& PIN_;
		volatile uint8_t& PORT_;
		const uint8_t BIT_;
		
		template<board::Port PORT, uint8_t BIT>
		friend class FastPin;
	};
	/// @endcond

	/**
	 * API that manipulates one digital IO pin of a given port.
	 * Implementation is highly optimized for size and speed: instances use
	 * no SRAM at all, most common methods use only 1 AVR instruction.
	 * 
	 * Note that, although it has public constructors, you generally do not 
	 * construct `FastPin` instances directly. You better use `FastPinType`,
	 * which is directly provided a `board::DigitalPin` constant. You may also
	 * obtain a `FastPin` instance through `FastPort::get_pin()`.
	 * 
	 * @tparam PORT_ the target port to which this pin belongs
	 * @tparam BIT_ the bit position (from `0` to `7`), in port, of this pin;
	 * note that if this position is not mapped to a physical IO of the MCU target,
	 * then a compilation error will occur.
	 * @sa board::Port
	 * @sa gpio::FastPinType
	 * @sa gpio::FastPort::get_pin()
	 */
	template<board::Port PORT_, uint8_t BIT_>
	class FastPin
	{
	private:
		using TRAIT = board_traits::Port_trait<PORT_>;

	public:
		/** The port to which this pin belongs. */
		static constexpr const board::Port PORT = PORT_;
		/** The bit position (from `0` to `7`), in port, of this pin. */
		static constexpr const uint8_t BIT = BIT_;

		/**
		 * Construct a `FastPin` without any physical setup on target MCU.
		 * This is useful if default pin direction and value are OK for you and 
		 * you want to avoid calling mode setup on target MCU.
		 */
		FastPin() INLINE
		{
			static_assert(TRAIT::DPIN_MASK & _BV(BIT), "BIT must be compatible with PORT available pins");
		}
		
		/**
		 * Construct a `FastPin` with the given mode and initial value.
		 * The pin mode is forced on the target MCU.
		 * 
		 * @param mode the mode of this pin
		 * @param value the initial pin level, if `mode == PinMode::OUTPUT`; not 
		 * used otherwise.
		 * @sa set_mode()
		 */
		FastPin(PinMode mode, bool value = false) INLINE
		{
			static_assert(TRAIT::DPIN_MASK & _BV(BIT), "BIT must be compatible with PORT available pins");
			set_mode(mode, value);
		}
		
		/**
		 * Set mode (direction) and value (if output) of this pin.
		 * 
		 * @param mode the mode of this pin
		 * @param value the initial pin level, if `mode == PinMode::OUTPUT`; not 
		 * used otherwise.
		 */
		void set_mode(PinMode mode, bool value = false) INLINE
		{
			if (mode == PinMode::OUTPUT)
				TRAIT::DDR |= _BV(BIT);
			else
				TRAIT::DDR &= ~_BV(BIT);
			if (value || mode == PinMode::INPUT_PULLUP)
				TRAIT::PORT |= _BV(BIT);
			else
				TRAIT::PORT &= ~_BV(BIT);
		}
		
		/**
		 * Set pin level to `HIGH` (i\.e\. Vcc).
		 * This method will generally use 1 single instruction in most cases.
		 */
		void set() INLINE
		{
			TRAIT::PORT |= _BV(BIT);
		}
		
		/**
		 * Set pin level to `LOW` (i\.e\. GND).
		 * This method will generally use 1 single instruction in most cases.
		 */
		void clear() INLINE
		{
			TRAIT::PORT &= ~_BV(BIT);
		}
		
		/**
		 * Toggle pin level, i\.e\. set it to `LOW` if it was `HIGH`, and `HIGH` 
		 * if it was `LOW`.
		 * This method will generally use 1 single instruction in most cases.
		 */
		void toggle() INLINE
		{
			TRAIT::PIN |= _BV(BIT);
		}
		
		/**
		 * Return the current level of this pin.
		 * @retval true if current pin  level is `HIGH` (i\.e\. Vcc)
		 * @retval false if current pin  level is `LOW` (i\.e\. GND)
		 */
		bool value() INLINE
		{
			return TRAIT::PIN & _BV(BIT);
		}

		// Experimental API, maybe removed if judged bad
		SlowPin as_slow_pin()
		{
			return SlowPin{TRAIT::DDR, TRAIT::PORT, TRAIT::PIN, BIT};
		}
	};

	/**
	 * API that manipulates a whole digital IO port.
	 * Implementation is highly optimized for size and speed: instances use
	 * no SRAM at all, most common methods use only 2 AVR instructions.
	 * 
	 * Note that using this API means that every method manipulates ALL pins of 
	 * a port at a time. If you need to handle several, but not all, pins of a 
	 * port, consider using gpio::FastMaskedPort instead.
	 * 
	 * @tparam PORT_ the target port
	 * @sa board::Port
	 * @sa gpio::FastMaskedPort
	 */
	template<board::Port PORT_>
	class FastPort
	{
	private:
		using TRAIT = board_traits::Port_trait<PORT_>;

	public:
		/** The actual port in target MCU. */
		static constexpr const board::Port PORT = PORT_;

		/**
		 * Construct a `FastPort` without any physical setup on target MCU.
		 * This is useful if default pins directions and values are OK for you and 
		 * you want to avoid calling mode setup on target MCU.
		 */
		FastPort() {}
		
		/**
		 * Construct a `FastPort` with the given direction byte and initial values
		 * byte.
		 * The pins mode is forced on the target MCU.
		 * 
		 * @param ddr the direction to set (in `DDR` register of this port) for 
		 * each pin (1 bit is one pin, TODO when `1` the pin is set as output, 
		 * when `0` as input).
		 * @param port the initial values for `PORT` register of this port; each  
		 * bit is for one pin of the port, its meaning depends on the pin direction: 
		 * if input, then it fixes if pullup resistor should be used, if output,
		 * then it fixes the output level of the pin.
		 * @sa set_DDR()
		 * @sa set_PORT()
		 */
		FastPort(uint8_t ddr, uint8_t port = 0) INLINE
		{
			set_DDR(ddr);
			set_PORT(port);
		}

		template<uint8_t BIT>
		FastPin<PORT, BIT> get_pin(PinMode mode, bool value = false)
		{
			return FastPin<PORT, BIT>{mode, value};
		}

		template<uint8_t BIT>
		FastPin<PORT, BIT> get_pin()
		{
			return FastPin<PORT, BIT>{};
		}

		void set_PORT(uint8_t port) INLINE
		{
			TRAIT::PORT = port;
		}
		uint8_t get_PORT() INLINE
		{
			return TRAIT::PORT;
		}
		void set_DDR(uint8_t ddr) INLINE
		{
			TRAIT::DDR = ddr;
		}
		uint8_t get_DDR() INLINE
		{
			return TRAIT::DDR;
		}
		void set_PIN(uint8_t pin) INLINE
		{
			TRAIT::PIN = pin;
		}
		uint8_t get_PIN() INLINE
		{
			return TRAIT::PIN;
		}
	};

	// This class maps to a PORT and handles several bits at a time based on a mask
	// SRAM size is 1 byte
	template<board::Port PORT_>
	class FastMaskedPort
	{
	private:
		using TRAIT = board_traits::Port_trait<PORT_>;

	public:
		static constexpr const board::Port PORT = PORT_;

		FastMaskedPort() {}
		FastMaskedPort(uint8_t mask, uint8_t ddr, uint8_t port = 0)
		:_mask{mask}
		{
			set_DDR(ddr);
			set_PORT(port);
		}

		void set_PORT(uint8_t port) INLINE
		{
			TRAIT::PORT = (TRAIT::PORT & ~_mask) | (port & _mask);
		}
		uint8_t get_PORT() INLINE
		{
			return TRAIT::PORT & _mask;
		}
		void set_DDR(uint8_t ddr) INLINE
		{
			TRAIT::DDR = (TRAIT::DDR & ~_mask) | (ddr & _mask);
		}
		uint8_t get_DDR() INLINE
		{
			return TRAIT::DDR & _mask;
		}
		void set_PIN(uint8_t pin) INLINE
		{
			TRAIT::PIN = pin & _mask;
		}
		uint8_t get_PIN() INLINE
		{
			return TRAIT::PIN & _mask;
		}

	private:
		uint8_t _mask;
	};

	/**
	 * API that manipulates a given digital IO pin of a the target MCU.
	 * It provides static methods to directly manipulate an IO pin (mode, level...)
	 * but it also helps you find out the exact `FastPin` type for a given IO pin.
	 * 
	 * Implementation is highly optimized for size and speed: it uses
	 * no SRAM at all, most common methods use only 1 AVR instruction.
	 * 
	 * The following snippet demonstrates usage of `FastPinType` to declare a 
	 * `FastPin` instance for later use in a function:
	 * 
	 * @code
	 * void f()
	 * {
	 *     gpio::FastPinType<board::DigitalPin::LED>::TYPE led{gpio::PinMode::OUTPUT};
	 *     led.clear();
	 *     ...
	 *     led.set();
	 * }
	 * @endcode
	 * 
	 * The next snippet demonstrate direct use of `FastPinType` static methods, 
	 * when you don't need an instance of `FastPin`:
	 * 
	 * @code
	 * void f()
	 * {
	 *     gpio::FastPinType<board::DigitalPin::LED>::set_mode(gpio::PinMode::INPUT);
	 *     ...
	 * }
	 * @endcode
	 * 
	 * @tparam DPIN a unique digital pin for the MCU target
	 * @sa gpio::DigitalPin
	 * @sa gpio::FastPin
	 */
	template<board::DigitalPin DPIN>
	class FastPinType
	{
	private:
		using TRAIT = board_traits::DigitalPin_trait<DPIN>;
		using PTRAIT = board_traits::Port_trait<TRAIT::PORT>;
		
	public:
		/** The port to which `DPIN` belongs. */
		static constexpr const board::Port PORT = TRAIT::PORT;
		/** The bit position of `DPIN` within its port. */
		static constexpr const uint8_t BIT = TRAIT::BIT;
		/** The bit-mask to use when accessing `DPIN` through `PORT`. */
		static constexpr const uint8_t MASK = _BV(BIT);
		
		/** The exact `FastPin` parameterized type for `DPIN` IO pin. */
		using TYPE = FastPin<PORT, BIT>;
		/** The exact `FastPort` parameterized type that `DPIN` IO pin belongs to. */
		using PORT_TYPE = FastPort<PORT>;
		
		/**
		 * Set mode (direction) and value (if output) of `DPIN`.
		 * 
		 * @param mode the mode of `PIN`
		 * @param value the initial pin level, if `mode == PinMode::OUTPUT`; not 
		 * used otherwise.
		 */
		static void set_mode(PinMode mode, bool value = false)
		{
			if (mode == PinMode::OUTPUT)
				PTRAIT::DDR |= _BV(BIT);
			else
				PTRAIT::DDR &= ~_BV(BIT);
			if (value || mode == PinMode::INPUT_PULLUP)
				PTRAIT::PORT |= _BV(BIT);
			else
				PTRAIT::PORT &= ~_BV(BIT);
		}
		
		/**
		 * Set pin level to `HIGH` (i\.e\. Vcc).
		 * This method will generally use 1 single instruction in most cases.
		 */
		static void set()
		{
			PTRAIT::PORT |= _BV(BIT);
		}
		
		/**
		 * Set pin level to `LOW` (i\.e\. GND).
		 * This method will generally use 1 single instruction in most cases.
		 */
		static void clear()
		{
			PTRAIT::PORT &= ~_BV(BIT);
		}
		
		/**
		 * Toggle pin level, i\.e\. set it to `LOW` if it was `HIGH`, and `HIGH` 
		 * if it was `LOW`.
		 * This method will generally use 1 single instruction in most cases.
		 */
		static void toggle()
		{
			PTRAIT::PIN |= _BV(BIT);
		}
		
		/**
		 * Return the current level pin `DPIN`.
		 * @retval true if current pin  level is `HIGH` (i\.e\. Vcc)
		 * @retval false if current pin  level is `LOW` (i\.e\. GND)
		 */
		static bool value()
		{
			return PTRAIT::PIN & _BV(BIT);
		}
	};

	/// @cond notdocumented
	template<>
	class FastPinType<board::DigitalPin::NONE>
	{
	public:
		static constexpr const board::Port PORT = board::Port::NONE;
		static constexpr const uint8_t BIT = 0;
		static constexpr const uint8_t MASK = 0;
		using TYPE = FastPin<PORT, BIT>;
		using PORT_TYPE = FastPort<PORT>;
		
		static void set_mode(UNUSED PinMode mode, UNUSED bool value = false) {}
		static void set() {}
		static void clear() {}
		static void toggle() {}
		static bool value()
		{
			return false;
		}
	};
	/// @endcond

	/// @cond notdocumented
	template<>
	class FastPin<board::Port::NONE, 0>
	{
	public:
		FastPin() INLINE {}
		FastPin(PinMode mode UNUSED, bool value UNUSED = false) INLINE {}
		void set() INLINE {}
		void clear() INLINE {}
		void toggle() INLINE {}
		bool value() INLINE
		{
			return false;
		}
	};
	/// @endcond
}

#endif	/* FASTIO_HH */
