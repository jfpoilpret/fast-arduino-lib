#ifndef SOFTUART_HH
#define	SOFTUART_HH

#include "uartcommons.hh"
#include "streams.hh"
#include "Board.hh"
#include "FastIO.hh"
#include "PCI.hh"

//FIXME Handle begin/end properly in relation to current queue content
//TODO Find out why netbeans shows an error on in()._push() and out().pull()
namespace Soft
{
	class AbstractUATX: virtual public Serial::UARTErrors, private OutputBuffer
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

		// Workaround for gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66957
		// Fixed in 4.9.4 (currently using 4.9.2 only)
		// We have to make the constructor public to allow virtual inheritance...
//	protected:
		template<uint8_t SIZE_TX>
		AbstractUATX(char (&output)[SIZE_TX]):OutputBuffer{output} {}
		
	protected:
		void _begin(uint32_t rate, Serial::Parity parity, Serial::StopBits stop_bits);
		static Serial::Parity calculate_parity(Serial::Parity parity, uint8_t value);

		Serial::Parity _parity;
		// Various timing constants based on rate
		uint16_t _interbit_tx_time;
		uint16_t _start_bit_tx_time;
		uint16_t _stop_bit_tx_time;
	};
	
	template<Board::DigitalPin TX>
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
		virtual void on_put() override
		{
			//FIXME we should write ONLY if UAT is active (begin() has been called and not end())
			char value;
			while (out().pull(value)) write(value);
		}
		virtual void on_overflow(UNUSED char c) override
		{
			_errors.all_errors.queue_overflow = true;
		}
		
	private:
		inline void write(uint8_t value)
		{
			synchronized _write(value);
		}
		void _write(uint8_t value);
		
		typename FastPinType<TX>::TYPE _tx;
	};

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

	class AbstractUARX: virtual public Serial::UARTErrors, private InputBuffer
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
		AbstractUARX(char (&input)[SIZE_RX]):InputBuffer{input} {}

		void _begin(uint32_t rate, Serial::Parity parity, Serial::StopBits stop_bits);

		// Check if we can further refactor here, as we don't want parity stored twice for RX and TX...
		Serial::Parity _parity;
		// Various timing constants based on rate
		uint16_t _interbit_rx_time;
		uint16_t _start_bit_rx_time;
		uint16_t _parity_bit_rx_time;
		uint16_t _stop_bit_rx_time_push;
		uint16_t _stop_bit_rx_time_no_push;
	};

	template<Board::DigitalPin RX>
	class UARX: public AbstractUARX
	{
	public:
		using PCI_TYPE = typename PCIType<RX>::TYPE;
		using PORT_TRAIT = typename PCI_TYPE::TRAIT;
		
//		static const constexpr Board::Port PCIPORT = PCIType<RX>::TYPE;

		template<uint8_t SIZE_RX>
		UARX(char (&input)[SIZE_RX]):AbstractUARX(input), _rx{PinMode::INPUT}
		{
			static_assert(PORT_TRAIT::PCI_MASK & _BV(Board::DigitalPin_trait<RX>::BIT), 
				"RX must be a PinChangeInterrupt pin");
		}
		
		void begin(	PCI_TYPE& pci,
					uint32_t rate, 
					Serial::Parity parity = Serial::Parity::NONE, 
					Serial::StopBits stop_bits = Serial::StopBits::ONE)
		{
			_pci = &pci;
			_begin(rate, parity, stop_bits);
			_pci->template enable_pin<RX>();
		}
		void end()
		{
			_pci->template disable_pin<RX>();
		}
		
	protected:
		void on_pin_change();

	private:
		typename FastPinType<RX>::TYPE _rx;
		PCI_TYPE* _pci;
	};

	template<Board::DigitalPin RX>
	void UARX<RX>::on_pin_change()
	{
		// Check RX is low (start bit)
		if (_rx.value()) return;
		uint8_t value = 0;
		bool odd = false;
		Serial::_UARTErrors errors;
		errors.has_errors = 0;
		// Wait for start bit to finish
		_delay_loop_2(_start_bit_rx_time);
		// Read first 7 bits
		for (uint8_t i = 0; i < 7; ++i)
		{
			if (_rx.value())
			{
				value |= 0x80;
				odd = !odd;
			}
			value >>= 1;
			_delay_loop_2(_interbit_rx_time);
		}
		// Read last bit
		if (_rx.value())
		{
			value |= 0x80;
			odd = !odd;
		}
		
		if (_parity != Serial::Parity::NONE)
		{
			// Wait for parity bit TODO NEED SPECIFIC DELAY HERE
			_delay_loop_2(_parity_bit_rx_time);
			bool parity_bit = (_parity == Serial::Parity::ODD ? !odd : odd);
			// Check parity bit
			errors.all_errors.parity_error = (_rx.value() != parity_bit);
		}

		// Push value if no error
		if (!errors.has_errors)
		{
			errors.all_errors.queue_overflow = !in()._push(value);
			// Wait for 1st stop bit
			_delay_loop_2(_stop_bit_rx_time_push);
		}
		else
		{
			_errors = errors;
			// Wait for 1st stop bit
			_delay_loop_2(_stop_bit_rx_time_no_push);
		}
		// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
		_pci->_clear();
		return;
	}
	
	template<Board::DigitalPin RX, Board::DigitalPin TX>
	class UART: public UARX<RX>, public UATX<TX>
	{
	public:
		template<uint8_t SIZE_RX, uint8_t SIZE_TX>
		UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX])
		:UARX<RX>{input}, UATX<TX>{output} {}
		
		void begin(	typename UARX<RX>::PCI_TYPE& pci,
					uint32_t rate, 
					Serial::Parity parity = Serial::Parity::NONE, 
					Serial::StopBits stop_bits = Serial::StopBits::ONE)
		{
			UARX<RX>::begin(pci, rate, parity, stop_bits);
			UATX<TX>::begin(rate, parity, stop_bits);
		}
		void end()
		{
			UARX<RX>::end();
			UATX<TX>::end();
		}
	};
}

#endif	/* SOFTUART_HH */
