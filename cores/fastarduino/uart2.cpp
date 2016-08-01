#include <util/delay.h>
#include <stdlib.h>

#include "uart.hh"

#if defined(ARDUINO_MEGA)
ISR(USART2_RX_vect)
{
	UART_ReceiveComplete(Board::USART::USART_2);
}

ISR(USART2_UDRE_vect)
{
	UART_DataRegisterEmpty(Board::USART::USART_2);
}
#endif
