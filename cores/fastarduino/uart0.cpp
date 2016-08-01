#include <util/delay.h>
#include <stdlib.h>

#include "uart.hh"

#if defined(ARDUINO_UNO)
ISR(USART_RX_vect)
{
	UART_ReceiveComplete(Board::USART::USART_0);
}

ISR(USART_UDRE_vect)
{
	UART_DataRegisterEmpty(Board::USART::USART_0);
}
#elif defined(ARDUINO_MEGA)
ISR(USART0_RX_vect)
{
	UART_ReceiveComplete(Board::USART::USART_0);
}

ISR(USART0_UDRE_vect)
{
	UART_DataRegisterEmpty(Board::USART::USART_0);
}
#endif
