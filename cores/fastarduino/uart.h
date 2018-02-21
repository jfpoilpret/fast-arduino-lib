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
 * Hardware serial API.
 */
#ifndef UART_HH
#define UART_HH

#include "interrupts.h"
#include "uart_commons.h"
#include "streams.h"
#include "boards/board_traits.h"

// Only MCU with physical USART are supported (not ATtiny then)
#if defined(UCSR0A) || defined(UCSR1A)

/// @cond notdocumented
#define REGISTER_UATX_ISR_METHOD_(UART_NUM, HANDLER, CALLBACK) \
	REGISTER_ISR_METHOD_(CAT3(USART, UART_NUM, _UDRE_vect), HANDLER, CALLBACK)
#define REGISTER_UARX_ISR_METHOD_(UART_NUM, HANDLER, CALLBACK) \
	REGISTER_ISR_METHOD_(CAT3(USART, UART_NUM, _RX_vect), HANDLER, CALLBACK)

#define UATX_CLASS_(UART_NUM) CAT(serial::hard::UATX < board::USART::USART, UART_NUM) >
#define UARX_CLASS_(UART_NUM) CAT(serial::hard::UARX < board::USART::USART, UART_NUM) >
#define UART_CLASS_(UART_NUM) CAT(serial::hard::UART < board::USART::USART, UART_NUM) >
/// @endcond

/**
 * Register the necessary ISR (interrupt Service Routine) for an serial::hard::UATX
 * to work correctly.
 * @param UART_NUM the number of the USART feature for the target MCU
 */
#define REGISTER_UATX_ISR(UART_NUM) \
	REGISTER_UATX_ISR_METHOD_(UART_NUM, UATX_CLASS_(UART_NUM), &UATX_CLASS_(UART_NUM)::data_register_empty)

/**
 * Register the necessary ISR (interrupt Service Routine) for an serial::hard::UARX
 * to work correctly.
 * @param UART_NUM the number of the USART feature for the target MCU
 */
#define REGISTER_UARX_ISR(UART_NUM) \
	REGISTER_UARX_ISR_METHOD_(UART_NUM, UARX_CLASS_(UART_NUM), &UARX_CLASS_(UART_NUM)::data_receive_complete)

/**
 * Register the necessary ISR (interrupt Service Routine) for an serial::hard::UART
 * to work correctly.
 * @param UART_NUM the number of the USART feature for the target MCU
 */
#define REGISTER_UART_ISR(UART_NUM)                                                                         \
	REGISTER_UATX_ISR_METHOD_(UART_NUM, UART_CLASS_(UART_NUM), &UART_CLASS_(UART_NUM)::data_register_empty) \
	REGISTER_UARX_ISR_METHOD_(UART_NUM, UART_CLASS_(UART_NUM), &UART_CLASS_(UART_NUM)::data_receive_complete)

namespace serial
{
	/**
	 * Defines API types used by hardware UART features.
	 * Note this API is only available to MCU that have hardware UART, such as all
	 * ATmega, but not other MCU, like ATtiny.
	 */
	namespace hard
	{
		//TODO Handle generic errors coming from UART TX (which errors?) in addition to internal overflow
		/// @cond notdocumented
		class AbstractUART : public UARTErrors
		{
		protected:
			struct SpeedSetup
			{
				constexpr SpeedSetup(uint16_t ubrr_value, bool u2x) : ubrr_value{ubrr_value}, u2x{u2x}
				{
				}
				const uint16_t ubrr_value;
				const bool u2x;
			};
			static constexpr SpeedSetup compute_speed(uint32_t rate)
			{
				return (UBRR_double(rate) < DOUBLE_SPEED_RATE_LIMIT) ? SpeedSetup(UBRR_double(rate), true) :
																	   SpeedSetup(UBRR_single(rate), false);
			}

		private:
			static constexpr const uint16_t DOUBLE_SPEED_RATE_LIMIT = 4096;

			static constexpr uint16_t UBRR_double(uint32_t rate)
			{
				return (F_CPU / 4 / rate - 1) / 2;
			}
			static constexpr uint16_t UBRR_single(uint32_t rate)
			{
				return (F_CPU / 8 / rate - 1) / 2;
			}
		};
		/// @endcond

