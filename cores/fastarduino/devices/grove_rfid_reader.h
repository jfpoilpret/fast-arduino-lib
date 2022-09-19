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
 * API to handle [Grove 125KHz RFID Reader](https://wiki.seeedstudio.com/Grove-125KHz_RFID_Reader/).
 */
#ifndef GROVE_RFID_READER_HH
#define GROVE_RFID_READER_HH

#include "../bits.h"
#include "../gpio.h"
#include "../int.h"
#include "../interrupts.h"
#include "../pci.h"
#include "../streambuf.h"
#include "../streams.h"
#include "../time.h"
#include "../uart_commons.h"
#include "wiegand.h"

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * devices::rfid::Grove125KHzRFIDReaderWiegandEXT to work correctly.
 * @note Grove125KHzRFIDReaderWiegandEXT uses 2 EXT pins, hence this macro
 * will generate 2 ISR, one for each pin.
 * 
 * @param DATA0_INT the number of the `INT` vector for the given DATA0 pin of 
 * Grove RFID reader device
 * @param DATA1_INT the number of the `INT` vector for the given DATA1 pin of 
 * Grove RFID reader device
 * @param READER the actual Grove125KHzRFIDReaderWiegandEXT<...> used
 * 
 * @sa Grove125KHzRFIDReaderWiegandEXT
 */
#define REGISTER_GROVE_RFID_READER_INT_ISR(DATA0_INT, DATA1_INT, READER)					\
	ISR(CAT3(INT, DATA0_INT, _vect))														\
	{																						\
		devices::rfid::isr_handler_grove::callback_fall_0<DATA0_INT, DATA1_INT, READER>();	\
	}																						\
	ISR(CAT3(INT, DATA1_INT, _vect))														\
	{																						\
		devices::rfid::isr_handler_grove::callback_fall_1<DATA0_INT, DATA1_INT, READER>();	\
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for a 
 * devices::rfid::Grove125KHzRFIDReaderWiegandPCI to work correctly.
 * @note Grove125KHzRFIDReaderWiegandPCI uses 2 PCI pins which must be on the 
 * same I/O port.
 * 
 * @param DATA01_PCI the number of the `PCINT` vector for the given DATA0 & DATA1
 * pins of Grove RFID reader device
 * @param READER the actual Grove125KHzRFIDReaderWiegandPCI<...> used
 * 
 * @sa Grove125KHzRFIDReaderWiegandPCI
 */
#define REGISTER_GROVE_RFID_READER_PCI_ISR(DATA01_PCI, READER)							\
	ISR(CAT3(PCINT, DATA01_PCI, _vect))													\
	{																					\
		devices::rfid::isr_handler_grove::callback_fall_0_or_1<DATA01_PCI, READER>();	\
	}																					\

namespace devices
{
	/**
	 * This namespace contains classes to support various RFID devices.
	 */
	namespace rfid
	{
	}
}

namespace devices::rfid
{
	/**
	 * Support for seeedstudio [Grove 125KHz RFID Reader](https://wiki.seeedstudio.com/Grove-125KHz_RFID_Reader/)
	 * in UART mode.
	 * 
	 * Pinout:
	 * - red cable:		+5V
	 * - black cable:	GND
	 * - white cable:	TX (probably not needed)
	 * - yellow cable:	RX
	 * 
	 * In general, it is preferred to use Wiegand mode rather than UART mode on the
	 * Grove device because:
	 * 1. It does not use UART (whether hardware or software)
	 * 2. It is more efficient in terms of ISR usage
	 * 
	 * @warning Although @p UART_ may be a software UART implementation, it is not 
	 * recommend to do so, because current FastArduino software UART implementation
	 * is not optimized for slow rates such as 9600bps as used by Grove device.
	 * 
	 * @note It is unclear, from original documentation and code examples, 
	 * whether this module can write tags or not. Hence we consider writing is
	 * not possible and hence white cable is also useless to connect in UART mode.
	 * 
	 * @tparam UART_ the UART_ class to use to access the Grove device; this may
	 * be hardware or software, this may be either UART or UARX (UATX not used).
	 * 
	 * @sa Grove125KHzRFIDReaderWiegandEXT
	 * @sa Grove125KHzRFIDReaderWiegandPCI
	 */
	template<typename UART_> class Grove125KHzRFIDReaderUART
	{
		static constexpr uint16_t UART_SPEED = 9600;

		// Check that UART type is suitable
		static_assert(serial::UART_trait<UART_>::IS_UART, "UART template argument must be a serial device");
		static_assert(serial::UART_trait<UART_>::HAS_RX, "UART template argument must be a serial device with RX mode");

	public:
		/** The UART type used to communicate with the Grove device. */
		using UART = UART_;

		/**
		 * Construct a new Grove 125KHz RFID Reader UART instance
		 * 
		 * @param uart the UART to use for communicating with the Grove device;
		 * the caller shall just care about instantiation, this Grove125KHzRFIDReaderUART
		 * instance will handle everything else on @p uart.
		 */
		explicit Grove125KHzRFIDReaderUART(UART& uart) : uart_{uart}, in_{uart_.in()}, buf_{in_.rdbuf()} {}

		/// @cond notdocumented
		Grove125KHzRFIDReaderUART(const Grove125KHzRFIDReaderUART&) = delete;
		Grove125KHzRFIDReaderUART& operator=(const Grove125KHzRFIDReaderUART&) = delete;
		/// @endcond
		
		/**
		 * Start operations of the device. From here, all data received from the Grove
		 * device will be read into UART buffer.
		 * @sa end()
		 */
		void begin()
		{
			// Just start UART with proper settings: 9600bps, 8 bits, 1 stop, no parity
			uart_.begin(UART_SPEED, serial::Parity::NONE, serial::StopBits::ONE);
		}

		/**
		 * Stop operations of the device. From here, any data received from the Grove
		 * device will be lost.
		 * @sa begin()
		 */
		void end()
		{
			// Stop UART
			uart_.end(serial::BufferHandling::CLEAR);
		}

		/**
		 * Check if data from Grove device is ready to read.
		 * In this situation, your program can call `get_data()` to receive the 
		 * complete access data (read from an RFID tag).
		 * 
		 * @note Actually this method will return `true` as soon as the first 
		 * character transmitted by the Grove device is ready, even if the remaining
		 * caharacters have not been received yet. `get_data()` will wait for and read
		 * the remaining characters if needed.
		 * 
		 * @sa get_data()
		 */
		bool has_data()
		{
			return buf_.sgetc() != streams::istreambuf::EOF;
		}

		/**
		 * Get complete data as ASCII C-string from the Grove device.
		 * Data is displayable in ASCII as an hexadecimal representation of RFID 
		 * tag actual data, e.g. "0F0024ADC442".
		 * 
		 * On UARX wire, the data transmitted by Grove device is embedded between
		 * special ASCII `STX` (`0x02`) and `ETX` (`0x03`); those markers are removed
		 * from returned data.
		 * 
		 * @note with tags used for tests, data read was 12 characters long 
		 * (6 bytes formatted as hexadecimal ASCII string); it is not clear yet, 
		 * whether other tags might produce different sizes on the Grove device.
		 * 
		 * @note this method might change in the future, depending on Grove device 
		 * behavior on different types of RFID tags.
		 * 
		 * @warning this method will block until data is available and complete! Call
		 * `has_data()` first, to ensure data will be available.
		 * 
		 * @param data data buffer to be fed from device data
		 * @param size size of @p data ; this will typically depend on RFID tags that
		 * you use; do not forget to add 1 character ofr the ending `\\0`.
		 */
		void get_data(char* data, uint8_t size)
		{
			// 1. Wait for STX
			while (in_.get() != STX)
				time::yield();

			// 2. Read each character until ETX
			in_.getline(data, size, ETX);
		}

	private:
		static constexpr char STX = 0x02;
		static constexpr char ETX = 0x03;

		UART& uart_;
		streams::istream in_;
		streams::istreambuf& buf_;
	};

	/**
	 * Support for seeedstudio Grove 125KHz RFID Reader in Wiegand mode.
	 * This implementation needs 2 EXT pins from the MCU. If your design does
	 * not allow this, then you may use `Grove125KHzRFIDReaderWiegandPCI` instead.
	 * 
	 * Pinout:
	 * - red cable:		+5V
	 * - black cable:	GND
	 * - white cable:	DATA0
	 * - yellow cable:	DATA1
	 * 
	 * In Wiegand mode, the device provides only 3 bytes from a RFID tag.
	 * From experiment, these 3 bytes are the 3rd to 5th bytes from the tag,
	 * as read from the Grove device in UART mode.
	 * 
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_GROVE_RFID_READER_INT_ISR()` macro.
	 * 
	 * @note Wiegand mode is activated with a jumper on the device.
	 * 
	 * @tparam DATA0_ the EXT pin used to connect to Grove device DATA0 (white) wire.
	 * @tparam DATA1_ the EXT pin used to connect to Grove device DATA1 (yellow) wire.
	 * 
	 * @sa Grove125KHzRFIDReaderWiegandPCI
	 * @sa REGISTER_GROVE_RFID_READER_INT_ISR()
	 */
	template<board::ExternalInterruptPin DATA0_, board::ExternalInterruptPin DATA1_>
	class Grove125KHzRFIDReaderWiegandEXT
	{
		static constexpr board::ExternalInterruptPin DATA0 = DATA0_;
		static constexpr board::ExternalInterruptPin DATA1 = DATA1_;
		static_assert(DATA0 != DATA1, "DATA0 and DATA1 must be two distinct pins");

	public:
		/** 
		 * The data type used to return data read from the Grove device.
		 * This may be larger than what is actually needed.
		 */
		using DATA_TYPE = typename protocols::Wiegand::DATA_TYPE;

		/** 
		 * The actual number of bits of data in `DATA_TYPE`. This does not include
		 * parity bits which do not hold any access control data.
		 * The actual bits are always the LSB in `DATA_TYPE`, other bits (HSB)
		 * are set to `0`.
		 */
		static constexpr uint8_t DATA_BITS = protocols::Wiegand::DATA_BITS;

		/**
		 * Construct a new Grove 125KHz RFID Reader instance in Wiegand mode,
		 * where wires are connected to `board::ExternalInterruptPin` pins.
		 */
		Grove125KHzRFIDReaderWiegandEXT()
		{
			interrupt::register_handler(*this);
		}

		/// @cond notdocumented
		Grove125KHzRFIDReaderWiegandEXT(const Grove125KHzRFIDReaderWiegandEXT&) = delete;
		Grove125KHzRFIDReaderWiegandEXT& operator=(const Grove125KHzRFIDReaderWiegandEXT&) = delete;
		/// @endcond

		/**
		 * Start operations of the device. From here, all data received from the Grove
		 * device will be read and stored.
		 * @sa end()
		 */
		void begin()
		{
			enabler0_.enable();
			enabler1_.enable();
		}

		/**
		 * Stop operations of the device. From here, any data received from the Grove
		 * device will be lost.
		 * @sa begin()
		 */
		void end()
		{
			enabler0_.disable();
			enabler1_.disable();
		}

		/**
		 * Check if data from Grove device is ready to read and valid (correct parity).
		 * In this situation, your program can call `get_data()` to receive the 
		 * complete access data (read from an RFID tag).
		 * 
		 * @sa get_data()
		 */
		bool has_data()
		{
			synchronized return wiegand_.available_() && wiegand_.valid_();
		}

		/**
		 * Get complete data as a `DATA_TYPE` value (integral type) from the Grove device.
		 * In Wiegand mode only 3 bytes are available from the tag read by Grove device.
		 * 
		 * @note this method will clear data read from Grove device and will allow for 
		 * starting to read a new RFID tag. This means 2 consecutive calls will not 
		 * return the same value (except if the same tag is used 2 times on the device).
		 * 
		 * @param data reference to be fed from device data; or `0` if no data was read
		 * from the device (no tag present since last call to `get_data()`).
		 * 
		 * @sa has_data()
		 */
		void get_data(DATA_TYPE& data)
		{
			if (has_data())
			{
				data = wiegand_.get_data_();
				wiegand_.reset();
			}
			else
				data = DATA_TYPE{};
		}

	private:
		void fall_0()
		{
			wiegand_.on_falling_data0();
		}

		void fall_1()
		{
			wiegand_.on_falling_data1();
		}

		gpio::FAST_EXT_PIN<DATA0> data0_{gpio::PinMode::INPUT_PULLUP};
		gpio::FAST_EXT_PIN<DATA1> data1_{gpio::PinMode::INPUT_PULLUP};
		interrupt::INTSignal<DATA0> enabler0_{interrupt::InterruptTrigger::FALLING_EDGE};
		interrupt::INTSignal<DATA1> enabler1_{interrupt::InterruptTrigger::FALLING_EDGE};
		protocols::Wiegand wiegand_;

		friend struct isr_handler_grove;
	};

	/**
	 * Support for seeedstudio Grove 125KHz RFID Reader in Wiegand mode.
	 * This implementation needs 2 PCI pins from the MCU. If your design allows,
	 * you may better use `Grove125KHzRFIDReaderWiegandEXT` instead.
	 * 
	 * Pinout:
	 * - red cable:		+5V
	 * - black cable:	GND
	 * - white cable:	DATA0
	 * - yellow cable:	DATA1
	 * 
	 * In Wiegand mode, the device provides only 3 bytes from a RFID tag.
	 * From experiment, these 3 bytes are the 3rd to 5th bytes from the tag,
	 * as read from the Grove device in UART mode.
	 * 
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_GROVE_RFID_READER_PCI_ISR()` macro.
	 * 
	 * @warning Both PCI pins must be on the same Port.
	 * 
	 * @note Wiegand mode is activated with a jumper on the device.
	 * 
	 * @tparam DATA0_ the PCI pin used to connect to Grove device DATA0 (white) wire.
	 * @tparam DATA1_ the PCI pin used to connect to Grove device DATA1 (yellow) wire.
	 * 
	 * @sa Grove125KHzRFIDReaderWiegandEXT
	 * @sa REGISTER_GROVE_RFID_READER_PCI_ISR()
	 */
	template<board::InterruptPin DATA0_, board::InterruptPin DATA1_>
	class Grove125KHzRFIDReaderWiegandPCI
	{
		static constexpr board::InterruptPin DATA0 = DATA0_;
		static constexpr board::InterruptPin DATA1 = DATA1_;

		static constexpr board::DigitalPin DATA0_PIN = board::PCI_PIN<DATA0>();
		using DATA0_TRAIT = board_traits::DigitalPin_trait<DATA0_PIN>;
		static constexpr board::DigitalPin DATA1_PIN = board::PCI_PIN<DATA1>();
		using DATA1_TRAIT = board_traits::DigitalPin_trait<DATA1_PIN>;
		using DATA_SIGNAL = interrupt::PCI_SIGNAL<DATA0>;

		static_assert(DATA0 != DATA1, "DATA0 and DATA1 must be two distinct pins");
		static_assert(DATA0_TRAIT::PORT == DATA1_TRAIT::PORT, "DATA0 and DATA1 must be on the same port");

	public:
		/** 
		 * The data type used to return data read from the Grove device.
		 * This may be larger than what is actually needed.
		 */
		using DATA_TYPE = typename protocols::Wiegand::DATA_TYPE;

		/** 
		 * The actual number of bits of data in `DATA_TYPE`. This does not include
		 * parity bits which do not hold any access control data.
		 * The actual bits are always the LSB in `DATA_TYPE`, other bits (HSB)
		 * are set to `0`.
		 */
		static constexpr uint8_t DATA_BITS = protocols::Wiegand::DATA_BITS;

		/**
		 * Construct a new Grove 125KHz RFID Reader instance in Wiegand mode,
		 * where wires are connected to `board::InterruptPin` pins.
		 */
		Grove125KHzRFIDReaderWiegandPCI()
		{
			interrupt::register_handler(*this);
			enabler_.set_enable_pins(PCI_MASK);
		}

		/// @cond notdocumented
		Grove125KHzRFIDReaderWiegandPCI(const Grove125KHzRFIDReaderWiegandPCI&) = delete;
		Grove125KHzRFIDReaderWiegandPCI& operator=(const Grove125KHzRFIDReaderWiegandPCI&) = delete;
		/// @endcond

		/**
		 * Start operations of the device. From here, all data received from the Grove
		 * device will be read and stored.
		 * @sa end()
		 */
		void begin()
		{
			enabler_.enable();
		}

		/**
		 * Stop operations of the device. From here, any data received from the Grove
		 * device will be lost.
		 * @sa begin()
		 */
		void end()
		{
			enabler_.disable();
		}

		/**
		 * Check if data from Grove device is ready to read and valid (correct parity).
		 * In this situation, your program can call `get_data()` to receive the 
		 * complete access data (read from an RFID tag).
		 * 
		 * @sa get_data()
		 */
		bool has_data()
		{
			synchronized return wiegand_.available_() && wiegand_.valid_();
		}

		/**
		 * Get complete data as a `DATA_TYPE` value (integral type) from the Grove device.
		 * In Wiegand mode only 3 bytes are available from the tag read by Grove device.
		 * 
		 * @note this method will clear data read from Grove device and will allow for 
		 * starting to read a new RFID tag. This means 2 consecutive calls will not 
		 * return the same value (except if the same tag is used 2 times on the device).
		 * 
		 * @param data reference to be fed from device data; or `0` if no data was read
		 * from the device (no tag present since last call to `get_data()`).
		 * 
		 * @sa has_data()
		 */
		void get_data(DATA_TYPE& data)
		{
			if (has_data())
			{
				data = wiegand_.get_data_();
				wiegand_.reset();
			}
			else
				data = DATA_TYPE{};
		}

	private:
		void fall_0_or_1()
		{
			if (!data0_.value())
				wiegand_.on_falling_data0();
			else if (!data1_.value())
				wiegand_.on_falling_data1();
		}

		static constexpr uint8_t PCI_MASK = bits::BV8(DATA0_TRAIT::BIT, DATA1_TRAIT::BIT);

		gpio::FAST_PIN<DATA0_PIN> data0_{gpio::PinMode::INPUT_PULLUP};
		gpio::FAST_PIN<DATA1_PIN> data1_{gpio::PinMode::INPUT_PULLUP};
		DATA_SIGNAL enabler_;
		protocols::Wiegand wiegand_;

		friend struct isr_handler_grove;
	};

	/// @cond notdocumented
	// Traits for Grove RFID Readers
	template<typename READER> struct Grove125KHzRFIDReader_trait
	{
		static constexpr bool IS_GROVE_125_READER = false;
		static constexpr bool IS_UART_MODE = false;
		static constexpr bool IS_WIEGAND_MODE = false;
		static constexpr bool IS_PCI = false;
		static constexpr bool IS_EXT = false;
		static constexpr bool USES_DISTINCT_PINS = false;
	};
	template<typename UART> struct Grove125KHzRFIDReader_trait<Grove125KHzRFIDReaderUART<UART>>
	{
		static constexpr bool IS_GROVE_125_READER = true;
		static constexpr bool IS_UART_MODE = true;
		static constexpr bool IS_WIEGAND_MODE = false;
		static constexpr bool IS_PCI = false;
		static constexpr bool IS_EXT = false;
		static constexpr bool USES_DISTINCT_PINS = false;
	};
	template<board::ExternalInterruptPin DATA0, board::ExternalInterruptPin DATA1>
	struct Grove125KHzRFIDReader_trait<Grove125KHzRFIDReaderWiegandEXT<DATA0, DATA1>>
	{
		static constexpr bool IS_GROVE_125_READER = true;
		static constexpr bool IS_UART_MODE = false;
		static constexpr bool IS_WIEGAND_MODE = true;
		static constexpr bool IS_PCI = false;
		static constexpr bool IS_EXT = true;
		static constexpr bool USES_DISTINCT_PINS = (DATA0 != DATA1);
	};
	template<board::InterruptPin DATA0, board::InterruptPin DATA1>
	struct Grove125KHzRFIDReader_trait<Grove125KHzRFIDReaderWiegandPCI<DATA0, DATA1>>
	{
		static constexpr bool IS_GROVE_125_READER = true;
		static constexpr bool IS_UART_MODE = false;
		static constexpr bool IS_WIEGAND_MODE = true;
		static constexpr bool IS_PCI = true;
		static constexpr bool IS_EXT = false;
		static constexpr bool USES_DISTINCT_PINS = (DATA0 != DATA1);
	};
	/// @endcond

	/// @cond notdocumented
	struct isr_handler_grove
	{
		template<uint8_t DATA0_NUM, uint8_t DATA1_NUM, typename READER> 
		static void callback_fall_0()
		{
			// Check READER is a proper type
			using GROVE_TRAIT = Grove125KHzRFIDReader_trait<READER>;
			static_assert(GROVE_TRAIT::IS_GROVE_125_READER, "READER must be a Grove125KHzRFIDReaderWiegandEXT type");
			static_assert(GROVE_TRAIT::IS_WIEGAND_MODE, "READER must be a Grove125KHzRFIDReaderWiegandEXT type");
			static_assert(GROVE_TRAIT::IS_EXT, "READER must be a Grove125KHzRFIDReaderWiegandEXT type");
			// Check pins are compliant
			interrupt::isr_handler_int::check_int_pin<DATA0_NUM, READER::DATA0>();
			interrupt::isr_handler_int::check_int_pin<DATA1_NUM, READER::DATA1>();
			static_assert(DATA0_NUM != DATA1_NUM, "DATA0 and DATA1 must be two distinct pins");
			// Call Grove RFID handler
			interrupt::CallbackHandler<void (READER::*)(), &READER::fall_0>::call();
		}

		template<uint8_t DATA0_NUM, uint8_t DATA1_NUM, typename READER> 
		static void callback_fall_1()
		{
			// Check READER is a proper type
			using GROVE_TRAIT = Grove125KHzRFIDReader_trait<READER>;
			static_assert(GROVE_TRAIT::IS_GROVE_125_READER, "READER must be a Grove125KHzRFIDReaderWiegandEXT type");
			static_assert(GROVE_TRAIT::IS_WIEGAND_MODE, "READER must be a Grove125KHzRFIDReaderWiegandEXT type");
			static_assert(GROVE_TRAIT::IS_EXT, "READER must be a Grove125KHzRFIDReaderWiegandEXT type");
			// Check pins are compliant
			interrupt::isr_handler_int::check_int_pin<DATA0_NUM, READER::DATA0>();
			interrupt::isr_handler_int::check_int_pin<DATA1_NUM, READER::DATA1>();
			static_assert(DATA0_NUM != DATA1_NUM, "DATA0 and DATA1 must be two distinct pins");
			// Call Grove RFID handler
			interrupt::CallbackHandler<void (READER::*)(), &READER::fall_1>::call();
		}

		template<uint8_t DATA01_NUM, typename READER> 
		static void callback_fall_0_or_1()
		{
			// Check READER is a proper type
			using GROVE_TRAIT = Grove125KHzRFIDReader_trait<READER>;
			static_assert(GROVE_TRAIT::IS_GROVE_125_READER, "READER must be a Grove125KHzRFIDReaderWiegandPCI type");
			static_assert(GROVE_TRAIT::IS_WIEGAND_MODE, "READER must be a Grove125KHzRFIDReaderWiegandPCI type");
			static_assert(GROVE_TRAIT::IS_PCI, "READER must be a Grove125KHzRFIDReaderWiegandPCI type");
			// Check pins are compliant
			static_assert(GROVE_TRAIT::USES_DISTINCT_PINS, "DATA0 and DATA1 must be two distinct pins");
			interrupt::isr_handler_pci::check_pci_pins<DATA01_NUM, READER::DATA0, READER::DATA1>();
			// Call Grove RFID handler
			interrupt::CallbackHandler<void (READER::*)(), &READER::fall_0_or_1>::call();
		}
	};
	/// @endcond
}

#endif /* GROVE_RFID_READER_HH */
/// @endcond
