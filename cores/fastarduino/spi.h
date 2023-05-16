//   Copyright 2016-2022 Jean-Francois Poilpret
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

/// @cond api

/**
 * @file
 * SPI support for AVR MCU.
 */
#ifndef FASTSPI_HH
#define FASTSPI_HH

#include "boards/board.h"
#include "gpio.h"

/**
 * Define API to define and manage SPI devices.
 * SPI is available to all MCU supported by FastArduino, even in ATtiny MCU, for which
 * SPI is implemented with *Universal Serial Interface* (USI).
 * @note USI does not allow full SPI support, contrarily to native SPI support of
 * ATmega MCU. When differences exist, they are documented in the API.
 */
namespace spi
{
	/**
	 * This function must be called once in your program, before any use of an SPI device.
	 * It simply sets up the pins used by SPI interface: MOSI, MISO, SCK.
	 */
	void init();

	/**
	 * Define SPI clock rate as a divider of MCU clock frequency.
	 * @note this is not used in ATtiny.
	 * @sa compute_clockrate()
	 */
	enum class ClockRate : uint8_t
	{
		CLOCK_DIV_4 = 0x00,
		CLOCK_DIV_16 = 0x01,
		CLOCK_DIV_64 = 0x02,
		CLOCK_DIV_128 = 0x03,
		CLOCK_DIV_2 = 0x10,
		CLOCK_DIV_8 = 0x11,
		CLOCK_DIV_32 = 0x12
	};

	/**
	 * Calculate `ClockRate` for the given @p frequency.
	 * Computations done by this method will be performed at compile-time as long
	 * as all provided arguments are constants; this is important as this will
	 * help optimize code size and execution time.
	 * @sa ClockRate
	 */
	constexpr ClockRate compute_clockrate(uint32_t frequency)
	{
		if (frequency >= (F_CPU / 2))
			return ClockRate::CLOCK_DIV_2;
		else if (frequency >= (F_CPU / 4))
			return ClockRate::CLOCK_DIV_4;
		else if (frequency >= (F_CPU / 8))
			return ClockRate::CLOCK_DIV_8;
		else if (frequency >= (F_CPU / 16))
			return ClockRate::CLOCK_DIV_16;
		else if (frequency >= (F_CPU / 32))
			return ClockRate::CLOCK_DIV_32;
		else if (frequency >= (F_CPU / 64))
			return ClockRate::CLOCK_DIV_64;
		else
			return ClockRate::CLOCK_DIV_128;
	}

#ifdef SPDR
	/**
	 * Bit ordering per byte.
	 */
	enum class DataOrder : uint8_t
	{
		/** Most significant bit transferred first. */
		MSB_FIRST = 0,
		/**
		 * Least significant bit transferred first.
		 * @note this is not available on ATtiny MCU.
		 */
		LSB_FIRST = bits::BV8(DORD)
	};
#else
	enum class DataOrder : uint8_t
	{
		MSB_FIRST = 0
	};
#endif

#ifdef SPDR
	/**
	 * SPI transmission mode.
	 * @sa https://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Mode_numbers
	 */
	enum class Mode : uint8_t
	{
		/** SPI mode 0: CPOL = 0 and CPHA = 0. */
		MODE_0 = 0,
		/** SPI mode 1: CPOL = 0 and CPHA = 1. */
		MODE_1 = bits::BV8(CPHA),
		/**
		 * SPI mode 2: CPOL = 1 and CPHA = 0.
		 * @note not available on ATtiny MCU.
		 */
		MODE_2 = bits::BV8(CPOL),
		/**
		 * SPI mode 3: CPOL = 1 and CPHA = 1.
		 * @note not available on ATtiny MCU.
		 */
		MODE_3 = bits::BV8(CPHA, CPOL)
	};
#else
	enum class Mode : uint8_t
	{
		MODE_0 = bits::BV8(USIWM0, USICLK, USICS1),
		MODE_1 = bits::BV8(USIWM0, USICLK, USICS1, USICS0)
	};
#endif

	/**
	 * Active polarity of slave selection pin.
	 */
	enum class ChipSelect : uint8_t
	{
		/** Slave device is active when SS pin is low. */
		ACTIVE_LOW = 0,
		/** Slave device is active when SS pin is high. */
		ACTIVE_HIGH = 1
	};

