#ifndef UART_HH
#define	UART_HH

#include "streams.hh"
#include "Board.hh"
#include "FastIO.hh"

namespace Soft
{
	class AbstractUAT
	{
	public:
		enum class Parity: uint8_t
		{
			NONE = 0,
			EVEN = 1,
			ODD = 3
		};
		enum class StopBits: uint8_t
		{
			ONE = 1,
			TWO = 2
		};

	protected:
		AbstractUAT() {}
		
		void _begin(uint32_t rate, Parity parity, StopBits stop_bits);
		Parity calculate_parity(uint8_t value);

		// Check if we can further refactor here, eg adding parity and stop bits as field members,
		// as well as time calculations?
		// various timing constants based on rate
		Parity _parity;
		uint16_t _interbit_tx_time;
		uint16_t _start_bit_tx_time;
		uint16_t _stop_bit_tx_time;
	};
	
	// IOPin is much inefficient in this context, try to opt for a template version...
	template<Board::DigitalPin DPIN>
	class UAT: public AbstractUAT, private OutputBuffer
	{
	public:
		template<uint8_t SIZE_TX>
		UAT(char (&output)[SIZE_TX])
		:	OutputBuffer{output}, 
			_tx{PinMode::OUTPUT, true}
			{}
		
		void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			_begin(rate, parity, stop_bits);
			//FIXME if queue is not empty, we should process it until everything is written...
		}
		void end()
		{
			//FIXME if queue not empty we should:
			// - prevent pushing to it (how?)
			// - flush it completely
			// - enable pushing again
		}
		
		OutputBuffer& out()
		{
			return (OutputBuffer&) *this;
		}

		FormattedOutput<OutputBuffer> fout()
		{
			return FormattedOutput<OutputBuffer>(*this);
		}
		
	protected:
		virtual void on_put()
		{
			//FIXME we should write ONLY if UAT is active (begin() has been called and not end())
			char value;
			while (pull(value)) write(value);
		}
		
	private:
		void write(uint8_t value);
		
		FastPin<DPIN> _tx;
	};

	//TODO THAT SHOULD BELONG TO AbstractUAT but should be passed TX as a parameter (template method) 
	// or just clear/set as FP or sth like that... but that might imply extra timing not so good...
	template<Board::DigitalPin DPIN>
	void UAT<DPIN>::write(uint8_t value)
	{
		// Pre-calculate all what we need: parity bit
		Parity parity_bit = calculate_parity(value);

		// Write start bit
		_tx.clear();
		// Wait before starting actual value transmission
		_delay_loop_2(_start_bit_tx_time);
		for (uint8_t bit = 0; bit < 8; ++bit)
		{
			if (value & 0x01) 
				_tx.set(); 
			else
			{
				// Additional NOP to ensure set/clear are executed exactly at the same time (cycle)
				asm volatile("NOP");
				_tx.clear();
			}
			value >>= 1;
			_delay_loop_2(_interbit_tx_time);
		}
		// Add parity if needed
		//TODO NOT CHECKED YET, NEED DEBUG!!!
		if (parity_bit != Parity::NONE)
		{
			if (parity_bit == _parity) _tx.clear(); else _tx.set();
			_delay_loop_2(_interbit_tx_time);
		}
		// Add stop bit
		_tx.set();
		_delay_loop_2(_stop_bit_tx_time);
	}

//class AbstractUART: private InputBuffer, private OutputBuffer
//{
//public:
//	enum class Parity: uint8_t
//	{
//		NONE = 0,
//		EVEN = 1,
//		ODD = 3
//	};
//	enum class StopBits: uint8_t
//	{
//		ONE = 1,
//		TWO = 2
//	};
//
//	InputBuffer& in()
//	{
//		return (InputBuffer&) *this;
//	}
//	
//	FormattedInput<InputBuffer> fin()
//	{
//		return FormattedInput<InputBuffer>(*this);
//	}
//	
//	OutputBuffer& out()
//	{
//		return (OutputBuffer&) *this;
//	}
//	
//	FormattedOutput<OutputBuffer> fout()
//	{
//		return FormattedOutput<OutputBuffer>(*this);
//	}
//	
//protected:
//	template<uint8_t SIZE_RX, uint8_t SIZE_TX>
//	AbstractUART(char (&input)[SIZE_RX], char (&output)[SIZE_TX])
//		:InputBuffer{input, true}, OutputBuffer{output}, _transmitting(false) {}
//	
//	void _begin(uint32_t rate, Parity parity, StopBits stop_bits);
//	void _end();
//	
//	void _on_put();
//	
//private:
//	bool _transmitting;
//	// various timing constants based on rate
//};

//template<Board::PCIPort PORT>
//class UART: public AbstractUART
//{
//public:
//	// MUST PASS TX/RX pins
//	template<uint8_t SIZE_RX, uint8_t SIZE_TX>
//	UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX])
//		:AbstractUART(input, output)
//	{
////		_uart = this;
//	}
//	
//	void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
//	{
//		_begin(rate, parity, stop_bits);
//	}
//	void end()
//	{
//		_end();
//	}
//
//protected:	
//	// Listeners of events on the buffer
//	virtual void on_put()
//	{
//		_on_put(UCSRB, UDR);
//	}
//	
//private:
//	void data_register_empty()
//	{
//		_data_register_empty(UCSRB, UDR);
//	}
//	void data_receive_complete()
//	{
//		_data_receive_complete(UDR);
//	}
//};

//template<Board::USART USART>
//UART<USART>* UART<USART>::_uart = 0;

}

#endif	/* UART_HH */
