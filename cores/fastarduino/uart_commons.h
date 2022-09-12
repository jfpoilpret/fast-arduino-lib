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
 * Common definitions for serial API.
 */
#ifndef UARTCOMMONS_HH
#define UARTCOMMONS_HH

#include <stdint.h>

/**
 * Defines all API for UART features.
 * This namespace embeds two namespaces:
 * - `hard` contains API for hardware UART (only for MCU that support it)
 * - `soft` contains API for software-emulated UART (for all MCU)
 */
namespace serial
{
	/**
	 * Parity used for serial transmission.
	 */
	enum class Parity : uint8_t
	{
		/** No parity bit */
		NONE = 0,
		/** Even parity bit */
		EVEN = 1,
		/** Odd parity bit */
		ODD = 3
	};

	/**
	 * Number of stop bits used for serial transmission.
	 */
	enum class StopBits : uint8_t
	{
		/** One stop bit */
		ONE = 1,
		/**  Two stop bits */
		TWO = 2
	};

	/**
	 * How the TX/RX buffer should be handled when ending transmission
	 * (see `end()` methods) on UATX/UARX.
	 */
	enum class BufferHandling : uint8_t
	{
		/** Stop transmission immediately, keep buffer as-is. */
		KEEP = 0x00,
		/** Stop transmission immediately, clear buffer. */
		CLEAR = 0x01,
		/**
		 * Flush buffer before stopping transmission (buffer will be empty after
		 * calling `end()`).
		 */
		FLUSH = 0x02
	};

	/// @cond notdocumented
	union Errors
	{
		Errors() = default;

		uint8_t has_errors = 0;
		struct
		{
			bool frame_error : 1;
			bool data_overrun : 1;
			bool queue_overflow : 1;
			bool parity_error : 1;
		};
	};
	/// @endcond

	/**
	 * Holder of latest UART errors. Used as public interface to check what errors 
	 * occurred lately on UATX/UARX/UART devices.
	 */
	class UARTErrors
	{
	public:
		/// @cond notdocumented
		UARTErrors() : errors_{} {}
		UARTErrors(const UARTErrors& that) = default;
		UARTErrors& operator=(const UARTErrors& that) = default;
		/// @endcond

		/**
		 * Reset UART errors to no error.
		 */
		void clear_errors()
		{
			errors_.has_errors = 0;
		}

		/**
		 * Indicate if there are UART errors pending.
		 * @retval true if some errors are pending; other methods will indicate
		 * the exact error(s).
		 * @retval false if no error is pending
		 */
		uint8_t has_errors() const
		{
			return errors_.has_errors;
		}

		/**
		 * Indicate if a frame error has occurred.
		 * @retval true if a frame error has occurred
		 * @retval false if no frame error has occurred
		 */
		bool frame_error() const
		{
			return errors_.frame_error;
		}

		/**
		 * Indicate if a data overrun has occurred.
		 * @retval true if a data overrun has occurred
		 * @retval false if no data overrun has occurred
		 */
		bool data_overrun() const
		{
			return errors_.data_overrun;
		}

		/**
		 * Indicate if a queue overflow has occurred.
		 * @retval true if a queue overflow has occurred
		 * @retval false if no queue overflow has occurred
		 */
		bool queue_overflow() const
		{
			return errors_.queue_overflow;
		}

		/**
		 * Indicate if a parity error has occurred.
		 * @retval true if a parity error has occurred
		 * @retval false if no parity error has occurred
		 */
		bool parity_error() const
		{
			return errors_.parity_error;
		}
	
	protected:
		/// @cond notdocumented
		Errors& errors()
		{
			return errors_;
		}
		/// @endcond

	private:
		Errors errors_;
	};

	/// @cond notdocumented
	// Useful traits for checking a provided type (in template) is an UART and what traits it supports
	template<typename> struct UART_trait
	{
		static constexpr bool IS_UART = false;
		static constexpr bool IS_HW_UART = false;
		static constexpr bool IS_SW_UART = false;
		static constexpr bool HAS_TX = false;
		static constexpr bool HAS_RX = false;
	};
	/// @endcond
};

#endif /* UARTCOMMONS_HH */
/// @endcond