	/**
	 * Contain general payload transfer API for an SPI device.
	 * This is not supposed to be used directly, as it does not include start/end
	 * transfer methods. SPI device implementers shall use `SPIDevice` template class
	 * instead.
	 * @sa SPIDevice
	 */
	class AbstractSPIDevice
	{
	protected:
		AbstractSPIDevice() = default;
		AbstractSPIDevice(const AbstractSPIDevice&) = delete;
		AbstractSPIDevice& operator=(const AbstractSPIDevice&) = delete;

#ifdef SPDR
		/**
		 * Transfer one byte to the currently selected SPI slave device through MOSI
		 * pin, and get the byte returned by the device through MISO pin.
		 * @param data the byte to transmit to slave device
		 * @return the byte received from slave device
		 * 
		 * @sa transfer(uint8_t*, uint16_t)
		 * @sa transfer(uint8_t*, uint16_t, uint8_t)
		 */
		uint8_t transfer(uint8_t data)
		{
			SPDR_ = data;
			SPSR_.loop_until_bit_set(SPIF);
			return SPDR_;
		}
#else
		uint8_t transfer(uint8_t data)
		{
			USIDR_ = data;
			// Clear counter overflow before transmission
			USISR_ = bits::BV8(USIOIF);
			synchronized
			{
				// Note: the following loop means 7 clock cycles per bit transmitted
				// Hence at 8MHz, that gives a SPI rate > 1MHz, which might be incompatible 
				// with some devices
				// FIXME add a NOP to ensure 8 clock cycles per bit, hence 1MHz SPI clock
				while ((USISR_ & bits::BV8(USIOIF)) == 0) USICR_ |= bits::BV8(USITC);
			}
			return USIDR_;
		}
#endif

		/**
		 * Transfer an array of payload data to the currently selected SPI slave device
		 * through MOSI pin, and get all data bytes simultaneously received from that
		 * device through MISO pin.
		 * @param data pointer to the payload to transmit; is also the placeholder for 
		 * payload simultaneously returned by the slave.
		 * @param size the payload size
		 * 
		 * @sa transfer(uint8_t)
		 * @sa transfer(uint8_t*, uint16_t, uint8_t)
		 * @sa transfer(const uint8_t*, uint16_t)
		 */
		void transfer(uint8_t* data, uint16_t size)
		{
			while (size--)
			{
				uint8_t value = *data;
				*data++ = transfer(value);
			}
		}

		/**
		 * Transfer an array of payload data to the currently selected SPI slave device
		 * through MOSI pin; any data bytes simultaneously received from that
		 * device through MISO pin are lost.
		 * @param data pointer to the payload to transmit
		 * @param size the payload size
		 * 
		 * @sa transfer(uint8_t)
		 * @sa transfer(uint16_t, uint8_t)
		 * @sa transfer(uint8_t*, uint16_t)
		 */
		void transfer(const uint8_t* data, uint16_t size)
		{
			while (size--) transfer(*data++);
		}

		/**
		 * Transfer the provided byte @p sent several times to the currently selected
		 * SPI slave device through MOSI pin, and get all data bytes simultaneously received 
		 * from that device through MISO pin.
		 * 
		 * This method is useful when you have requested information from the slave device
		 * and then you must always send the same byte to receive all requested information:
		 * with this method, you do not need to initialize @p data with @p sent @p size times.
		 * 
		 * @param data placeholder for the payload returned by the slave whilst transmitting
		 * @p size times the @p sent byte
		 * @param size the number of times to transmit @p sent byte to the SPI slave
		 * @param sent the data byte to transmit several times
		 * 
		 * @sa transfer(uint8_t)
		 * @sa transfer(uint8_t*, uint16_t)
		 * @sa transfer(uint16_t, uint8_t)
		 */
		void transfer(uint8_t* data, uint16_t size, uint8_t sent)
		{
			while (size--) *data++ = transfer(sent);
		}

		/**
		 * Transfer the provided byte @p sent several times to the currently selected
		 * SPI slave device through MOSI pin; any data bytes simultaneously received 
		 * from that device through MISO pin are lost.
		 * 
		 * @param size the number of times to transmit @p sent byte to the SPI slave
		 * @param sent the data byte to transmit several times
		 * 
		 * @sa transfer(uint8_t)
		 * @sa transfer(const uint8_t*, uint16_t)
		 * @sa transfer(uint8_t*, uint16_t, uint8_t)
		 */
		void transfer(uint16_t size, uint8_t sent)
		{
			while (size--) transfer(sent);
		}

	private:
		using REG8 = board_traits::REG8;
#ifdef SPDR
		static constexpr const REG8 SPDR_{SPDR};
		static constexpr const REG8 SPSR_{SPSR};
#else
		static constexpr const REG8 USIDR_{USIDR};
		static constexpr const REG8 USISR_{USISR};
		static constexpr const REG8 USICR_{USICR};
#endif
	};

