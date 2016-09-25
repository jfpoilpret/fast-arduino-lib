#ifndef UART_HH
#define	UART_HH

#include "uartcommons.hh"
#include "streams.hh"
#include "Board.hh"

// Only MCU with physical USART are supported (not ATtiny then)
#if defined(UCSR0A)

// This macro is internally used in further macros and should not be used in your programs
#define _USE_UART(NAME)													\
ISR(USART ## NAME ## _RX_vect)											\
{																		\
	UART<Board::USART::USART ## NAME>::_uart->data_receive_complete();	\
}																		\
ISR(USART ## NAME ## _UDRE_vect)										\
{																		\
	UART<Board::USART::USART ## NAME>::_uart->data_register_empty();	\
}

// Those macros should be added somewhere in a cpp file (advised name: vectors.cpp) to indicate you
// use a given UART in your program hence you need the proper ISR vector correctly defined
#define USE_UART0()	_USE_UART(0)
#define USE_UART1()	_USE_UART(1)
#define USE_UART2()	_USE_UART(2)
#define USE_UART3()	_USE_UART(3)

// Internal macros to simplify friends declaration
#define _FRIEND_RX_VECT(NAME) friend void NAME ## _RX_vect()
#define _FRIEND_UDRE_VECT(NAME) friend void NAME ## _UDRE_vect()
#define _FRIENDS(NAME)		\
	_FRIEND_RX_VECT(NAME);	\
	_FRIEND_UDRE_VECT(NAME);

//TODO Handle generic errors coming from UART TX (which errors?) in addition to internal overflow
class AbstractUART: public Serial::UARTErrors, private InputBuffer, private OutputBuffer
{
public:
	InputBuffer& in()
	{
		return (InputBuffer&) *this;
	}
	
	FormattedInput<InputBuffer> fin()
	{
		return FormattedInput<InputBuffer>(*this);
	}
	
	OutputBuffer& out()
	{
		return (OutputBuffer&) *this;
	}
	
	FormattedOutput<OutputBuffer> fout()
	{
		return FormattedOutput<OutputBuffer>(*this);
	}
	
protected:
	template<uint8_t SIZE_RX, uint8_t SIZE_TX>
	AbstractUART(char (&input)[SIZE_RX], char (&output)[SIZE_TX])
		:InputBuffer{input}, OutputBuffer{output}, _transmitting(false) {}
	
	static void _begin(	uint32_t rate, Serial::Parity parity, Serial::StopBits stop_bits,
						volatile uint16_t& UBRR, volatile uint8_t& UCSRA,
						volatile uint8_t& UCSRB, volatile uint8_t& UCSRC);
	static void _end(volatile uint8_t& UCSRB);
	
	void _on_put(volatile uint8_t& UCSRB, volatile uint8_t& UDR);
	
	void _data_register_empty(volatile uint8_t& UCSRB, volatile uint8_t& UDR);
	void _data_receive_complete(volatile uint8_t& UCSRA, volatile uint8_t& UDR);

private:
	bool _transmitting;
};

template<Board::USART USART>
class UART: public AbstractUART
{
public:
	template<uint8_t SIZE_RX, uint8_t SIZE_TX>
	UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX])
		:AbstractUART(input, output)
	{
		_uart = this;
	}
	
	void begin(	uint32_t rate,
				Serial::Parity parity = Serial::Parity::NONE, 
				Serial::StopBits stop_bits = Serial::StopBits::ONE)
	{
		_begin(rate, parity, stop_bits, UBRR, UCSRA, UCSRB, UCSRC);
	}
	void end()
	{
		_end(UCSRC);
	}

protected:	
	// Listeners of events on the buffer
	virtual void on_put() override
	{
		_on_put(UCSRB, UDR);
	}
	virtual void on_overflow(UNUSED char c) override
	{
		_errors.all_errors.queue_overflow = true;
	}
	
private:
	void data_register_empty()
	{
		_data_register_empty(UCSRB, UDR);
	}
	void data_receive_complete()
	{
		_data_receive_complete(UCSRA, UDR);
	}
	
	static UART<USART>* _uart;
	
	static const constexpr REGISTER UCSRA = Board::UCSRA_REG(USART);
	static const constexpr REGISTER UCSRB = Board::UCSRB_REG(USART);
	static const constexpr REGISTER UCSRC = Board::UCSRC_REG(USART);
	static const constexpr REGISTER UBRR = Board::UBRR_REG(USART);
	static const constexpr REGISTER UDR = Board::UDR_REG(USART);
	
	_FRIENDS(USART0)
#if defined(UCSR1A)
	_FRIENDS(USART1)
#endif
#if defined(UCSR2A)
	_FRIENDS(USART2)
#endif
#if defined(UCSR3A)
	_FRIENDS(USART3)
#endif
};

template<Board::USART USART>
UART<USART>* UART<USART>::_uart = 0;
#endif

#endif	/* UART_HH */
