#include <util/delay.h>
#include <stdlib.h>

#include "uart.hh"

#if defined(UCSR0A)
void AbstractUART::_begin(	uint32_t rate, Parity parity, StopBits stop_bits,
							volatile uint16_t& UBRR, volatile uint8_t& UCSRA,
							volatile uint8_t& UCSRB, volatile uint8_t& UCSRC)
{
	bool u2x = true;
	uint16_t ubrr = (F_CPU / 4 / rate - 1) / 2;
	if (ubrr >= 4096)
	{
		ubrr = (F_CPU / 8 / rate - 1) / 2;
		u2x = false;
	}
	ClearInterrupt clint;
	UBRR = ubrr;
	UCSRA = (u2x ? _BV(U2X0) : 0);
	UCSRB = _BV(RXCIE0) | _BV(UDRIE0) | _BV(RXEN0) | _BV(TXEN0);
	UCSRC = ((uint8_t) parity) | ((uint8_t) stop_bits) | _BV(UCSZ00) | _BV(UCSZ01);
}

void AbstractUART::_end(volatile uint8_t& UCSRB)
{
	ClearInterrupt clint;
	UCSRB = 0;
}

void AbstractUART::_on_put(volatile uint8_t& UCSRB, volatile uint8_t& UDR)
{
	ClearInterrupt clint;
	// Check if TX is not currently active, if so, activate it
	if (!_transmitting)
	{
		// Yes, trigger TX
		char value;
		if (OutputBuffer::pull(value))
		{
			// Set UDR interrupt to be notified when we can send the next character
			UCSRB |= _BV(UDRIE0);
			UDR = value;
			_transmitting = true;
		}
	}
}

void AbstractUART::_data_register_empty(volatile uint8_t& UCSRB, volatile uint8_t& UDR)
{
	Queue<char>& buffer = out();
	char value;
	if (buffer.pull(value))
		UDR = value;
	else
	{
		_transmitting = false;
		// Clear UDRIE to prevent UDR interrupt to go on forever
		UCSRB &= ~_BV(UDRIE0);
	}
}

void AbstractUART::_data_receive_complete(volatile uint8_t& UDR)
{
	char value = UDR;
	in().push(value);
}

#endif
