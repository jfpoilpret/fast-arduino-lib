//   Copyright 2016-2019 Jean-Francois Poilpret
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

/// @cond api

/**
 * @file 
 * Software-emulated serial API.
 */
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

/**
 * Register the necessary ISR (Interrupt Service Routine) for an serial::soft::UARX
 * to work correctly. This applies to an `UARX` (or `UART`) which @p RX pin is
 * a PCINT pin.
 * @param RX the `board::InterruptPin` used as RX for the UARX (or UART)
 * @param PCI_NUM the number of the `PCINT` vector for the given @p RX pin
 */
#define REGISTER_UART_PCI_ISR(RX, PCI_NUM)                        \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                              \
	{                                                             \
		serial::soft::isr_handler::check_uart_pci<PCI_NUM, RX>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for an serial::soft::UARX
 * to work correctly. This applies to an `UARX` (or `UART`) which @p RX pin is
 * a External INT pin.
 * @param RX the `board::ExternalInterruptPin` used as RX for the UARX (or UART)
 * @param INT_NUM the number of the `INT` vector for the given @p RX pin
 */
#define REGISTER_UART_INT_ISR(RX, INT_NUM)                        \
	ISR(CAT3(INT, INT_NUM, _vect))                                \
	{                                                             \
		serial::soft::isr_handler::check_uart_int<INT_NUM, RX>(); \
	}

//FIXME Handle begin/end properly in relation to current queue content
/**
 * Defines API types used by software UART features.
 * This API is available to all MCU, even those that do not have hardware UART,
 * hence even ATtiny MCU are supported.
 * 
 * @sa serial::hard
 */
namespace serial::soft
{
	/// @cond notdocumented
	class AbstractUATX : virtual public UARTErrors, private streams::ostreambuf
	{
	protected:
		template<uint8_t SIZE_TX> AbstractUATX(char (&output)[SIZE_TX]) : ostreambuf{output} {}

		streams::ostream out()
		{
			return streams::ostream(*this);
		}

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
	/// @endcond

	/**
	 * Software-emulated serial transmitter API.
	 * 
	 * @tparam TX_ the `board::DigitalPin` to which transmitted signal is sent
	 * 
	 * @sa UARX
	 * @sa UART
	 */
	template<board::DigitalPin TX_> class UATX : public AbstractUATX
	{
	public:
		/** The `board::DigitalPin` to which transmitted signal is sent */
		static constexpr const board::DigitalPin TX = TX_;

		/**
		 * Construct a new software serial transmitter and provide it with a
		 * buffer for payload transmission.
		 * @param output an array of characters used by this transmitter to
		 * buffer output during transmission
		 */
		template<uint8_t SIZE_TX> UATX(char (&output)[SIZE_TX]) : AbstractUATX{output}, tx_{gpio::PinMode::OUTPUT, true}
		{}

		/**
		 * Enable the transmitter. 
		 * This is needed before any transmission can take place.
		 * Once called, it is possible to push content to `out()`,
		 * which will be then transmitted through the serial connection.
		 * 
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 */
		void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			begin_serial(rate, parity, stop_bits);
			//FIXME if queue is not empty, we should process it until everything is written (or clear it?)
		}

		/**
		 * Get the formatted output stream used to send content through this serial
		 * transmitter.
		 */
		streams::ostream out()
		{
			return AbstractUATX::out();
		}

		/**
		 * Stop all transmissions.
		 * Once called, it is possible to re-enable transmission again by
		 * calling `begin()`.
		 */
		void end()
		{
			//FIXME if queue not empty we should:
			// - prevent pushing to it (how?)
			// - flush it completely
			// - enable pushing again?
		}

	protected:
		/// @cond notdocumented
		virtual void on_put() override
		{
			//FIXME we should write ONLY if UAT is active (begin() has been called and not end())
			check_overflow();
			char value;
			while (out_().queue().pull(value)) write(value);
		}
		/// @endcond

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

	/// @cond notdocumented
	class AbstractUARX : virtual public UARTErrors, private streams::istreambuf
	{
	protected:
		template<uint8_t SIZE_RX> AbstractUARX(char (&input)[SIZE_RX]) : istreambuf{input} {}

		streams::istreambuf& in_()
		{
			return (streams::istreambuf&) *this;
		}

		streams::istream in()
		{
			return streams::istream(*this);
		}

		void begin_serial(uint32_t rate, Parity parity, StopBits stop_bits);

		template<board::DigitalPin RX> void pin_change();

		// Check if we can further refactor here, as we don't want parity stored twice for RX and TX...
		Parity parity_;
		// Various timing constants based on rate
		uint16_t interbit_rx_time_;
		uint16_t start_bit_rx_time_;
		uint16_t parity_bit_rx_time_;
		uint16_t stop_bit_rx_time_push_;
		uint16_t stop_bit_rx_time_no_push_;
	};
	/// @endcond

	template<board::DigitalPin RX_> void AbstractUARX::pin_change()
	{
		using RX = gpio::FastPinType<RX_>;
		// Check RX is low (start bit)
		if (RX::value()) return;
		uint8_t value = 0;
		bool odd = false;
		Errors errors;
		errors.has_errors = 0;
		// Wait for start bit to finish
		_delay_loop_2(start_bit_rx_time_);
		// Read first 7 bits
		for (uint8_t i = 0; i < 7; ++i)
		{
			if (RX::value())
			{
				value |= 0x80;
				odd = !odd;
			}
			value >>= 1;
			_delay_loop_2(interbit_rx_time_);
		}
		// Read last bit
		if (RX::value())
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
			errors.parity_error = (RX::value() != parity_bit);
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
	}

	/// @cond notdocumented
	template<typename T, T IRQ> class UARX {};
	/// @endcond

	/**
	 * Software-emulated serial receiver API.
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_UART_INT_ISR()`.
	 * 
	 * @tparam RX_ the `board::ExternalInterruptPin` which shall receive serial signal
	 * 
	 * @sa REGISTER_UART_INT_ISR()
	 * @sa UATX
	 * @sa UART
	 */
	template<board::ExternalInterruptPin RX_>
	class UARX<board::ExternalInterruptPin, RX_> : public AbstractUARX
	{
	public:
		/**
		 * The `board::DigitalPin` which shall receive serial signal; this
		 * must be either an External INT pin (`board::ExternalInterruptPin`) or a 
		 * PinChangeInterrupt pin (`board::InterruptPin`).
		 */
		static constexpr const board::DigitalPin RX = board::EXT_PIN<RX_>;

	public:
		/**
		 * The interrupt::INTSignal type for `RX_` pin, if it is a
		 * External Interrupt pin. This type is used in `begin()` call.
		 * @sa begin()
		 */
		using INT_TYPE = typename interrupt::INTSignal<RX_>;

		/**
		 * Construct a new software serial receiver and provide it with a
		 * buffer for interrupt-based reception.
		 * Reception is asynchronous.
		 * @param input an array of characters used by this receiver to
		 * store content received through serial line, buffered until read through
		 * `in()`.
		 * @sa REGISTER_UART_INT_ISR()
		 */
		template<uint8_t SIZE_RX> UARX(char (&input)[SIZE_RX]) : AbstractUARX{input}, rx_{gpio::PinMode::INPUT}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Get the formatted input stream used to read content received through
		 * this serial transmitter.
		 */
		streams::istream in()
		{
			return AbstractUARX::in();
		}

		/**
		 * Enable the receiver. 
		 * This is needed before any reception can take place.
		 * Once called, it is possible to read content, received through serial
		 * connection, by using `in()`.
		 * 
		 * @param enabler the `interrupt::INTSignal` for the RX pin; it is used to
		 * enable interrupts on that pin.
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 */
		void begin(INT_TYPE& enabler, uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			int_ = &enabler;
			AbstractUARX::begin_serial(rate, parity, stop_bits);
			int_->enable();
		}
		
		/**
		 * Stop reception.
		 * Once called, it is possible to re-enable reception again by
		 * calling `begin()`.
		 */
		void end()
		{
			int_->disable();
		}

	private:
		void on_pin_change()
		{
			this->pin_change<RX>();
			// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
			int_->clear_();
		}

		typename gpio::FastPinType<RX>::TYPE rx_;
		INT_TYPE* int_;
		friend struct isr_handler;
	};

	/**
	 * Software-emulated serial receiver API.
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_UART_PCI_ISR()`.
	 * 
	 * @tparam RX_ the `board::InterruptPin` which shall receive serial signal
	 * 
	 * @sa REGISTER_UART_PCI_ISR()
	 * @sa UATX
	 * @sa UART
	 */
	template<board::InterruptPin RX_> class UARX<board::InterruptPin, RX_> : public AbstractUARX
	{
	public:
		/**
		 * The `board::DigitalPin` which shall receive serial signal; this
		 * must be either an External INT pin (`board::ExternalInterruptPin`) or a 
		 * PinChangeInterrupt pin (`board::InterruptPin`).
		 */
		static constexpr const board::DigitalPin RX = board::PCI_PIN<RX_>;

	public:
		/**
		 * The interrupt::PCISignal type for `RX_` pin, if it is a
		 * PinChangeInterrupt pin. This type is used in `begin()` call.
		 * @sa begin()
		 */
		using PCI_TYPE = typename interrupt::PCIType<RX_>::TYPE;

		/**
		 * Construct a new software serial receiver and provide it with a
		 * buffer for interrupt-based reception.
		 * Reception is asynchronous.
		 * @param input an array of characters used by this receiver to
		 * store content received through serial line, buffered until read through
		 * `in()`.
		 * @sa REGISTER_UART_PCI_ISR()
		 */
		template<uint8_t SIZE_RX> UARX(char (&input)[SIZE_RX]) : AbstractUARX{input}, rx_{gpio::PinMode::INPUT}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Get the formatted input stream used to read content received through
		 * this serial transmitter.
		 */
		streams::istream in()
		{
			return AbstractUARX::in();
		}

		/**
		 * Enable the receiver. 
		 * This is needed before any reception can take place.
		 * Once called, it is possible to read content, received through serial
		 * connection, by using `in()`.
		 * 
		 * @param enabler the `interrupt::PCISignal` for the RX pin; it is used to
		 * enable interrupts on that pin.
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 * 
		 * @sa begin(INT_TYPE&, uint32_t, Parity, StopBits)
		 */
		void begin(PCI_TYPE& enabler, uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			pci_ = &enabler;
			AbstractUARX::begin_serial(rate, parity, stop_bits);
			pci_->template enable_pin<RX_>();
		}

		/**
		 * Stop reception.
		 * Once called, it is possible to re-enable reception again by
		 * calling `begin()`.
		 */
		void end()
		{
			pci_->template disable_pin<RX_>();
		}

	private:
		void on_pin_change()
		{
			this->pin_change<RX>();
			// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
			pci_->clear_();
		}

		typename gpio::FastPinType<RX>::TYPE rx_;
		PCI_TYPE* pci_;
		friend struct isr_handler;
	};

	/// @cond notdocumented
	template<typename T, T IRQ, board::DigitalPin TX> class UART {};
	/// @endcond

	/**
	 * Software-emulated serial receiver/transceiver API.
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_UART_INT_ISR()`.
	 * 
	 * @tparam RX_ the `board::ExternalInterruptPin` which shall receive serial signal
	 * @tparam TX_ the `board::DigitalPin` to which transmitted signal is sent
	 * 
	 * @sa REGISTER_UART_INT_ISR()
	 * @sa UATX
	 * @sa UARX
	 */
	template<board::ExternalInterruptPin RX_, board::DigitalPin TX_>
	class UART<board::ExternalInterruptPin, RX_, TX_> : public UARX<board::ExternalInterruptPin, RX_>, public UATX<TX_>
	{
	private:
		using UARX_TYPE = UARX<board::ExternalInterruptPin, RX_>;
		using UATX_TYPE = UATX<TX_>;

	public:
		/** The `board::DigitalPin` to which transmitted signal is sent */
		static constexpr const board::DigitalPin TX = TX_;
		/**
		 * The `board::DigitalPin` which shall receive serial signal.
		 */
		static constexpr const board::DigitalPin RX = board::EXT_PIN<RX_>;

		/**
		 * Construct a new software serial receiver/transceiver and provide it 
		 * with 2 buffers, one for interrupt-based reception, one for transmission.
		 * 
		 * @param input an array of characters used by this receiver to
		 * store content received through serial line, buffered until read through
		 * `in()`.
		 * @param output an array of characters used by this transmitter to
		 * buffer output during transmission.
		 */
		template<uint8_t SIZE_RX, uint8_t SIZE_TX>
		UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX]) : UARX_TYPE{input}, UATX_TYPE{output} {}

		/**
		 * Enable the receiver/transceiver. 
		 * This is needed before any transmission or reception can take place.
		 * Once called, it is possible to send and receive content through serial
		 * connection, by using `in()` for reading and `out()` for writing.
		 * 
		 * @param enabler the `interrupt::INTSignal` for the RX pin; it is used to
		 * enable interrupts on that pin.
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 * 
		 * @sa begin(INT_TYPE&, uint32_t, Parity, StopBits)
		 */
		void begin(typename UARX_TYPE::INT_TYPE& int_enabler, uint32_t rate, Parity parity = Parity::NONE,
				   StopBits stop_bits = StopBits::ONE)
		{
			UARX_TYPE::begin(int_enabler, rate, parity, stop_bits);
			UATX_TYPE::begin(rate, parity, stop_bits);
		}

		/**
		 * Stop all transmissions and receptions.
		 * Once called, it is possible to re-enable transmission and reception 
		 * again by calling `begin()`.
		 */
		void end()
		{
			UARX_TYPE::end();
			UATX_TYPE::end();
		}
	};

	/**
	 * Software-emulated serial receiver/transceiver API.
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_UART_PCI_ISR()`.
	 * 
	 * @tparam RX_ the `board::InterruptPin` which shall receive serial signal
	 * @tparam TX_ the `board::DigitalPin` to which transmitted signal is sent
	 * 
	 * @sa REGISTER_UART_PCI_ISR()
	 * @sa UATX
	 * @sa UARX
	 */
	template<board::InterruptPin RX_, board::DigitalPin TX_>
	class UART<board::InterruptPin, RX_, TX_> : public UARX<board::InterruptPin, RX_>, public UATX<TX_>
	{
	private:
		using UARX_TYPE = UARX<board::InterruptPin, RX_>;
		using UATX_TYPE = UATX<TX_>;

	public:
		/** The `board::DigitalPin` to which transmitted signal is sent */
		static constexpr const board::DigitalPin TX = TX_;
		/**
		 * The `board::DigitalPin` which shall receive serial signal.
		 */
		static constexpr const board::DigitalPin RX = board::PCI_PIN<RX_>;

		/**
		 * Construct a new software serial receiver/transceiver and provide it 
		 * with 2 buffers, one for interrupt-based reception, one for transmission.
		 * 
		 * @param input an array of characters used by this receiver to
		 * store content received through serial line, buffered until read through
		 * `in()`.
		 * @param output an array of characters used by this transmitter to
		 * buffer output during transmission.
		 */
		template<uint8_t SIZE_RX, uint8_t SIZE_TX>
		UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX]) : UARX_TYPE{input}, UATX_TYPE{output} {}

		/**
		 * Enable the receiver/transceiver. 
		 * This is needed before any transmission or reception can take place.
		 * Once called, it is possible to send and receive content through serial
		 * connection, by using `in()` for reading and `out()` for writing.
		 * 
		 * @param enabler the `interrupt::PCISignal` for the RX pin; it is used to
		 * enable interrupts on that pin.
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 * 
		 * @sa begin(INT_TYPE&, uint32_t, Parity, StopBits)
		 */
		void begin(typename UARX_TYPE::PCI_TYPE& pci_enabler, uint32_t rate, Parity parity = Parity::NONE,
				   StopBits stop_bits = StopBits::ONE)
		{
			UARX_TYPE::begin(pci_enabler, rate, parity, stop_bits);
			UATX_TYPE::begin(rate, parity, stop_bits);
		}

		/**
		 * Stop all transmissions and receptions.
		 * Once called, it is possible to re-enable transmission and reception 
		 * again by calling `begin()`.
		 */
		void end()
		{
			UARX_TYPE::end();
			UATX_TYPE::end();
		}
	};

	/// @cond notdocumented
	struct isr_handler
	{
		template<uint8_t PCI_NUM_, board::InterruptPin RX_> static void check_uart_pci()
		{
			interrupt::isr_handler_pci::check_pci_pins<PCI_NUM_, RX_>();
			interrupt::HandlerHolder<serial::soft::UARX<board::InterruptPin, RX_>>::handler()->on_pin_change();
		}

		template<uint8_t INT_NUM_, board::ExternalInterruptPin RX_> static void check_uart_int()
		{
			interrupt::isr_handler_int::check_int_pin<INT_NUM_, RX_>();
			interrupt::HandlerHolder<serial::soft::UARX<board::ExternalInterruptPin, RX_>>::handler()->on_pin_change();
		}
	};
	/// @endcond
}

#endif /* SOFTUART_HH */
/// @endcond
