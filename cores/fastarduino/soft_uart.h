//   Copyright 2016-2018 Jean-Francois Poilpret
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
#define SOFTUART_HH

#include "boards/board.h"
#include "interrupts.h"
#include "utilities.h"
#include "uart_commons.h"
#include "streams.h"
#include "gpio.h"
#include "pci.h"
#include "int.h"

#define REGISTER_UART_PCI_ISR(RX, PCI_NUM)					\
	ISR(CAT3(PCINT, PCI_NUM, _vect))						\
	{														\
		serial::soft::check_uart_pci<PCI_NUM, RX>();		\
	}

#define REGISTER_UART_INT_ISR(RX, INT_NUM)					\
	ISR(CAT3(INT, INT_NUM, _vect))							\
	{														\
		serial::soft::check_uart_int<INT_NUM, RX>();		\
	}

//FIXME Handle begin/end properly in relation to current queue content
namespace serial::soft
{
	class AbstractUATX : virtual public UARTErrors, private streams::ostreambuf
	{
	public:
		streams::ostream out()
		{
			return streams::ostream(*this);
		}

		// Workaround for gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66957
		// Fixed in 4.9.4 (currently using 4.9.2 only)
		// We have to make the constructor public to allow virtual inheritance...
		//	protected:
		template<uint8_t SIZE_TX> AbstractUATX(char (&output)[SIZE_TX]) : ostreambuf{output}
		{
		}

	protected:
		void begin_serial(uint32_t rate, Parity parity, StopBits stop_bits);
		static Parity calculate_parity(Parity parity, uint8_t value);

		streams::ostreambuf& out_()
		{
			return (streams::ostreambuf&) *this;
		}

		void check_overflow()
		{
			errors_.queue_overflow = overflow();
		}

		Parity parity_;
		// Various timing constants based on rate
		uint16_t interbit_tx_time_;
		uint16_t start_bit_tx_time_;
		uint16_t stop_bit_tx_time_;
	};

	template<board::DigitalPin TX_> class UATX : public AbstractUATX
	{
	public:
		static constexpr const board::DigitalPin TX = TX_;

		template<uint8_t SIZE_TX> UATX(char (&output)[SIZE_TX]) : AbstractUATX{output}, tx_{gpio::PinMode::OUTPUT, true}
		{
		}

		void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			begin_serial(rate, parity, stop_bits);
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
			check_overflow();
			char value;
			while (out_().queue().pull(value)) write(value);
		}

	private:
		inline void write(uint8_t value)
		{
			synchronized write_(value);
		}
		void write_(uint8_t value);

