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
#define SOFTUART_HH

#include "interrupts.h"
#include "utilities.h"
#include "uart_commons.h"
#include "streams.h"
#include "boards/board.h"
#include "gpio.h"
#include "pci.h"
#include "int.h"

#define REGISTER_UART_PCI_ISR(RX, PCI_NUM) \
	REGISTER_PCI_ISR_METHOD(PCI_NUM, serial::soft::UARX<RX>, &serial::soft::UARX<RX>::on_pin_change, RX)

#define REGISTER_UART_INT_ISR(RX, INT_NUM) \
	REGISTER_INT_ISR_METHOD(INT_NUM, RX, serial::soft::UARX<RX>, &serial::soft::UARX<RX>::on_pin_change)

//FIXME Handle begin/end properly in relation to current queue content
namespace serial
{
	namespace soft
	{
		class AbstractUATX : virtual public UARTErrors, private streams::OutputBuffer
		{
		public:
			streams::OutputBuffer& out()
			{
				return (OutputBuffer&) *this;
			}

			streams::FormattedOutput<OutputBuffer> fout()
			{
				return streams::FormattedOutput<streams::OutputBuffer>(*this);
			}

			// Workaround for gcc bug https://gcc.gnu.org/bugzilla/show_bug.cgi?id=66957
			// Fixed in 4.9.4 (currently using 4.9.2 only)
			// We have to make the constructor public to allow virtual inheritance...
			//	protected:
			template<uint8_t SIZE_TX> AbstractUATX(char (&output)[SIZE_TX]) : OutputBuffer{output}
			{
			}

		protected:
			void begin_serial(uint32_t rate, Parity parity, StopBits stop_bits);
			static Parity calculate_parity(Parity parity, uint8_t value);

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

			template<uint8_t SIZE_TX>
			UATX(char (&output)[SIZE_TX]) : AbstractUATX{output}, tx_{gpio::PinMode::OUTPUT, true}
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
				char value;
				while (out().pull(value)) write(value);
			}
			virtual void on_overflow(UNUSED char c) override
			{
				errors_.all_errors.queue_overflow = true;
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

		class AbstractUARX : virtual public UARTErrors, private streams::InputBuffer
		{
		public:
			streams::InputBuffer& in()
			{
				return (streams::InputBuffer&) *this;
			}

			streams::FormattedInput<streams::InputBuffer> fin()
			{
				return streams::FormattedInput<streams::InputBuffer>(*this);
			}

		protected:
			template<uint8_t SIZE_RX> AbstractUARX(char (&input)[SIZE_RX]) : InputBuffer{input}
			{
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

			using PIN_TRAIT = board_traits::DigitalPin_trait<RX>;
			using PCI_TYPE = typename interrupt::PCIType<RX>::TYPE;
			using PORT_TRAIT = typename PCI_TYPE::TRAIT;
			using INT_TYPE = interrupt::INTSignal<RX>;

			template<uint8_t SIZE_RX> UARX(char (&input)[SIZE_RX]) : AbstractUARX(input), rx_{gpio::PinMode::INPUT}
			{
				static_assert(
					(PORT_TRAIT::PCI_MASK & _BV(board_traits::DigitalPin_trait<RX>::BIT)) || (PIN_TRAIT::IS_INT),
					"RX must be a PinChangeInterrupt or an ExternalInterrupt pin");
			}

			void register_rx_handler()
			{
				interrupt::register_handler(*this);
			}

			void begin(PCI_TYPE& enabler, uint32_t rate, Parity parity = Parity::NONE,
					   StopBits stop_bits = StopBits::ONE)
			{
				pci_ = &enabler;
				begin_serial(rate, parity, stop_bits);
				pci_->template enable_pin<RX>();
			}

			void begin(INT_TYPE& enabler, uint32_t rate, Parity parity = Parity::NONE,
					   StopBits stop_bits = StopBits::ONE)
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

			//	protected:
			void on_pin_change();

		private:
			typename gpio::FastPinType<RX>::TYPE rx_;
			union
			{
				PCI_TYPE* pci_;
				INT_TYPE* int_;
			};
		};

		template<board::DigitalPin RX> void UARX<RX>::on_pin_change()
		{
			// Check RX is low (start bit)
			if (rx_.value()) return;
			uint8_t value = 0;
			bool odd = false;
			UARTErrors_ errors;
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
				errors.all_errors.parity_error = (rx_.value() != parity_bit);
			}

			// Push value if no error
			if (!errors.has_errors)
			{
				errors.all_errors.queue_overflow = !in().push_(value);
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
	}
}

#endif /* SOFTUART_HH */
