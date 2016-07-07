#ifndef UART_HH
#define	UART_HH

#include <stddef.h>

#include "Queue.hh"
#include "Board.hh"

//FIXME issue here is to START transmission to UART somehow...
// But don't know how to do it (without template or virtual method)
// Possible solution, have AbstractUART implement both methods for HW and SW UART?
class OutputBuffer
{
public:
	OutputBuffer(Queue<char>& buffer):_buffer(buffer) {}
	void put(char c);
	void puts(const char* str);
//	void puts_P();
	bool overflow() const;

	Queue<char>& buffer()
	{
		return _buffer;
	}
	
private:
	//TODO better subclass Queue<char> no?
	Queue<char>& _buffer;
};

class InputBuffer
{
public:
	InputBuffer(Queue<char>& buffer):_buffer(buffer) {}
	int available() const;
	int get();
	int gets(char* str, size_t max);
	
	Queue<char>& buffer()
	{
		return _buffer;
	}
	
private:
	Queue<char>& _buffer;
};

//TODO directly embed InputBuffer/OutputBuffer and provide in() and out() methods
//TODO improve performance and size (data and maybe code) by templatizing AFTER debug is done
class AbstractUART
{
public:
	enum class Parity
	{
		NONE = 0x00,
		EVEN = _BV(UPM00),
		ODD = _BV(UPM00) | _BV(UPM01)
	};
	enum class StopBits
	{
		ONE = 0x00,
		TWO = _BV(USBS0)
	};
	AbstractUART(Board::USART usart, InputBuffer& input, OutputBuffer& output)
		:_usart(usart), _input(input), _output(output)
	{
		_uart[(uint8_t) usart] = this;
	}
	void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE);
	void end();
	
private:
	const Board::USART _usart;
	InputBuffer& _input;
	OutputBuffer& _output;
	
	//TODO replace with global InputBuffer/OutputBuffer ?
	static AbstractUART* _uart[Board::USART_MAX];
	//TODO and then remove those stupid friends from here
	friend void UART_DataRegisterEmpty(Board::USART usart);
	friend void UART_ReceiveComplete(Board::USART usart);
};

//Not sure we need a template...
//template<Board::USART USART>
//class UART: private AbstractUART
//{
//public:
//	UART(InputBuffer& input, OutputBuffer& output):AbstractUART(input, output) {}
//	void begin();
//	void end();
//	
//private:
//	static const constexpr volatile uint8_t& UCSRA = *Board::UCSRA(USART);
//	static const constexpr volatile uint8_t& UCSRB = *Board::UCSRB(USART);
//	static const constexpr volatile uint8_t& UCSRC = *Board::UCSRC(USART);
//	static const constexpr volatile uint8_t& UBRR = *Board::UBRR(USART);
//	static const constexpr volatile uint8_t& UDR = *Board::UDR(USART);
//};

#endif	/* UART_HH */
