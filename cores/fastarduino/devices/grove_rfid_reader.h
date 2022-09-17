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

#include "../gpio.h"
#include "../int.h"
#include "../interrupts.h"
#include "../pci.h"
#include "../streambuf.h"
#include "../streams.h"
#include "../time.h"
#include "../uart_commons.h"
#include "wiegand.h"

#define REGISTER_GROVE_RFID_READER_INT(DATA0_INT, DATA0, DATA1_INT, DATA1)							\
	ISR(CAT3(INT, DATA0_INT, _vect))																\
	{																								\
		devices::rfid::isr_handler_grove::callback_fall_0<DATA0_INT, DATA0, DATA1_INT, DATA1>();	\
	}																								\
	ISR(CAT3(INT, DATA1_INT, _vect))																\
	{																								\
		devices::rfid::isr_handler_grove::callback_fall_1<DATA0_INT, DATA0, DATA1_INT, DATA1>();	\
	}

//TODO macros to register Wiegand reader with PCI pins

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

	//TODO Wiegand communication mode with either 2 INT or 2 PCI pins
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
	template<board::ExternalInterruptPin DATA0, board::ExternalInterruptPin DATA1>
	class Grove125KHzRFIDReaderWiegand
	{
	public:
		using DATA_TYPE = typename protocols::Wiegand::DATA_TYPE;
		static constexpr uint8_t DATA_BITS = protocols::Wiegand::DATA_BITS;
		static constexpr DATA_TYPE DATA_MASK = protocols::Wiegand::DATA_MASK;

		Grove125KHzRFIDReaderWiegand()
		{
			interrupt::register_handler(*this);
		}

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
			return wiegand_.available() && wiegand_.valid();
		}

		void get_data(DATA_TYPE& data)
		{
			if (has_data())
			{
				data = wiegand_.get_data();
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

	/// @cond notdocumented
	struct isr_handler_grove
	{
		template<uint8_t INT_NUM_, board::ExternalInterruptPin INT_PIN_> static void check_int_pin()
		{
			static_assert(board_traits::ExternalInterruptPin_trait<INT_PIN_>::INT == INT_NUM_,
						  "PIN INT number must match INT_NUM");
		}

		template<uint8_t DATA0_NUM, board::ExternalInterruptPin DATA0_PIN, 
			uint8_t DATA1_NUM, board::ExternalInterruptPin DATA1_PIN> 
		static void callback_fall_0()
		{
			// Check pins are compliant
			check_int_pin<DATA0_NUM, DATA0_PIN>();
			check_int_pin<DATA1_NUM, DATA1_PIN>();
			static_assert(DATA0_NUM != DATA1_NUM, "DATA0 and DATA1 must be two distinct pins");
			// Call Grove RFID handler
			using GROVE = Grove125KHzRFIDReaderWiegand<DATA0_PIN, DATA1_PIN>;
			interrupt::CallbackHandler<void (GROVE::*)(), &GROVE::fall_0>::call();
		}

		template<uint8_t DATA0_NUM, board::ExternalInterruptPin DATA0_PIN, 
			uint8_t DATA1_NUM, board::ExternalInterruptPin DATA1_PIN> 
		static void callback_fall_1()
		{
			// Check pins are compliant
			check_int_pin<DATA0_NUM, DATA0_PIN>();
			check_int_pin<DATA1_NUM, DATA1_PIN>();
			static_assert(DATA0_NUM != DATA1_NUM, "DATA0 and DATA1 must be two distinct pins");
			// Call Grove RFID handler
			using GROVE = Grove125KHzRFIDReaderWiegand<DATA0_PIN, DATA1_PIN>;
			interrupt::CallbackHandler<void (GROVE::*)(), &GROVE::fall_1>::call();
		}
	};
	/// @endcond
}

#endif /* GROVE_RFID_READER_HH */
/// @endcond
