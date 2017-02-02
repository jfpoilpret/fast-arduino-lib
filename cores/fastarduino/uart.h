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
#include "board_traits.h"

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

class AbstractUART: public Serial::UARTErrors
{
protected:
	static void _begin(	uint32_t rate, Serial::Parity parity, Serial::StopBits stop_bits,
						volatile uint16_t& UBRR, volatile uint8_t& UCSRA,
						volatile uint8_t& UCSRB, volatile uint8_t& UCSRC,
						bool has_rx,
						bool has_tx);
	static void _end(volatile uint8_t& UCSRB);
};

class AbstractUATX: virtual public AbstractUART, private OutputBuffer
{
public:
	inline OutputBuffer& out()
	{
		return (OutputBuffer&) *this;
	}
	
	inline FormattedOutput<OutputBuffer> fout()
	{
		return FormattedOutput<OutputBuffer>(*this);
	}
	
	// Workaround for gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66957
	// Fixed in 4.9.4 (currently using 4.9.2 only)
	// We have to make the constructor public to allow virtual inheritance...
//protected:
	template<uint8_t SIZE_TX>
	AbstractUATX(char (&output)[SIZE_TX])
		:OutputBuffer{output}, _transmitting(false) {}
	
protected:
	void _on_put(volatile uint8_t& UCSRB, volatile uint8_t& UDR);
	void _data_register_empty(volatile uint8_t& UCSRB, volatile uint8_t& UDR);

private:
	bool _transmitting;
};

template<Board::USART USART>
class UATX: public AbstractUATX
{
private:
	using TRAIT = Board::USART_trait<USART>;
	
public:
	template<uint8_t SIZE_TX>
	UATX(char (&output)[SIZE_TX]):AbstractUATX(output)
	{
		_uatx = this;
	}
	
	inline void begin(	uint32_t rate,
						Serial::Parity parity = Serial::Parity::NONE, 
						Serial::StopBits stop_bits = Serial::StopBits::ONE)
	{
		_begin(rate, parity, stop_bits, TRAIT::UBRR, TRAIT::UCSRA, TRAIT::UCSRB, TRAIT::UCSRC, false, true);
	}
	inline void end()
	{
		_end(TRAIT::UCSRC);
	}

protected:	
	// Listeners of events on the buffer
	virtual void on_put() override
	{
		_on_put(TRAIT::UCSRB, TRAIT::UDR);
	}
	virtual void on_overflow(UNUSED char c) override
	{
		_errors.all_errors.queue_overflow = true;
	}
	
private:
	inline void data_register_empty()
	{
		_data_register_empty(TRAIT::UCSRB, TRAIT::UDR);
	}
	
	static UATX<USART>* _uatx;
	
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

class AbstractUARX: virtual public AbstractUART, private InputBuffer
{
public:
	inline InputBuffer& in()
	{
		return (InputBuffer&) *this;
	}
	
	inline FormattedInput<InputBuffer> fin()
	{
		return FormattedInput<InputBuffer>(*this);
	}
	
	// Workaround for gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66957
	// Fixed in 4.9.4 (currently using 4.9.2 only)
	// We have to make the constructor public to allow virtual inheritance...
//protected:
	template<uint8_t SIZE_RX>
	AbstractUARX(char (&input)[SIZE_RX]):InputBuffer{input} {}
	
protected:
	void _data_receive_complete(volatile uint8_t& UCSRA, volatile uint8_t& UDR);
};

template<Board::USART USART>
class UARX: public AbstractUARX
{
private:
	using TRAIT = Board::USART_trait<USART>;
	
public:
	template<uint8_t SIZE_RX>
	UARX(char (&input)[SIZE_RX]):AbstractUARX(input)
	{
		_uarx = this;
	}
	
	inline void begin(	uint32_t rate,
						Serial::Parity parity = Serial::Parity::NONE, 
						Serial::StopBits stop_bits = Serial::StopBits::ONE)
	{
		_begin(rate, parity, stop_bits, TRAIT::UBRR, TRAIT::UCSRA, TRAIT::UCSRB, TRAIT::UCSRC, true, false);
	}
	inline void end()
	{
		_end(TRAIT::UCSRC);
	}

private:
	inline void data_receive_complete()
	{
		_data_receive_complete(TRAIT::UCSRA, TRAIT::UDR);
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
	using TRAIT = Board::USART_trait<USART>;
	
public:
	template<uint8_t SIZE_RX, uint8_t SIZE_TX>
	UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX]):UARX<USART>{input}, UATX<USART>{output} {}
	
	inline void begin(	uint32_t rate,
						Serial::Parity parity = Serial::Parity::NONE, 
						Serial::StopBits stop_bits = Serial::StopBits::ONE)
	{
		AbstractUART::_begin(rate, parity, stop_bits, TRAIT::UBRR, TRAIT::UCSRA, TRAIT::UCSRB, TRAIT::UCSRC, true, true);
	}
	inline void end()
	{
		AbstractUART::_end(TRAIT::UCSRC);
	}
};

#endif	/* UCSR0A */
#endif	/* UART_HH */
