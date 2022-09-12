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

#include "../streambuf.h"
#include "../streams.h"
#include "../time.h"
#include "../uart_commons.h"

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
	//TODO Implement basic API (UART-type -HW/SW- agnostic)
	// Example sample read through serial <0x02> 0 F 0 0 2 4 A D C 4 4 2 <0x03>
	// STX / ... / ETX
	template<typename UART> class Grove125KHzRFIDReaderUART
	{
		static constexpr uint16_t UART_SPEED = 9600;

		// Check that UART type is suitable
		static_assert(serial::UART_trait<UART>::IS_UART, "UART template argument must be a serial device");
		static_assert(serial::UART_trait<UART>::HAS_RX, "UART template argument must be a serial device with TX mode");

	public:
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
}

#endif /* GROVE_RFID_READER_HH */
/// @endcond