	/**
	 * Base class for any SPI slave device.
	 * It contains the barebones API for implementing a real device.
	 * 
	 * Implementing a new SPI device consists mainly in subclassing `SPIDevice`
	 * and add `public` device-specific methods that will use `start_transfer()`,
	 * `transfer()` and `end_transfer()` to communicate with the actual device.
	 * 
	 * The snippet below illustrates this and summarizes the definition of an 
	 * SPI based device, the Winbond W25Q80BV flash-memory chip:
	 * @code
	 * template<board::DigitalPin CS>
	 * class WinBond : public spi::SPIDevice<CS, spi::ChipSelect::ACTIVE_LOW, spi::ClockRate::CLOCK_DIV_2>
	 * {
	 * public:
	 *     void write_page(uint32_t address, uint8_t* data, uint8_t size)
	 *     {
	 *         send(0x02, address, data, (size == 0 ? 256 : size));
	 *     }
	 *     uint8_t read_data(uint32_t address);
	 *     void read_data(uint32_t address, uint8_t* data, uint16_t size);
	 *     ...
	 * private:
	 *     void send(uint8_t code, uint32_t address, uint8_t* data, uint16_t size)
	 *     {
	 *         this->start_transfer();
	 *         this->transfer(code);
	 *         this->transfer(address >> 16);
	 *         this->transfer(address >> 8);
	 *         this->transfer(address);
	 *         this->transfer(data, size);
	 *         this->end_transfer();
	 *     }
	 *     ...
	 * };
	 * @endcode
	 * 
	 * @tparam CS the pin used to select the slave device; this may be `DigitalPin::NONE`
	 * if your device is alone on the SPI bus, its CS pin is forced low (always active),
	 * and your device actually supports this way of working. Some devices do not like
	 * having an always-0 CS pin!
	 * @tparam CS_MODE the chip select active mode
	 * @tparam RATE the SPI clock rate for this device
	 * @tparam MODE the SPI mode used for this device
	 * @tparam ORDER the bit order for this device
	 */
	template<board::DigitalPin CS, ChipSelect CS_MODE = ChipSelect::ACTIVE_LOW, ClockRate RATE = ClockRate::CLOCK_DIV_4,
			 Mode MODE = Mode::MODE_0, DataOrder ORDER = DataOrder::MSB_FIRST>
	class SPIDevice : public AbstractSPIDevice
	{
	protected:
		/**
		 * Create a new `SPIDevice`; this sets up the @p CS pin for later use 
		 * during transfers.
		 */
		SPIDevice() INLINE = default;

#ifdef SPDR
		/**
		 * Start an SPI transfer to this device.
		 * Concretely this sets the active level on @p CS pin.
		 */
		void start_transfer()
		{
			cs_.toggle();
			SPCR_ = SPCR_START_;
			SPSR_ = SPSR_START_;
		}
#else
		void start_transfer()
		{
			cs_.toggle();
			// Set 3-wire mode (SPI) and requested SPI mode (0 or 1) and use software clock strobe (through USITC)
			USICR_ = USICR_START_;
		}
#endif
		/**
		 * End the current SPI ransfer tot hsi device.
		 * Concretely this clears the active level on @p CS pin.
		 */
		void end_transfer() INLINE
		{
			cs_.toggle();
		}

	private:
		using REG8 = board_traits::REG8;
#ifdef SPDR
		static constexpr const REG8 SPCR_{SPCR};
		static constexpr const REG8 SPDR_{SPDR};
		static constexpr const REG8 SPSR_{SPSR};
		// Configuration values to reset at beginning of each transfer
		static const constexpr uint8_t SPCR_START_ =
			bits::BV8(SPE, MSTR) | (uint8_t(RATE) & 0x03U) | uint8_t(ORDER) | uint8_t(MODE);
		static const constexpr uint8_t SPSR_START_ = (uint8_t(RATE) & 0x10U) ? bits::BV8(SPI2X) : 0;
#else
		static constexpr const REG8 USIDR_{USIDR};
		static constexpr const REG8 USISR_{USISR};
		static constexpr const REG8 USICR_{USICR};
		static const constexpr uint8_t USICR_START_ = uint8_t(MODE);
#endif
		gpio::FAST_PIN<CS> cs_ = gpio::FAST_PIN<CS>{gpio::PinMode::OUTPUT, CS_MODE == ChipSelect::ACTIVE_LOW};
	};
};

#endif /* FASTSPI_HH */
/// @endcond
