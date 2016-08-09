#ifndef UART_HH
#define	UART_HH

#include "streams.hh"
#include "Board.hh"
#include "IO.hh"

namespace Soft
{
	// IOPin is much inefficient in this context, try to opt for a template version...
	class UAT: private OutputBuffer
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

		template<uint8_t SIZE_TX>
		UAT(char (&output)[SIZE_TX], Board::DigitalPin tx)
		:	OutputBuffer{output}, 
			_tx{tx, PinMode::OUTPUT}
			//TODO do we really need these initializations?
//			_parity{Parity::NONE},
//			_stop_bits{StopBits::ONE},
//			_interbit_tx_time{0},
//			_start_bit_tx_time{0},
//			_stop_bit_tx_time{0}
			{}
		
		void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			_parity = parity;
			_stop_bits = stop_bits;
			// Calculate timing for TX
			// Timing is based on number of times to count 4 cycles, because we use _delay_loop_2()
			_interbit_tx_time = uint16_t(F_CPU / 4UL / rate);
			// For start bit we shorten the duration of 50% to guarantee alignment of RX side on edges
			_start_bit_tx_time = _interbit_tx_time / 2;
			// For stop bit we lengthten the duration of 25% to guarantee alignment of RX side on stop duration
			_stop_bit_tx_time = 5 * _interbit_tx_time / 4;
			//TODO Now we adjust each time with the actual number of cycles used within write() method
			_interbit_tx_time -= 24 /4;

			// We set high level on TX (not busy)
			_tx.set();
			
			//FIXME if queue is not empty, we should process it until everything is written...
		}
		void end()
		{
			// We restore TX level to 0
			_tx.clear();
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
			if (pull(value)) write(value);
		}
		static bool calculate_parity(uint8_t value, Parity parity);
		
	private:
		void write(uint8_t value);
		
		IOPin _tx;
		// various timing constants based on rate
		Parity _parity;
		StopBits _stop_bits;
		uint16_t _interbit_tx_time;
		uint16_t _start_bit_tx_time;
		uint16_t _stop_bit_tx_time;
	};
	
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
