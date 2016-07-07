#ifndef UART_HH
#define	UART_HH

#include <stddef.h>

#include "Queue.hh"
#include "Board.hh"

//TODO check if inheritance can be made private (or protected) rather than public without defining a zillion friends...
//TODO Handle generic errors coming from UART TX (which errors?) in addition to internal overflow
//TODO Improve on_flush somehow to make it automatic, ie as soon as a char is added to the queue
class OutputBuffer:public Queue<char>
{
public:
	template<uint8_t SIZE>
	OutputBuffer(char buffer[SIZE]): Queue<char>(buffer) {}

	void flush()
	{
		on_flush();
	}
	void put(char c)
	{
		if (!push(c)) on_overflow(c);
	}
	void put(const char* content, size_t size)
	{
		while (size--) put(*content++);
	}
	void puts(const char* str)
	{
		while (*str) put(*str++);
	}
//	void puts_P();

protected:
	// Listeners of events on the buffer
	virtual void on_overflow(__attribute__((unused)) char c) {}
	virtual void on_flush() {}
};

//TODO Handle generic errors coming from UART RX (eg Parity...)
class InputBuffer: public Queue<char>
{
public:
	template<uint8_t SIZE>
	InputBuffer(char buffer[SIZE]): Queue<char>(buffer) {}

	int available() const;
	int get();
	int gets(char* str, size_t max);
	
protected:
	// Listeners of events on the buffer
	virtual void on_empty() {}
	virtual void on_get(__attribute__((unused)) char c) {}
};

//TODO improve performance and size (data and maybe code) by templatizing AFTER debug is done
class AbstractUART: private InputBuffer, private OutputBuffer
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
	
	template<uint8_t SIZE_RX, uint8_t SIZE_TX>
	AbstractUART(Board::USART usart, char input[SIZE_RX], char output[SIZE_TX])
		:InputBuffer{input}, OutputBuffer{output}, _usart(usart), _transmitting(false)
	{
		_uart[(uint8_t) usart] = this;
	}
	
	void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE);
	void end();
	
	InputBuffer& in()
	{
		return (InputBuffer&) *this;
	}
	
	OutputBuffer& out()
	{
		return (OutputBuffer&) *this;
	}
	
protected:
	// Listeners of events on the buffer
	virtual void on_flush();
	
private:
	const Board::USART _usart;
	bool _transmitting;
	
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
