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

#ifndef UART_HH
#define	UART_HH

#include "uart_commons.h"
#include "streams.h"
#include "boards/board_traits.h"

// Only MCU with physical USART are supported (not ATtiny then)
#if defined(UCSR0A)

#define REGISTER_UATX_ISR_METHOD_(UART_NUM, HANDLER, CALLBACK)	\
REGISTER_ISR_METHOD_(CAT3(USART, UART_NUM, _UDRE_vect), HANDLER, CALLBACK)
#define REGISTER_UARX_ISR_METHOD_(UART_NUM, HANDLER, CALLBACK)	\
REGISTER_ISR_METHOD_(CAT3(USART, UART_NUM, _RX_vect), HANDLER, CALLBACK)

#define UATX_CLASS_(UART_NUM) CAT(serial::UATX<board::USART::USART, UART_NUM) >
#define UARX_CLASS_(UART_NUM) CAT(serial::UARX<board::USART::USART, UART_NUM) >
#define UART_CLASS_(UART_NUM) CAT(serial::UART<board::USART::USART, UART_NUM) >

#define REGISTER_UATX_ISR(UART_NUM)												\
REGISTER_UATX_ISR_METHOD_(UART_NUM, UATX_CLASS_(UART_NUM), & UATX_CLASS_(UART_NUM) ::data_register_empty)
#define REGISTER_UARX_ISR(UART_NUM)												\
REGISTER_UARX_ISR_METHOD_(UART_NUM, UARX_CLASS_(UART_NUM), & UARX_CLASS_(UART_NUM) ::data_receive_complete)
#define REGISTER_UART_ISR(UART_NUM)																			\
REGISTER_UATX_ISR_METHOD_(UART_NUM, UART_CLASS_(UART_NUM), & UART_CLASS_(UART_NUM) ::data_register_empty)	\
REGISTER_UARX_ISR_METHOD_(UART_NUM, UART_CLASS_(UART_NUM), & UART_CLASS_(UART_NUM) ::data_receive_complete)

namespace serial
{
	//TODO Handle generic errors coming from UART TX (which errors?) in addition to internal overflow
	class AbstractUART: public UARTErrors
	{
	protected:
		struct SpeedSetup
		{
			constexpr SpeedSetup(uint16_t ubrr_value, bool u2x): ubrr_value{ubrr_value}, u2x{u2x} {}
			const uint16_t ubrr_value;
			const bool u2x;
		};
		static constexpr SpeedSetup compute_speed(uint32_t rate)
		{
			return (UBRR_double(rate) < DOUBLE_SPEED_RATE_LIMIT) ? SpeedSetup(UBRR_double(rate), true) : SpeedSetup(UBRR_single(rate), false);
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

	using streams::OutputBuffer;
	using streams::FormattedOutput;
	
	template<board::USART USART>
	class UATX: virtual public AbstractUART, private OutputBuffer
	{
	private:
		using TRAIT = board_traits::USART_trait<USART>;

	public:
		template<uint8_t SIZE_TX> UATX(char (&output)[SIZE_TX]):OutputBuffer{output}, _transmitting(false) {}

		inline void register_handler()
		{
			::register_handler(*this);
		}

		inline void begin(	uint32_t rate,
							Parity parity = Parity::NONE, 
							StopBits stop_bits = StopBits::ONE)
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
		inline void end()
		{
			synchronized TRAIT::UCSRB = 0;
		}

		inline OutputBuffer& out()
		{
			return (OutputBuffer&) *this;
		}

		inline FormattedOutput<OutputBuffer> fout()
		{
			return FormattedOutput<OutputBuffer>(*this);
		}

		inline void data_register_empty()
		{
			_errors.has_errors = 0;
			char value;
			if (out()._pull(value))
				TRAIT::UDR = value;
			else
			{
				_errors.all_errors.queue_overflow = true;
				_transmitting = false;
				// Clear UDRIE to prevent UDR interrupt to go on forever
				TRAIT::UCSRB &= ~TRAIT::UDRIE_MASK;
			}
		}

	protected:	
		// Listeners of events on the buffer
		virtual void on_put() override
		{
			synchronized
			{
				// Check if TX is not currently active, if so, activate it
				if (!_transmitting)
				{
					// Yes, trigger TX
					char value;
					if (OutputBuffer::_pull(value))
					{
						// Set UDR interrupt to be notified when we can send the next character
						TRAIT::UCSRB |= TRAIT::UDRIE_MASK;
						TRAIT::UDR = value;
						_transmitting = true;
					}
				}
			}
		}
		virtual void on_overflow(UNUSED char c) override
		{
			_errors.all_errors.queue_overflow = true;
		}

	private:
		bool _transmitting;
	};

	using streams::InputBuffer;
	using streams::FormattedInput;
	
	template<board::USART USART>
	class UARX: virtual public AbstractUART, private InputBuffer
	{
	private:
		using TRAIT = board_traits::USART_trait<USART>;

	public:
		template<uint8_t SIZE_RX> UARX(char (&input)[SIZE_RX]):InputBuffer{input} {}

		inline void register_handler()
		{
			::register_handler(*this);
		}

		inline InputBuffer& in()
		{
			return (InputBuffer&) *this;
		}

		inline FormattedInput<InputBuffer> fin()
		{
			return FormattedInput<InputBuffer>(*this);
		}

		inline void begin(	uint32_t rate,
							Parity parity = Parity::NONE, 
							StopBits stop_bits = StopBits::ONE)
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
		inline void end()
		{
			synchronized TRAIT::UCSRB = 0;
		}

		inline void data_receive_complete()
		{
			//FIXME all constants should be in USART traits!
			char status = TRAIT::UCSRA;
			_errors.all_errors.data_overrun = status & _BV(DOR0);
			_errors.all_errors.frame_error = status & _BV(FE0);
			_errors.all_errors.parity_error = status & _BV(UPE0);
			char value = TRAIT::UDR;
			_errors.all_errors.queue_overflow = !in()._push(value);
		}
	};

	template<board::USART USART>
	class UART: public UARX<USART>, public UATX<USART>
	{
	private:
		using TRAIT = board_traits::USART_trait<USART>;

	public:
		template<uint8_t SIZE_RX, uint8_t SIZE_TX>
		UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX]):UARX<USART>{input}, UATX<USART>{output} {}

		inline void register_handler()
		{
			::register_handler(*this);
		}

		inline void begin(	uint32_t rate,
							Parity parity = Parity::NONE, 
							StopBits stop_bits = StopBits::ONE)
		{
			AbstractUART::SpeedSetup setup = AbstractUART::compute_speed(rate);
			synchronized
			{
				TRAIT::UBRR = setup.ubrr_value;
				TRAIT::UCSRA = (setup.u2x ? TRAIT::U2X_MASK : 0);
				TRAIT::UCSRB = TRAIT::TX_ENABLE_MASK | TRAIT::RX_ENABLE_MASK | TRAIT::UDRIE_MASK | TRAIT::RXCIE_MASK;
				TRAIT::UCSRC = TRAIT::UCSRC_value(parity, stop_bits);
			}
		}
		inline void end()
		{
			synchronized TRAIT::UCSRB = 0;
		}

		// Workaround trick to make REGISTER_UART_ISR work properly
		inline void data_register_empty()
		{
			UATX<USART>::data_register_empty();
		}
		inline void data_receive_complete()
		{
			UARX<USART>::data_receive_complete();
		}
	};
}

#endif	/* UCSR0A */
#endif	/* UART_HH */
