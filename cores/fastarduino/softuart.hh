#ifndef SOFTUART_HH
#define	SOFTUART_HH

#include "streams.hh"
#include "Board.hh"
#include "FastIO.hh"
#include "PCI.hh"

namespace Soft
{
	namespace Serial
	{
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
	}

	//TODO Create AbstractUART which would be parent of all (virtual inheritance) and include Parity as field
	
	class AbstractUATX: private OutputBuffer
	{
	public:
		OutputBuffer& out()
		{
			return (OutputBuffer&) *this;
		}

		FormattedOutput<OutputBuffer> fout()
		{
			return FormattedOutput<OutputBuffer>(*this);
		}
		
	protected:
		template<uint8_t SIZE_TX>
		AbstractUATX(char (&output)[SIZE_TX]):OutputBuffer{output} {}
		
		void _begin(uint32_t rate, Serial::Parity parity, Serial::StopBits stop_bits);
		static Serial::Parity calculate_parity(Serial::Parity parity, uint8_t value);

		// Check if we can further refactor here, as we don't want parity stored twice for RX and TX...
		Serial::Parity _parity;
		// Various timing constants based on rate
		uint16_t _interbit_tx_time;
		uint16_t _start_bit_tx_time;
		uint16_t _stop_bit_tx_time;
	};
	
	template<Board::DigitalPin DPIN>
	class UATX: public AbstractUATX
	{
	public:
		template<uint8_t SIZE_TX>
		UATX(char (&output)[SIZE_TX]):AbstractUATX{output}, _tx{PinMode::OUTPUT, true} {}
		
		void begin(	uint32_t rate, 
					Serial::Parity parity = Serial::Parity::NONE, 
					Serial::StopBits stop_bits = Serial::StopBits::ONE)
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
		
	protected:
		virtual void on_put()
		{
			//FIXME we should write ONLY if UAT is active (begin() has been called and not end())
			char value;
			while (out().pull(value)) write(value);
		}
		
	private:
		inline void write(uint8_t value)
		{
			synchronized _write(value);
		}
		void _write(uint8_t value);
		
		FastPin<DPIN> _tx;
	};

	//TODO THAT SHOULD BELONG TO AbstractUAT but should be passed TX as a parameter (template method) 
	// or just clear/set as FP or sth like that... but that might imply extra timing not so good...
	template<Board::DigitalPin DPIN>
	void UATX<DPIN>::_write(uint8_t value)
	{
		// Pre-calculate all what we need: parity bit
		Serial::Parity parity_bit = calculate_parity(_parity, value);

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
		if (parity_bit != Serial::Parity::NONE)
		{
			if (parity_bit == _parity) _tx.clear(); else _tx.set();
			_delay_loop_2(_interbit_tx_time);
		}
		// Add stop bit
		_tx.set();
		_delay_loop_2(_stop_bit_tx_time);
	}

	class AbstractUARX: private InputBuffer
	{
	public:
		InputBuffer& in()
		{
			return (InputBuffer&) *this;
		}

		FormattedInput<InputBuffer> fin()
		{
			return FormattedInput<InputBuffer>(*this);
		}

	protected:
		template<uint8_t SIZE_RX>
		AbstractUARX(char (&input)[SIZE_RX], bool blocking):InputBuffer{input, blocking} {}
//		AbstractUARX(char (&input)[SIZE_RX]):InputBuffer{input, true} {}

		void _begin(uint32_t rate, Soft::Serial::Parity parity, Soft::Serial::StopBits stop_bits);

		// Check if we can further refactor here, as we don't want parity stored twice for RX and TX...
		Serial::Parity _parity;
		// Various timing constants based on rate
		uint16_t _interbit_rx_time;
		uint16_t _start_bit_rx_time;
		uint16_t _stop_bit_rx_time;
	};

	template<Board::InterruptPin RX>
	class UARX: public AbstractUARX, public PCIHandler
	{
	public:
		static const constexpr Board::DigitalPin DPIN = (Board::DigitalPin) RX;
		static const constexpr Board::PCIPort PCIPORT = Board::PCI_PORT(RX);

		template<uint8_t SIZE_RX>
		UARX(char (&input)[SIZE_RX], bool blocking = false):AbstractUARX(input, blocking), _rx{PinMode::INPUT} {}
		
		void begin(	PCI<PCIPORT>& pci,
					uint32_t rate, 
					Soft::Serial::Parity parity = Soft::Serial::Parity::NONE, 
					Soft::Serial::StopBits stop_bits = Soft::Serial::StopBits::ONE)
		{
			_pci = &pci;
			_begin(rate, parity, stop_bits);
			_pci->enable_pin(RX);
		}
	//	void end()
	//	{
	//		_end();
	//	}
		
	protected:
		virtual bool pin_change()
		{
			return _pin_change(*this);
		}
		//TODO Recheck if static is needed
		static bool _pin_change(UARX<RX>& uarx);

	private:
		FastPin<DPIN> _rx;
		PCI<PCIPORT>* _pci;
	};

	template<Board::InterruptPin RX>
	bool UARX<RX>::_pin_change(UARX<RX>& uarx)
	{
		// Check RX is low (start bit)
		if (uarx._rx.value()) return false;
		uint8_t value = 0;
		// Wait for start bit to finish
		_delay_loop_2(uarx._start_bit_rx_time);
		// Read first 7 bits
		for (uint8_t i = 0; i < 7; ++i)
		{
			if (uarx._rx.value()) value |= 0x80;
			value >>= 1;
			_delay_loop_2(uarx._interbit_rx_time);
		}
		// Read last bit
		if (uarx._rx.value()) value |= 0x80;
		//TODO if parity calculate and check it
		
		// Push value (TODO and check if overflow)
		uarx.in()._push(value);
		_delay_loop_2(uarx._stop_bit_rx_time);
		// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
		uarx._pci->_clear();
		return true;
	}
}

#endif	/* SOFTUART_HH */