		/**
		 * Hardware serial transmitter API.
		 * For this API to be fully functional, you must register the right ISR in your
		 * program, through `REGISTER_UATX_ISR()`, then call `register_handler()`
		 * immediately after the UATX instance has been constructed.
		 * 
		 * @tparam USART_ the hardware `board::USART` to use
		 * @sa REGISTER_UATX_ISR()
		 */
		template<board::USART USART_> class UATX : virtual public AbstractUART, private streams::ostreambuf
		{
		public:
			/** The hardware `board::USART` used by this UATX. */
			static constexpr const board::USART USART = USART_;

		private:
			using TRAIT = board_traits::USART_trait<USART>;

		public:
			/**
			 * Construct a new hardware serial transmitter and provide it with a
			 * buffer for interrupt-based transmission.
			 * @param output an array of characters used by this transmitter to
			 * buffer output during transmission so that write methods are not
			 * blocking.
			 */
			template<uint8_t SIZE_TX>
			UATX(char (&output)[SIZE_TX]) : streams::ostreambuf{output}, transmitting_{false}
			{
			}

			/**
			 * Register this transmitter with the matching ISR that should have been
			 * registered with REGISTER_UATX_ISR().
			 * @sa REGISTER_UATX_ISR()
			 */
			inline void register_handler()
			{
				interrupt::register_handler(*this);
			}

			/**
			 * Enable the transmitter. 
			 * This is needed before any transmission can take place.
			 * Once called, it is possible to push content to `out()`,
			 * which will be then transmitted though the serial connection.
			 * 
			 * @param rate the transmission rate in bits per second (bps)
			 * @param parity the kind of parity check used by transmission
			 * @param stop_bits the number of stop bits used by transmission
			 */
			inline void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
			{
				AbstractUART::SpeedSetup setup = AbstractUART::compute_speed(rate);
				synchronized
				{
					TRAIT::UBRR = setup.ubrr_value;
					TRAIT::UCSRA = (setup.u2x ? TRAIT::U2X_MASK : 0);
					TRAIT::UCSRB = TRAIT::TX_ENABLE_MASK | TRAIT::UDRIE_MASK;
					TRAIT::UCSRC = TRAIT::UCSRC_value(parity, stop_bits);
				}
			}

			/**
			 * Stop all transmissions.
			 * Once called, it is possible to re-enable transmission again by
			 * calling `begin()`.
			 */
			inline void end()
			{
				synchronized TRAIT::UCSRB = 0;
			}

			/**
			 * Get the formatted output stream used to send content through this serial
			 * transmitter.
			 */
			inline streams::ostream out()
			{
				return streams::ostream(*this);
			}

		protected:
			/// @cond notdocumented
			//TODO should be private
			inline void data_register_empty()
			{
				errors_.has_errors = 0;
				char value;
				if (queue().pull_(value))
					TRAIT::UDR = value;
				else
				{
					transmitting_ = false;
					// Clear UDRIE to prevent UDR interrupt to go on forever
					TRAIT::UCSRB &= ~TRAIT::UDRIE_MASK;
				}
			}
			/// @endcond

		protected:
			/// @cond notdocumented
			// Listeners of events on the buffer
			virtual void on_put() override
			{
				errors_.queue_overflow = ostreambuf::overflow();
				synchronized
				{
					// Check if TX is not currently active, if so, activate it
					if (!transmitting_)
					{
						// Yes, trigger TX
						char value;
						if (queue().pull_(value))
						{
							// Set UDR interrupt to be notified when we can send the next character
							TRAIT::UCSRB |= TRAIT::UDRIE_MASK;
							TRAIT::UDR = value;
							transmitting_ = true;
						}
					}
				}
			}
			/// @endcond

		private:
			bool transmitting_;
			DECL_UDRE_ISR_FRIENDS
		};

		/**
		 * Hardware serial receiver API.
		 * For this API to be fully functional, you must register the right ISR in your
		 * program, through `REGISTER_UARX_ISR()`, then call `register_handler()`
		 * immediately after the UARX instance has been constructed.
		 * 
		 * @tparam USART_ the hardware `board::USART` to use
		 * @sa REGISTER_UARX_ISR()
		 */
		template<board::USART USART_> class UARX : virtual public AbstractUART, private streams::istreambuf
		{
		public:
			/** The hardware `board::USART` used by this UARX. */
			static constexpr const board::USART USART = USART_;

		private:
			using TRAIT = board_traits::USART_trait<USART>;

		public:
			/**
			 * Construct a new hardware serial receiver and provide it with a
			 * buffer for interrupt-based reception.
			 * Reception is asynchronous.
			 * @param input an array of characters used by this receiver to
			 * store content received through serial line, buffered until read through
			 * `in()`.
			 */
			template<uint8_t SIZE_RX> UARX(char (&input)[SIZE_RX]) : istreambuf{input}
			{
			}

			/**
			 * Register this receiver with the matching ISR that should have been
			 * registered with REGISTER_UARX_ISR().
			 * @sa REGISTER_UARX_ISR()
			 */
			inline void register_handler()
			{
				interrupt::register_handler(*this);
			}

			/**
			 * Enable the receiver. 
			 * This is needed before any reception can take place.
			 * Once called, it is possible to read content, received through serial
			 * connection, by using `in()`.
			 * 
			 * @param rate the transmission rate in bits per second (bps)
			 * @param parity the kind of parity check used by transmission
			 * @param stop_bits the number of stop bits used by transmission
			 */
			inline void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
			{
				AbstractUART::SpeedSetup setup = AbstractUART::compute_speed(rate);
				synchronized
				{
					TRAIT::UBRR = setup.ubrr_value;
					TRAIT::UCSRA = (setup.u2x ? TRAIT::U2X_MASK : 0);
					TRAIT::UCSRB = TRAIT::TX_ENABLE_MASK | TRAIT::RXCIE_MASK;
					TRAIT::UCSRC = TRAIT::UCSRC_value(parity, stop_bits);
				}
			}

			/**
			 * Stop reception.
			 * Once called, it is possible to re-enable reception again by
			 * calling `begin()`.
			 */
			inline void end()
			{
				synchronized TRAIT::UCSRB = 0;
			}

			/**
			 * Get the formatted input stream used to read content received through
			 * this serial transmitter.
			 */
			inline streams::istream in()
			{
				return streams::istream(*this);
			}

		protected:
			/// @cond notdocumented
			//TODO should be private
			inline void data_receive_complete()
			{
				char status = TRAIT::UCSRA;
				errors_.data_overrun = status & TRAIT::DOR_MASK;
				errors_.frame_error = status & TRAIT::FE_MASK;
				errors_.parity_error = status & TRAIT::UPE_MASK;
				char value = TRAIT::UDR;
				errors_.queue_overflow = !queue().push_(value);
			}
			/// @endcond

			DECL_RX_ISR_FRIENDS
		};

