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

// This macro is internally used in further macros and should not be used in your programs
#define _USE_UATX(NAME)													\
ISR(CAT3(USART, NAME, _UDRE_vect))										\
{																		\
	CAT(UATX<Board::USART::USART, NAME)>::_uatx->data_register_empty();	\
}
#define _USE_UARX(NAME)														\
ISR(CAT3(USART, NAME, _RX_vect))											\
{																			\
	CAT(UARX<Board::USART::USART, NAME)>::_uarx->data_receive_complete();	\
}

#define _USE_UART(NAME)		\
_USE_UARX(NAME)				\
_USE_UATX(NAME)

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// use a given UART in your program hence you need the proper ISR vector correctly defined
#define USE_UATX0()	_USE_UATX(0)
#define USE_UATX1()	_USE_UATX(1)
#define USE_UATX2()	_USE_UATX(2)
#define USE_UATX3()	_USE_UATX(3)

#define USE_UARX0()	_USE_UARX(0)
#define USE_UARX1()	_USE_UARX(1)
#define USE_UARX2()	_USE_UARX(2)
#define USE_UARX3()	_USE_UARX(3)

#define USE_UART0()	_USE_UART(0)
#define USE_UART1()	_USE_UART(1)
#define USE_UART2()	_USE_UART(2)
#define USE_UART3()	_USE_UART(3)

//TODO Handle generic errors coming from UART TX (which errors?) in addition to internal overflow
//FIXME reuse generic way to handle ISR (cleaner and avoids multiple friends in class)
class AbstractUART: public Serial::UARTErrors
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

template<Board::USART USART>
class UATX: virtual public AbstractUART, private OutputBuffer
{
private:
	using TRAIT = board_traits::USART_trait<USART>;
	
public:
	template<uint8_t SIZE_TX>
	UATX(char (&output)[SIZE_TX]):OutputBuffer{output}, _transmitting(false)
	{
		_uatx = this;
	}
	
	inline void begin(	uint32_t rate,
						Serial::Parity parity = Serial::Parity::NONE, 
						Serial::StopBits stop_bits = Serial::StopBits::ONE)
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
	
	static UATX<USART>* _uatx;
	
	bool _transmitting;

	friend void USART0_UDRE_vect();
#if defined(UCSR1A)
	friend void USART1_UDRE_vect();
#endif
#if defined(UCSR2A)
	friend void USART2_UDRE_vect();
#endif
#if defined(UCSR3A)
	friend void USART3_UDRE_vect();
#endif
};

template<Board::USART USART>
UATX<USART>* UATX<USART>::_uatx = 0;

template<Board::USART USART>
class UARX: virtual public AbstractUART, private InputBuffer
{
private:
	using TRAIT = board_traits::USART_trait<USART>;
	
public:
	template<uint8_t SIZE_RX>
	UARX(char (&input)[SIZE_RX]):InputBuffer{input}
	{
		_uarx = this;
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
						Serial::Parity parity = Serial::Parity::NONE, 
						Serial::StopBits stop_bits = Serial::StopBits::ONE)
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

private:
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
	
	static UARX<USART>* _uarx;
	
	friend void USART0_RX_vect();
#if defined(UCSR1A)
	friend void USART1_RX_vect();
#endif
#if defined(UCSR2A)
	friend void USART2_RX_vect();
#endif
#if defined(UCSR3A)
	friend void USART3_RX_vect();
#endif
};

template<Board::USART USART>
UARX<USART>* UARX<USART>::_uarx = 0;

template<Board::USART USART>
class UART: public UARX<USART>, public UATX<USART>
{
private:
	using TRAIT = board_traits::USART_trait<USART>;
	
public:
	template<uint8_t SIZE_RX, uint8_t SIZE_TX>
	UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX]):UARX<USART>{input}, UATX<USART>{output} {}
	
	inline void begin(	uint32_t rate,
						Serial::Parity parity = Serial::Parity::NONE, 
						Serial::StopBits stop_bits = Serial::StopBits::ONE)
	{
		AbstractUART::SpeedSetup setup = AbstractUART::compute_speed(rate);
		synchronized
		{
			//FIXME all constants should be in USART traits!
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
};

#endif	/* UCSR0A */
#endif	/* UART_HH */
