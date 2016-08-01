#include <util/delay.h>
#include <stdlib.h>

#include "uart.hh"

AbstractUART* AbstractUART::_uart[Board::USART_MAX];

void AbstractUART::begin(uint32_t rate, Parity parity, StopBits stop_bits)
{
	bool u2x = true;
	uint16_t ubrr = (F_CPU / 4 / rate - 1) / 2;
	if (ubrr >= 4096)
	{
		ubrr = (F_CPU / 8 / rate - 1) / 2;
		u2x = false;
	}
	ClearInterrupt clint;
	*Board::UBRR(_usart) = ubrr;
	*Board::UCSRA(_usart) = (u2x ? _BV(U2X0) : 0);
	*Board::UCSRB(_usart) = _BV(RXCIE0) | _BV(UDRIE0) | _BV(RXEN0) | _BV(TXEN0);
	*Board::UCSRC(_usart) = ((uint8_t) parity) | ((uint8_t) stop_bits) | _BV(UCSZ00) | _BV(UCSZ01);
}

void AbstractUART::end()
{
	ClearInterrupt clint;
	*Board::UCSRB(_usart) = 0;
}

void AbstractUART::on_flush()
{
	ClearInterrupt clint;
	// Check if TX is not currently active, if so, activate it
	if (!_transmitting)
	{
		// Yes, trigger TX
		char value;
		if (OutputBuffer::pull(value))
		{
			*Board::UDR(_usart) = value;
			_transmitting = true;
		}
	}
}

void UART_DataRegisterEmpty(Board::USART usart)
{
	AbstractUART* uart = AbstractUART::_uart[(uint8_t) usart];
	Queue<char>& buffer = uart->out();
	char value;
	if (buffer.pull(value))
		*Board::UDR(usart) = value;
	else
		uart->_transmitting = false;
}

void UART_ReceiveComplete(Board::USART usart)
{
	char value = *Board::UDR(usart);
	AbstractUART::_uart[(uint8_t) usart]->in().push(value);
}
