#include "uart.hh"

#if defined(UCSR0A)
void AbstractUART::_begin(	uint32_t rate, Serial::Parity parity, Serial::StopBits stop_bits,
							volatile uint16_t& UBRR, volatile uint8_t& UCSRA,
							volatile uint8_t& UCSRB, volatile uint8_t& UCSRC,
							bool has_rx, bool has_tx)
{
	bool u2x = true;
	uint16_t ubrr = (F_CPU / 4 / rate - 1) / 2;
	if (ubrr >= 4096)
	{
		ubrr = (F_CPU / 8 / rate - 1) / 2;
		u2x = false;
	}
	synchronized
	{
		UBRR = ubrr;
		UCSRA = (u2x ? _BV(U2X0) : 0);
		UCSRB = (has_rx ? _BV(RXCIE0) : 0) | (has_tx ? _BV(UDRIE0) : 0) | _BV(RXEN0) | _BV(TXEN0);
		UCSRC = Serial::AVR_USART_PARITY(parity) | Serial::AVR_USART_STOPBITS(stop_bits) | _BV(UCSZ00) | _BV(UCSZ01);
	}
}

void AbstractUART::_end(volatile uint8_t& UCSRB)
{
	synchronized UCSRB = 0;
}

void AbstractUATX::_on_put(volatile uint8_t& UCSRB, volatile uint8_t& UDR)
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
				UCSRB |= _BV(UDRIE0);
				UDR = value;
				_transmitting = true;
			}
		}
	}
}

void AbstractUATX::_data_register_empty(volatile uint8_t& UCSRB, volatile uint8_t& UDR)
{
	_errors.has_errors = 0;
	char value;
	if (out()._pull(value))
		UDR = value;
	else
	{
		_errors.all_errors.queue_overflow = true;
		_transmitting = false;
		// Clear UDRIE to prevent UDR interrupt to go on forever
		UCSRB &= ~_BV(UDRIE0);
	}
}

void AbstractUARX::_data_receive_complete(volatile uint8_t& UCSRA, volatile uint8_t& UDR)
{
	char status = UCSRA;
	_errors.all_errors.data_overrun = status & _BV(DOR0);
	_errors.all_errors.frame_error = status & _BV(FE0);
	_errors.all_errors.parity_error = status & _BV(UPE0);
	char value = UDR;
	_errors.all_errors.queue_overflow = !in()._push(value);
}

#endif
