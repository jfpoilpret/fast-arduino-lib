#include <util/delay.h>

#include "uart.hh"

AbstractUART* AbstractUART::_uart[Board::USART_MAX];

void AbstractUART::begin(uint32_t rate, Parity parity, StopBits stop_bits)
{
	ClearInterrupt clint;
	*Board::UBRR(_usart) = F_CPU / 16 / rate - 1;
	*Board::UCSRB(_usart) = _BV(RXCIE0) | _BV(UDRIE0) | _BV(RXEN0) | _BV(TXEN0);
	*Board::UCSRC(_usart) = ((uint8_t) parity) | ((uint8_t) stop_bits) | _BV(UCSZ00) | _BV(UCSZ01);
}

void AbstractUART::end()
{
	ClearInterrupt clint;
	*Board::UCSRB(_usart) = 0;
}

void UART_DataRegisterEmpty(Board::USART usart)
{
	Queue<char>& buffer = AbstractUART::_uart[(uint8_t) usart]->_output.buffer();
	char value;
	if (buffer.pull(value))
		*Board::UDR(usart) = value;
}

void UART_ReceiveComplete(Board::USART usart)
{
	char value = *Board::UDR(usart);
	AbstractUART::_uart[(uint8_t) usart]->_input.buffer().push(value);
}

ISR(USART_RX_vect)
{
	UART_ReceiveComplete(Board::USART::USART_0);
}

ISR(USART_UDRE_vect)
{
	UART_DataRegisterEmpty(Board::USART::USART_0);
}