		/**
		 * Hardware serial receiver/transceiver API.
		 * For this API to be fully functional, you must register the right ISR in your
		 * program, through `REGISTER_UART_ISR()`, then call `register_handler()`
		 * immediately after the UART instance has been constructed.
		 * 
		 * @tparam USART_ the hardware `board::USART` to use
		 * @sa REGISTER_UART_ISR()
		 */
		template<board::USART USART_> class UART : public UARX<USART_>, public UATX<USART_>
		{
		public:
			/** The hardware `board::USART` used by this UART. */
			static constexpr const board::USART USART = USART_;

		private:
			using TRAIT = board_traits::USART_trait<USART>;

		public:
			/**
			 * Construct a new hardware serial receiver/transceiver and provide it 
			 * with 2 buffers, one for interrupt-based reception, one for 
			 * interrupt-based transmission.
			 * 
			 * @param input an array of characters used by this receiver to
			 * store content received through serial line, buffered until read through
			 * `in()`.
			 * @param output an array of characters used by this transmitter to
			 * buffer output during transmission so that write methods are not
			 * blocking.
			 */
			template<uint8_t SIZE_RX, uint8_t SIZE_TX>
			UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX]) : UARX<USART>{input}, UATX<USART>{output}
			{
			}

			/**
			 * Register this receiver/transmitter with the matching ISR that should 
			 * have been registered with REGISTER_UART_ISR().
			 * @sa REGISTER_UART_ISR()
			 */
			inline void register_handler()
			{
				interrupt::register_handler(*this);
			}

			/**
			 * Enable the receiver/transceiver. 
			 * This is needed before any transmission or reception can take place.
			 * Once called, it is possible to send and receive content through serial
			 * connection, by using `in()` for reading and `out()` for writing.
			 * 
			 * @param rate the transmission rate in bits per second (bps)
			 * @param parity the kind of parity check used by transmission
			 * @param stop_bits the number of stop bits used by transmission
			 */
			inline void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
			{
				AbstractUART::SpeedSetup setup = AbstractUART::compute_speed(rate);
				synchronized
				{
					TRAIT::UBRR = setup.ubrr_value;
					TRAIT::UCSRA = (setup.u2x ? TRAIT::U2X_MASK : 0);
					TRAIT::UCSRB =
						TRAIT::TX_ENABLE_MASK | TRAIT::RX_ENABLE_MASK | TRAIT::UDRIE_MASK | TRAIT::RXCIE_MASK;
					TRAIT::UCSRC = TRAIT::UCSRC_value(parity, stop_bits);
				}
			}

			/**
			 * Stop all transmissions and receptions.
			 * Once called, it is possible to re-enable transmission and reception 
			 * again by calling `begin()`.
			 */
			inline void end()
			{
				synchronized TRAIT::UCSRB = 0;
			}

		private:
			// Workaround trick to make REGISTER_UART_ISR work properly
			inline void data_register_empty()
			{
				UATX<USART>::data_register_empty();
			}
			inline void data_receive_complete()
			{
				UARX<USART>::data_receive_complete();
			}
			/// @endcond

			DECL_UDRE_ISR_FRIENDS
			DECL_RX_ISR_FRIENDS
		};
	}
}

#endif /* UCSR0A */
#endif /* UART_HH */
/// @endcond
