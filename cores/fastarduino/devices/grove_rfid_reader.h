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

#define REGISTER_GROVE_RFID_READER_INT_ISR(DATA0_INT, DATA1_INT, READER)					\
	ISR(CAT3(INT, DATA0_INT, _vect))														\
	{																						\
		devices::rfid::isr_handler_grove::callback_fall_0<DATA0_INT, DATA1_INT, READER>();	\
	}																						\
	ISR(CAT3(INT, DATA1_INT, _vect))														\
	{																						\
		devices::rfid::isr_handler_grove::callback_fall_1<DATA0_INT, DATA1_INT, READER>();	\
	}

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
	 * Support for seeedstudio Grove 125KHz RFID Reader in UART mode.
	 * 
	 * Pinout:
	 * - red cable:		+5V
	 * - black cable:	GND
	 * - white cable:	TX (probably not needed)
	 * - yellow cable:	RX
	 * 
	 * @note It is unclear, from original documentation and code exampels, 
	 * whether this module can write tags or not. Hence we consider writing is
	 * not possible and hence white cable is also useless to connect in UART mode.
	 */
	template<typename UART_> class Grove125KHzRFIDReaderUART
	{
		static constexpr uint16_t UART_SPEED = 9600;

		// Check that UART type is suitable
		static_assert(serial::UART_trait<UART_>::IS_UART, "UART template argument must be a serial device");
		static_assert(serial::UART_trait<UART_>::HAS_RX, "UART template argument must be a serial device with RX mode");

	public:
		using UART = UART_;

		Grove125KHzRFIDReaderUART(UART& uart) : uart_{uart}, in_{uart_.in()}, buf_{in_.rdbuf()} {}

		/// @cond notdocumented
		Grove125KHzRFIDReaderUART(const Grove125KHzRFIDReaderUART&) = delete;
		Grove125KHzRFIDReaderUART& operator=(const Grove125KHzRFIDReaderUART&) = delete;
		/// @endcond
		
		void begin()
		{
			// Just start UART with proper settings: 9600bps, 8 bits, 1 stop, no parity
			uart_.begin(UART_SPEED, serial::Parity::NONE, serial::StopBits::ONE);
		}

		void end()
		{
			// Stop UART
			uart_.end(serial::BufferHandling::CLEAR);
		}

		bool has_data()
		{
			return buf_.sgetc() != streams::istreambuf::EOF;
		}

		// Example sample read through serial <0x02> 0 F 0 0 2 4 A D C 4 4 2 <0x03>
		// STX / ... / ETX
		//TODO It is not clear if this device can work with tags with more than 48 bits (inc. checksum)
		//TODO better to use template<SIZE> (safer)?
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
	 * 
	 * Pinout:
	 * - red cable:		+5V
	 * - black cable:	GND
	 * - white cable:	DATA0
	 * - yellow cable:	DATA1
	 * 
	 * In Wiegand mode, the device provides only 3 bytes from a RFID tag.
	 * From experiment, these 3 bytes are the 3rd to 5th bytes from the tag.
	 * 
	 * @note Wiegand mode is activated with a jumper on the device.
	 */
	template<board::ExternalInterruptPin DATA0_, board::ExternalInterruptPin DATA1_>
	class Grove125KHzRFIDReaderWiegandEXT
	{
		static constexpr board::ExternalInterruptPin DATA0 = DATA0_;
		static constexpr board::ExternalInterruptPin DATA1 = DATA1_;
		static_assert(DATA0 != DATA1, "DATA0 and DATA1 must be two distinct pins");

	public:
		using DATA_TYPE = typename protocols::Wiegand::DATA_TYPE;
		static constexpr uint8_t DATA_BITS = protocols::Wiegand::DATA_BITS;

		Grove125KHzRFIDReaderWiegandEXT()
		{
			interrupt::register_handler(*this);
		}
		Grove125KHzRFIDReaderWiegandEXT(const Grove125KHzRFIDReaderWiegandEXT&) = delete;
		Grove125KHzRFIDReaderWiegandEXT& operator=(const Grove125KHzRFIDReaderWiegandEXT&) = delete;

		void begin()
		{
			enabler0_.enable();
			enabler1_.enable();
		}

		void end()
		{
			enabler0_.disable();
			enabler1_.disable();
		}

		bool has_data()
		{
			synchronized return wiegand_.available_() && wiegand_.valid_();
		}

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
		using DATA_TYPE = typename protocols::Wiegand::DATA_TYPE;
		static constexpr uint8_t DATA_BITS = protocols::Wiegand::DATA_BITS;

		Grove125KHzRFIDReaderWiegandPCI()
		{
			interrupt::register_handler(*this);
			enabler_.set_enable_pins(PCI_MASK);
		}
		Grove125KHzRFIDReaderWiegandPCI(const Grove125KHzRFIDReaderWiegandPCI&) = delete;
		Grove125KHzRFIDReaderWiegandPCI& operator=(const Grove125KHzRFIDReaderWiegandPCI&) = delete;

		void begin()
		{
			enabler_.enable();
		}

		void end()
		{
			enabler_.disable();
		}

		bool has_data()
		{
			synchronized return wiegand_.available_() && wiegand_.valid_();
		}

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