		typename gpio::FastPinType<TX>::TYPE tx_;
	};

	template<board::DigitalPin DPIN> void UATX<DPIN>::write_(uint8_t value)
	{
		// Pre-calculate all what we need: parity bit
		Parity parity_bit = calculate_parity(parity_, value);

		// Write start bit
		tx_.clear();
		// Wait before starting actual value transmission
		_delay_loop_2(start_bit_tx_time_);
		for (uint8_t bit = 0; bit < 8; ++bit)
		{
			if (value & 0x01)
				tx_.set();
			else
			{
				// Additional NOP to ensure set/clear are executed exactly at the same time (cycle)
				asm volatile("NOP");
				tx_.clear();
			}
			value >>= 1;
			_delay_loop_2(interbit_tx_time_);
		}
		// Add parity if needed
		if (parity_bit != Parity::NONE)
		{
			if (parity_bit == parity_)
				tx_.clear();
			else
				tx_.set();
			_delay_loop_2(interbit_tx_time_);
		}
		// Add stop bit
		tx_.set();
		_delay_loop_2(stop_bit_tx_time_);
	}

	class AbstractUARX : virtual public UARTErrors, private streams::istreambuf
	{
	public:
		streams::istream in()
		{
			return streams::istream(*this);
		}

	protected:
		template<uint8_t SIZE_RX> AbstractUARX(char (&input)[SIZE_RX]) : istreambuf{input}
		{
		}

		streams::istreambuf& in_()
		{
			return (streams::istreambuf&) *this;
		}

		void begin_serial(uint32_t rate, Parity parity, StopBits stop_bits);

		// Check if we can further refactor here, as we don't want parity stored twice for RX and TX...
		Parity parity_;
		// Various timing constants based on rate
		uint16_t interbit_rx_time_;
		uint16_t start_bit_rx_time_;
		uint16_t parity_bit_rx_time_;
		uint16_t stop_bit_rx_time_push_;
		uint16_t stop_bit_rx_time_no_push_;
	};

	template<board::DigitalPin RX_> class UARX : public AbstractUARX
	{
	public:
		static constexpr const board::DigitalPin RX = RX_;
		using PCI_TYPE = typename interrupt::PCIType<RX>::TYPE;
		using INT_TYPE = interrupt::INTSignal<RX>;

	private:
		using PIN_TRAIT = board_traits::DigitalPin_trait<RX>;
		using PORT_TRAIT = board_traits::Port_trait<PIN_TRAIT::PORT>;
		using PCI_TRAIT = board_traits::PCI_trait<PORT_TRAIT::PCINT>;

	public:
		template<uint8_t SIZE_RX> UARX(char (&input)[SIZE_RX]) : AbstractUARX(input), rx_{gpio::PinMode::INPUT}
		{
			static_assert((PCI_TRAIT::PCI_MASK & _BV(PIN_TRAIT::BIT)) || (PIN_TRAIT::IS_INT),
						  "RX must be a PinChangeInterrupt or an ExternalInterrupt pin");
		}

		void register_rx_handler()
		{
			interrupt::register_handler(*this);
		}

		void begin(PCI_TYPE& enabler, uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			pci_ = &enabler;
			begin_serial(rate, parity, stop_bits);
			pci_->template enable_pin<RX>();
		}

		void begin(INT_TYPE& enabler, uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			int_ = &enabler;
			begin_serial(rate, parity, stop_bits);
			int_->enable();
		}
		void end()
		{
			if (PIN_TRAIT::IS_INT)
				int_->disable();
			else
				pci_->template disable_pin<RX>();
		}

	private:
		void on_pin_change();

		typename gpio::FastPinType<RX>::TYPE rx_;
		union
		{
			PCI_TYPE* pci_;
			INT_TYPE* int_;
		};

		friend struct isr_handler;
	};

	template<board::DigitalPin RX> void UARX<RX>::on_pin_change()
	{
		// Check RX is low (start bit)
		if (rx_.value()) return;
		uint8_t value = 0;
		bool odd = false;
		Errors errors;
		errors.has_errors = 0;
		// Wait for start bit to finish
		_delay_loop_2(start_bit_rx_time_);
		// Read first 7 bits
		for (uint8_t i = 0; i < 7; ++i)
		{
			if (rx_.value())
			{
				value |= 0x80;
				odd = !odd;
			}
			value >>= 1;
			_delay_loop_2(interbit_rx_time_);
		}
		// Read last bit
		if (rx_.value())
		{
			value |= 0x80;
			odd = !odd;
		}

		if (parity_ != Parity::NONE)
		{
			// Wait for parity bit TODO NEED SPECIFIC DELAY HERE
			_delay_loop_2(parity_bit_rx_time_);
			bool parity_bit = (parity_ == Parity::ODD ? !odd : odd);
			// Check parity bit
			errors.parity_error = (rx_.value() != parity_bit);
		}

		// Push value if no error
		if (!errors.has_errors)
		{
			errors.queue_overflow = !in_().queue().push_(value);
			// Wait for 1st stop bit
			_delay_loop_2(stop_bit_rx_time_push_);
		}
		else
		{
			errors_ = errors;
			// Wait for 1st stop bit
			_delay_loop_2(stop_bit_rx_time_no_push_);
		}
		// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
		if (PIN_TRAIT::IS_INT)
			int_->clear_();
		else
			pci_->clear_();
	}

	template<board::DigitalPin RX_, board::DigitalPin TX_> class UART : public UARX<RX_>, public UATX<TX_>
	{
	public:
		static constexpr const board::DigitalPin TX = TX_;
		static constexpr const board::DigitalPin RX = RX_;

		template<uint8_t SIZE_RX, uint8_t SIZE_TX>
		UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX]) : UARX<RX>{input}, UATX<TX>{output}
		{
		}

		void begin(typename UARX<RX>::PCI_TYPE& pci_enabler, uint32_t rate, Parity parity = Parity::NONE,
				   StopBits stop_bits = StopBits::ONE)
		{
			UARX<RX>::begin(pci_enabler, rate, parity, stop_bits);
			UATX<TX>::begin(rate, parity, stop_bits);
		}
		void begin(typename UARX<RX>::INT_TYPE& int_enabler, uint32_t rate, Parity parity = Parity::NONE,
				   StopBits stop_bits = StopBits::ONE)
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

	struct isr_handler
	{
		template<uint8_t PCI_NUM_, board::DigitalPin RX_>
		static void check_uart_pci()
		{
			interrupt::isr_handler_pci::check_pci_pins<PCI_NUM_, RX_>();
			interrupt::HandlerHolder<serial::soft::UARX<RX_>>::handler()->on_pin_change();
		}

		template<uint8_t INT_NUM_, board::DigitalPin RX_>
		static void check_uart_int()
		{
			interrupt::isr_handler_int::check_int_pin<INT_NUM_, RX_>();
			interrupt::HandlerHolder<serial::soft::UARX<RX_>>::handler()->on_pin_change();
		}
	};
}

#endif /* SOFTUART_HH */
