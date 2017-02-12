//   Copyright 2016-2017 Jean-Francois Poilpret
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

#ifndef SOFTUART_HH
#define	SOFTUART_HH

#include "utilities.h"
#include "uart_commons.h"
#include "streams.h"
#include "boards/board.h"
#include "fast_io.h"
#include "pci.h"
#include "int.h"

#define REGISTER_UART_PCI_ISR(RX, PCI_NUM) \
REGISTER_PCI_ISR_METHOD(PCI_NUM, Soft::UARX< RX >, & Soft::UARX< RX >::on_pin_change)

#define REGISTER_UART_INT_ISR(RX, INT_NUM) \
REGISTER_INT_ISR_METHOD(INT_NUM, Soft::UARX< RX >, & Soft::UARX< RX >::on_pin_change)

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
		using PIN_TRAIT = board_traits::DigitalPin_trait<RX>;
		using PCI_TYPE = typename PCIType<RX>::TYPE;
		using PORT_TRAIT = typename PCI_TYPE::TRAIT;
		using INT_TYPE = INTSignal<RX>;
		
		template<uint8_t SIZE_RX>
		UARX(char (&input)[SIZE_RX]):AbstractUARX(input), _rx{PinMode::INPUT}
		{
			static_assert(
				(PORT_TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<RX>::BIT)) || (PIN_TRAIT::IS_INT), 
				"RX must be a PinChangeInterrupt or an ExternalInterrupt pin");
		}
		
		void register_rx_handler()
		{
			register_handler(*this);
		}
		
		void begin(	PCI_TYPE& enabler,
					uint32_t rate, 
					Serial::Parity parity = Serial::Parity::NONE, 
					Serial::StopBits stop_bits = Serial::StopBits::ONE)
		{
			_pci = &enabler;
			_begin(rate, parity, stop_bits);
			_pci->template enable_pin<RX>();
		}
		
		void begin(	INT_TYPE& enabler,
					uint32_t rate, 
					Serial::Parity parity = Serial::Parity::NONE, 
					Serial::StopBits stop_bits = Serial::StopBits::ONE)
		{
			_int = &enabler;
			_begin(rate, parity, stop_bits);
			_int->enable();
		}
		void end()
		{
			if (PIN_TRAIT::IS_INT)
				_int->disable();
			else
				_pci->template disable_pin<RX>();
		}
		
//	protected:
		void on_pin_change();

	private:
		typename FastPinType<RX>::TYPE _rx;
		union
		{
			PCI_TYPE* _pci;
			INT_TYPE* _int;
		};
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
		if (PIN_TRAIT::IS_INT)
			_int->_clear();
		else
			_pci->_clear();
	}
	
	template<Board::DigitalPin RX, Board::DigitalPin TX>
	class UART: public UARX<RX>, public UATX<TX>
	{
	public:
		template<uint8_t SIZE_RX, uint8_t SIZE_TX>
		UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX])
		:UARX<RX>{input}, UATX<TX>{output} {}
		
		void begin(	typename UARX<RX>::PCI_TYPE& pci_enabler,
					uint32_t rate, 
					Serial::Parity parity = Serial::Parity::NONE, 
					Serial::StopBits stop_bits = Serial::StopBits::ONE)
		{
			UARX<RX>::begin(pci_enabler, rate, parity, stop_bits);
			UATX<TX>::begin(rate, parity, stop_bits);
		}
		void begin(	typename UARX<RX>::INT_TYPE& int_enabler,
					uint32_t rate, 
					Serial::Parity parity = Serial::Parity::NONE, 
					Serial::StopBits stop_bits = Serial::StopBits::ONE)
		{
			UARX<RX>::begin(int_enabler, rate, parity, stop_bits);
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
