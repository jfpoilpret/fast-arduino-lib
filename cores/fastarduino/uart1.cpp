#include <util/delay.h>
#include <stdlib.h>

#include "uart.hh"

#if defined(ARDUINO_MEGA)
ISR(USART1_RX_vect)
{
	UART_ReceiveComplete(Board::USART::USART_1);
}

ISR(USART1_UDRE_vect)
{
	UART_DataRegisterEmpty(Board::USART::USART_1);
}
#endif
