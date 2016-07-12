#ifndef UART_HH
#define	UART_HH

#include "streams.hh"
#include "Board.hh"

//TODO check if inheritance can be made private (or protected) rather than public without defining a zillion friends...
//TODO Handle generic errors coming from UART TX (which errors?) in addition to internal overflow

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
	AbstractUART(Board::USART usart, char (&input)[SIZE_RX], char (&output)[SIZE_TX])
	:	InputBuffer{input, SIZE_RX}, OutputBuffer{output, SIZE_TX},
		_usart(usart), _transmitting(false)
	{
		static_assert(SIZE_RX && !(SIZE_RX & (SIZE_RX - 1)), "SIZE_RX must be a power of 2");
		static_assert(SIZE_TX && !(SIZE_TX & (SIZE_TX - 1)), "SIZE_TX must be a power of 2");
		_uart[(uint8_t) usart] = this;
	}
	
	template<uint8_t SIZE_RX, uint8_t SIZE_TX>
	static AbstractUART create(Board::USART usart, char input[SIZE_RX], char output[SIZE_TX])
	{
		static_assert(SIZE_RX && !(SIZE_RX & (SIZE_RX - 1)), "SIZE_RX must be a power of 2");
		static_assert(SIZE_TX && !(SIZE_TX & (SIZE_TX - 1)), "SIZE_TX must be a power of 2");
		return AbstractUART{usart, input, SIZE_RX, output, SIZE_TX};
	}
	
	void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE);
	void end();
	
	InputBuffer& in()
	{
		return (InputBuffer&) *this;
	}
	
	FormattedOutput<OutputBuffer> out()
	{
		return FormattedOutput<OutputBuffer>(*this);
	}
	
protected:
	AbstractUART(Board::USART usart, char* input, uint8_t size_rx, char* output, uint8_t size_tx)
	:	InputBuffer{input, size_rx}, OutputBuffer{output, size_tx}, 
		_usart(usart), _transmitting(false)
	{
		_uart[(uint8_t) usart] = this;
	}
	
	// Listeners of events on the buffer
	virtual void on_flush();
	
private:
	InputBuffer& inqueue()
	{
		return (InputBuffer&) *this;
	}
	
	OutputBuffer& outqueue()
	{
		return (OutputBuffer&) *this;
	}
	
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
