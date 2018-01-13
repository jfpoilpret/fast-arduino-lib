//   Copyright 2016-2018 Jean-Francois Poilpret
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
 * Defines API types used by hardware and software UART features.
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

	/// @cond notdocumented
	//TODO Do we really need this public union here? Cam't it be private inside UARTErrors class?
	union UARTErrors_
	{
		uint8_t has_errors;
		struct
		{
			bool frame_error : 1;
			bool data_overrun : 1;
			bool queue_overflow : 1;
			bool parity_error : 1;
		} all_errors;
	};
	/// @endcond

	/**
	 * Holder of latest UART errors.
	 */
	class UARTErrors
	{
	public:
		UARTErrors()
		{
			clear_errors();
		}

		/**
		 * Reset UART errors to no error.
		 */
		inline void clear_errors()
		{
			errors_.has_errors = 0;
		}

		/**
		 * Indicate if there are UART errors pending.
		 * @retval true if some errors are pending; other methods will indicate
		 * the exact error(s).
		 * @retval false if no error is pending
		 */
		inline uint8_t has_errors() const
		{
			return errors_.has_errors;
		}

		/**
		 * Indicate if a frame error has occurred.
		 * @retval true if a frame error has occurred
		 * @retval false if no frame error has occurred
		 */
		inline bool frame_error() const
		{
			return errors_.all_errors.frame_error;
		}

		/**
		 * Indicate if a data overrun has occurred.
		 * @retval true if a data overrun has occurred
		 * @retval false if no data overrun has occurred
		 */
		inline bool data_overrun() const
		{
			return errors_.all_errors.data_overrun;
		}

		/**
		 * Indicate if a queue overflow has occurred.
		 * @retval true if a queue overflow has occurred
		 * @retval false if no queue overflow has occurred
		 */
		inline bool queue_overflow() const
		{
			return errors_.all_errors.queue_overflow;
		}

		/**
		 * Indicate if a parity error has occurred.
		 * @retval true if a parity error has occurred
		 * @retval false if no parity error has occurred
		 */
		inline bool parity_error() const
		{
			return errors_.all_errors.parity_error;
		}

	protected:
		/// @cond notdocumented
		UARTErrors_ errors_;
		/// @endcond
	};
};

#endif /* UARTCOMMONS_HH */
/// @endcond
