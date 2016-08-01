#include <util/delay.h>
#include <stdlib.h>

#include "uart.hh"

#if defined(ARDUINO_MEGA)
ISR(USART3_RX_vect)
{
	UART_ReceiveComplete(Board::USART::USART_3);
}

ISR(USART3_UDRE_vect)
{
	UART_DataRegisterEmpty(Board::USART::USART_3);
}
#endif
