//   Copyright 2016-2022 Jean-Francois Poilpret
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
 * to work correctly. This applies to an `UARX` which @p RX pin is a PCINT pin.
 * @param RX the `board::InterruptPin` used as RX for the UARX
 * @param PCI_NUM the number of the `PCINT` vector for the given @p RX pin
 */
#define REGISTER_UARX_PCI_ISR(RX, PCI_NUM)                        \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                              \
	{                                                             \
		serial::soft::isr_handler::check_uarx_pci<PCI_NUM, RX>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for an serial::soft::UARX
 * to work correctly. This applies to an `UARX` which @p RX pin is an External INT pin.
 * @param RX the `board::ExternalInterruptPin` used as RX for the UARX
 * @param INT_NUM the number of the `INT` vector for the given @p RX pin
 */
#define REGISTER_UARX_INT_ISR(RX, INT_NUM)                        \
	ISR(CAT3(INT, INT_NUM, _vect))                                \
	{                                                             \
		serial::soft::isr_handler::check_uarx_int<INT_NUM, RX>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for an serial::soft::UART
 * to work correctly. This applies to an `UART` which @p RX pin is a PCINT pin.
 * @param RX the `board::InterruptPin` used as RX for the UART
 * @param TX the `board::DigitalPin` used as TX for the UART
 * @param PCI_NUM the number of the `PCINT` vector for the given @p RX pin
 */
#define REGISTER_UART_PCI_ISR(RX, TX, PCI_NUM)                        \
	ISR(CAT3(PCINT, PCI_NUM, _vect))                                  \
	{                                                                 \
		serial::soft::isr_handler::check_uart_pci<PCI_NUM, RX, TX>(); \
	}

/**
 * Register the necessary ISR (Interrupt Service Routine) for an serial::soft::UART
 * to work correctly. This applies to an `UART` which @p RX pin is an External INT pin.
 * @param RX the `board::ExternalInterruptPin` used as RX for the UART
 * @param TX the `board::DigitalPin` used as TX for the UART
 * @param INT_NUM the number of the `INT` vector for the given @p RX pin
 */
#define REGISTER_UART_INT_ISR(RX, TX, INT_NUM)                        \
	ISR(CAT3(INT, INT_NUM, _vect))                                    \
	{                                                                 \
		serial::soft::isr_handler::check_uart_int<INT_NUM, RX, TX>(); \
	}

namespace serial
{
	/**
	 * Defines API types used by software UART features.
	 * This API is available to all MCU, even those that do not have hardware UART,
	 * hence even ATtiny MCU are supported.
	 * IMPORTANT! Note that software-emulated UART cannot be as fast as hardware UART,
	 * for that reason a maximum rate of 115'200bps is supported, preferrably with 2 
	 * stop bits (depending on the sending device, UART reception may lose bits if only
	 * one stop bit is used).
	 * @sa serial::hard
	 */
	namespace soft
	{
	}
}

namespace serial::soft
{
	/// @cond notdocumented
	class AbstractUATX
	{
	public:
		/**
		 * Get the formatted output stream used to send content through this serial
		 * transmitter.
		 */
		streams::ostream out()
		{
			return streams::ostream(out_());
		}

	protected:
		AbstractUATX(const AbstractUATX&) = delete;
		AbstractUATX& operator=(const AbstractUATX&) = delete;

		template<uint8_t SIZE_TX> 
		explicit AbstractUATX(char (&output)[SIZE_TX]) : obuf_{output} {}

		// WARNING!!! This computation is super touchy for high rates because it depends totally on generated code
		// Code generation may change:
		// - when we change optimization options (we never do it and never shall)
		// - when we change avr-gcc version: we have to recalculate based on write_,TX>() generated code analysis
		// Actual timing is based on number of times to count 4 cycles, because we use _delay_loop_2(), this is
		// why all values are divided by 4
		void compute_times(uint32_t rate, StopBits stop_bits)
		{
			// Calculate timing for TX in number of cycles
			uint16_t bit_time = uint16_t(F_CPU / rate);

			// 5 or 6 cycles + delay [counted from start bit (cbi) to first bit (sbi or cbi)]
			// if 1st bit is 1: ldi + sbrs + cbi		=> 1 + 2 + 2		= 5
			// if 1st bit is 0: ldi + sbrs + rjmp + sbi	=> 1 + 1 + 2 + 2	= 6
			// => we select 5 (because it is preferrable that rounding provides higher waiting times)
			start_bit_tx_time_ = (bit_time - 5) / 4;
			
			// between 8 and 11 cycles + delay counted from first bit (sbi or cbi) to second bit (sbi or cbi)
			// part 1: if previous bit is 1: nothing			=> 0
			// part 1: if previous bit is 0: rjmp				=> 2
			// part 2: lsr + subi + brne						=> 1 + 1 + 2 = 4
			// part 3: if current bit is 1: sbrs + sbi			=> 2 + 2 = 4
			// part 3: if current bit is 0: sbrs + rjmp + cbi	=> 1 + 2 + 2 = 5
			// => we select 9 (mid-range lowest value, because it is preferrable that rounding provides higher waiting times)
			interbit_tx_time_ = (bit_time - 9) / 4;
			
			// This one is simple because waiting loop occurs immediately after sbi (stop bit set)
			stop_bit_tx_time_ = (bit_time / 4);
			if (stop_bits == StopBits::TWO) stop_bit_tx_time_ *= 2;
			// For stop bit we lengthen the bit duration of 25% to guarantee alignment of RX side on stop duration
			stop_bit_tx_time_ = (bit_time / 4) * 5 / 4;
		}
		
		static Parity calculate_parity(Parity parity, uint8_t value)
		{
			if (parity == Parity::NONE) return Parity::NONE;
			bool odd = false;
			while (value)
			{
				if (value & 0x01) odd = !odd;
				value >>= 1;
			}
			return (odd ? Parity::ODD : Parity::EVEN);
		}

		streams::ostreambuf& out_()
		{
			return obuf_;
		}

		void check_overflow(Errors& errors)
		{
			errors.queue_overflow = obuf_.overflow();
		}

		template<board::DigitalPin DPIN> void write(Parity parity, uint8_t value)
		{
			synchronized write_<DPIN>(parity, value);
		}
		template<board::DigitalPin DPIN> void write_(Parity parity, uint8_t value);

	private:
		// NOTE declaring obuf_ first instead of last optimizes code size (4 bytes)
		streams::ostreambuf obuf_;

		// Various timing constants based on rate
		uint16_t interbit_tx_time_;
		uint16_t start_bit_tx_time_;
		uint16_t stop_bit_tx_time_;
	};

	template<board::DigitalPin DPIN> void AbstractUATX::write_(Parity parity, uint8_t value)
	{
		using TX = gpio::FastPinType<DPIN>;
		// Pre-calculate all what we need: parity bit
		Parity parity_bit = calculate_parity(parity, value);

		// Write start bit
		TX::clear();
		// Wait before starting actual value transmission
		_delay_loop_2(start_bit_tx_time_);
		for (uint8_t bit = 0; bit < 8; ++bit)
		{
			if (value & 0x01)
				TX::set();
			else
				TX::clear();
			value >>= 1;
			_delay_loop_2(interbit_tx_time_);
		}
		// Add parity if needed
		if (parity_bit != Parity::NONE)
		{
			if (parity_bit == parity)
				TX::clear();
			else
				TX::set();
			_delay_loop_2(interbit_tx_time_);
		}
		// Add stop bit
		TX::set();
		_delay_loop_2(stop_bit_tx_time_);
	}
	/// @endcond

	/**
	 * Software-emulated serial transmitter API.
	 * For this API to be fully functional, you must register this class as a 
	 * `streams::ostreambuf` callback listener through `REGISTER_OSTREAMBUF_LISTENERS()`.
	 * 
	 * @tparam TX_ the `board::DigitalPin` to which transmitted signal is sent
	 * 
	 * @sa UARX
	 * @sa UART
	 * @sa REGISTER_OSTREAMBUF_LISTENERS()
	 */
	template<board::DigitalPin TX_> class UATX : public AbstractUATX, public UARTErrors
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
		template<uint8_t SIZE_TX> explicit UATX(char (&output)[SIZE_TX]) : AbstractUATX{output} 
		{
			interrupt::register_handler(*this);
		}

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
			parity_ = parity;
			compute_times(rate, stop_bits);
			out_().queue().unlock();
		}

		/**
		 * Stop all transmissions.
		 * Once called, it is possible to re-enable transmission again by
		 * calling `begin()`.
		 * @param buffer_handling unused argument, present for symetry with 
		 * `serial::hard::UATX::end()`; this is unused because useless, as 
		 * software UATX is totally synchronous.
		 */
		void end(UNUSED BufferHandling buffer_handling = BufferHandling::KEEP)
		{
			out_().queue().lock();
		}

	private:
		bool on_put(streams::ostreambuf& obuf)
		{
			if (&obuf != &out_()) return false;
			check_overflow(errors());
			char value;
			while (out_().queue().pull(value)) write<TX>(parity_, uint8_t(value));
			return true;
		}

		Parity parity_;
		gpio::FAST_PIN<TX> tx_ = gpio::FAST_PIN<TX>{gpio::PinMode::OUTPUT, true};

		DECL_OSTREAMBUF_LISTENERS_FRIEND
	};

	/// @cond notdocumented
	class AbstractUARX
	{
	public:
		/**
		 * Get the formatted input stream used to read content received through
		 * this serial transmitter.
		 */
		streams::istream in()
		{
			return streams::istream(in_());
		}

	protected:
		AbstractUARX(const AbstractUARX&) = delete;
		AbstractUARX& operator=(const AbstractUARX&) = delete;
		
		template<uint8_t SIZE_RX> explicit AbstractUARX(char (&input)[SIZE_RX]) : ibuf_{input} {}

		streams::istreambuf& in_()
		{
			return ibuf_;
		}

		void compute_times(uint32_t rate, UNUSED bool has_parity, UNUSED StopBits stop_bits)
		{
			// Calculate timing for RX in number of cycles
			uint16_t bit_time = uint16_t(F_CPU / rate);

			// Actual timing is based on number of times to count 4 cycles, because we use _delay_loop_2()

			// Time to wait (_delay_loop_2) between detection of start bit and sampling of first bit
			// For sampling of first bit we wait until middle of first bit
			// We remove processing time due to ISR call and ISR code:
			// - 3 cycles to generate the PCI interrupt
			// - 1-4 (take 2) cycles to complete current instruction
			// - 4 cycles to process the interrupt + 2 cycles rjmp in vector table
			// - 48 cycles spent in PCINT vector to save context and check stop bit (sbic)
			//		17 push + 1 in + 1 eor	=> 34 + 1 + 1 = 36
			//		2 lds + 1 ldd + 1 in + 1 mov + 1 andi + 1 sbrc => 4 + 2 + 1 + 1 + 1 + 3 = 12
			// - 2 cycles to setup stack variables
			//		1 std => 2
			// - (4N) + 4 in delay
			// - 8 cycles until first bit sample read (sbis)
			//		2 ldd + 3 ldi + 1 sbis => 4 + 3 + 1
			start_bit_rx_time_ = compute_delay((3 * bit_time) / 2, 3 + 2 + 4 + 48 + 2 + 4 + 8);

			// Time to wait (_delay_loop_2) between sampling of 2 consecutive data bits
			// This is also use between last bit and parity bit (if checked) or stop bit
			// We have to wait exactly for `bit_time` cycles
			// We remove processing time due to each bit sampling and data value update
			// - 10+4N cycles elapse between processing of each bit
			interbit_rx_time_ = compute_delay(bit_time, 10);

			// No extra delay is used after sampling of first stop bit (as there are already 
			// enough code cycles used for pushing data and restoring context on ISR leave)
			// Actually, >80 cycles elapse until next PCI can be handled
		}

		template<board::DigitalPin DPIN> void pin_change(Parity parity, Errors& errors);

	private:
		static constexpr uint16_t compute_delay(uint16_t total_cycles, uint16_t less_cycles)
		{
			// We add 3 cycles to allow rounding
			return (total_cycles > less_cycles) ? ((total_cycles - less_cycles + 3) / 4) : 1;
		}

		// NOTE declaring ibuf_ first instead of last optimizes code size (2 bytes)
		streams::istreambuf ibuf_;

		// Various timing constants based on rate
		uint16_t interbit_rx_time_;
		uint16_t start_bit_rx_time_;
	};

	template<board::DigitalPin DPIN> void AbstractUARX::pin_change(Parity parity, Errors& errors)
	{
		using RX = gpio::FastPinType<DPIN>;
		// Check RX is low (start bit)
		if (RX::value()) return;
		uint8_t value = 0;
		bool odd = false;
		errors.has_errors = 0;
		// Wait for start bit to finish
		_delay_loop_2(start_bit_rx_time_);
		// Read first 7 bits
		for (uint8_t i = 0; i < 8; ++i)
		{
			if (RX::value())
			{
				value |= 0x80;
				odd = !odd;
			}
			if (i < 7)
				value >>= 1;
			_delay_loop_2(interbit_rx_time_);
		}

		if (parity != Parity::NONE)
		{
			// Check parity bit
			bool parity_bit = (parity == Parity::ODD ? !odd : odd);
			// Check parity bit
			errors.parity_error = (RX::value() != parity_bit);
			_delay_loop_2(interbit_rx_time_);
		}

		// Check we receive a stop bit
		if (!RX::value())
			errors.frame_error = true;
		// Push value if no error
		if (errors.has_errors == 0)
			errors.queue_overflow = !in_().queue().push_(char(value));
	}
	/// @endcond

	/// @cond notdocumented
	template<typename T, T IRQ> class UARX {};
	/// @endcond

	/** @sa UARX_EXT */
	template<board::ExternalInterruptPin RX_>
	class UARX<board::ExternalInterruptPin, RX_> : public AbstractUARX, public UARTErrors
	{
	public:
		/**
		 * The `board::DigitalPin` which shall receive serial signal; this
		 * must be either an External INT pin (`board::ExternalInterruptPin`) or a 
		 * PinChangeInterrupt pin (`board::InterruptPin`).
		 */
		static constexpr const board::DigitalPin RX = board::EXT_PIN<RX_>();

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
		 * @param enabler the `interrupt::INTSignal` for the RX pin; it is used to
		 * enable interrupts on that pin.
		 * @sa REGISTER_UART_INT_ISR()
		 */
		template<uint8_t SIZE_RX> 
		explicit UARX(char (&input)[SIZE_RX], INT_TYPE& enabler) : AbstractUARX{input}, int_{enabler}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Enable the receiver. 
		 * This is needed before any reception can take place.
		 * Once called, it is possible to read content, received through serial
		 * connection, by using `in()`.
		 * 
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 */
		void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			parity_ = parity;
			AbstractUARX::compute_times(rate, parity != Parity::NONE, stop_bits);
			int_.enable();
		}
		
		/**
		 * Stop reception.
		 * Once called, it is possible to re-enable reception again by
		 * calling `begin()`.
		 * @param buffer_handling how to handle output buffer before ending
		 * transmissions
		 */
		void end(BufferHandling buffer_handling = BufferHandling::KEEP)
		{
			int_.disable();
			if (buffer_handling == BufferHandling::CLEAR)
				in_().queue().clear();
		}

	private:
		void on_pin_change()
		{
			this->pin_change<RX>(parity_, errors());
			// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
			int_.clear_();
		}

		Parity parity_;
		gpio::FAST_PIN<RX> rx_ = gpio::PinMode::INPUT;
		INT_TYPE& int_;
		friend struct isr_handler;
	};

	/// @cond notdocumented
	template<typename T, T IRQ, board::DigitalPin TX> class UART {};
	/// @endcond

	/** @sa UART_EXT */
	template<board::ExternalInterruptPin RX_, board::DigitalPin TX_>
	class UART<board::ExternalInterruptPin, RX_, TX_> : public AbstractUARX, public AbstractUATX, public UARTErrors
	{
	public:
		/** The `board::DigitalPin` to which transmitted signal is sent */
		static constexpr const board::DigitalPin TX = TX_;
		/**
		 * The `board::DigitalPin` which shall receive serial signal.
		 */
		static constexpr const board::DigitalPin RX = board::EXT_PIN<RX_>();

		/**
		 * The interrupt::INTSignal type for `RX_` pin, if it is a
		 * External Interrupt pin. This type is used in `begin()` call.
		 * @sa begin()
		 */
		using INT_TYPE = typename interrupt::INTSignal<RX_>;

		/**
		 * Construct a new software serial receiver/transceiver and provide it 
		 * with 2 buffers, one for interrupt-based reception, one for transmission.
		 * 
		 * @param input an array of characters used by this receiver to
		 * store content received through serial line, buffered until read through
		 * `in()`.
		 * @param output an array of characters used by this transmitter to
		 * buffer output during transmission.
		 * @param enabler the `interrupt::INTSignal` for the RX pin; it is used to
		 * enable interrupts on that pin.
		 */
		template<uint8_t SIZE_RX, uint8_t SIZE_TX>
		explicit UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX], INT_TYPE& enabler)
		:	AbstractUARX{input}, AbstractUATX{output}, int_{enabler}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Enable the receiver/transceiver. 
		 * This is needed before any transmission or reception can take place.
		 * Once called, it is possible to send and receive content through serial
		 * connection, by using `in()` for reading and `out()` for writing.
		 * 
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 */
		void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			out_().queue().unlock();
			parity_ = parity;
			AbstractUARX::compute_times(rate, parity != Parity::NONE, stop_bits);
			AbstractUATX::compute_times(rate, stop_bits);
			int_.enable();
		}

		/**
		 * Stop all transmissions and receptions.
		 * Once called, it is possible to re-enable transmission and reception 
		 * again by calling `begin()`.
		 * @param buffer_handling how to handle output buffer before ending
		 * transmissions
		 */
		void end(BufferHandling buffer_handling = BufferHandling::KEEP)
		{
			int_.disable();
			if (buffer_handling == BufferHandling::CLEAR)
				in_().queue().clear();
			out_().queue().lock();
		}

	private:
		bool on_put(streams::ostreambuf& obuf)
		{
			if (&obuf != &out_()) return false;
			check_overflow(errors());
			char value;
			while (out_().queue().pull(value)) write<TX>(parity_, uint8_t(value));
			return true;
		}

		void on_pin_change()
		{
			this->pin_change<RX>(parity_, errors());
			// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
			int_.clear_();
		}

		Parity parity_;
		gpio::FAST_PIN<TX> tx_ = gpio::FAST_PIN<TX>{gpio::PinMode::OUTPUT, true};
		gpio::FAST_PIN<RX> rx_ = gpio::PinMode::INPUT;
		INT_TYPE& int_;

		friend struct isr_handler;
		DECL_OSTREAMBUF_LISTENERS_FRIEND
	};

	/** @sa UARX_PCI */
	template<board::InterruptPin RX_> class UARX<board::InterruptPin, RX_> : public AbstractUARX, public UARTErrors
	{
	public:
		/**
		 * The `board::DigitalPin` which shall receive serial signal; this
		 * must be either an External INT pin (`board::ExternalInterruptPin`) or a 
		 * PinChangeInterrupt pin (`board::InterruptPin`).
		 */
		static constexpr const board::DigitalPin RX = board::PCI_PIN<RX_>();

		/**
		 * The interrupt::PCISignal type for `RX_` pin, if it is a
		 * PinChangeInterrupt pin. This type is used in `begin()` call.
		 * @sa begin()
		 */
		using PCI_TYPE = interrupt::PCI_SIGNAL<RX_>;

		/**
		 * Construct a new software serial receiver and provide it with a
		 * buffer for interrupt-based reception.
		 * Reception is asynchronous.
		 * @param input an array of characters used by this receiver to
		 * store content received through serial line, buffered until read through
		 * `in()`.
		 * @param enabler the `interrupt::PCISignal` for the RX pin; it is used to
		 * enable interrupts on that pin.
		 * @sa REGISTER_UART_PCI_ISR()
		 */
		template<uint8_t SIZE_RX>
		explicit UARX(char (&input)[SIZE_RX], PCI_TYPE& enabler)
		: AbstractUARX{input}, pci_{enabler}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Enable the receiver. 
		 * This is needed before any reception can take place.
		 * Once called, it is possible to read content, received through serial
		 * connection, by using `in()`.
		 * 
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 */
		void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			parity_ = parity;
			AbstractUARX::compute_times(rate, parity != Parity::NONE, stop_bits);
			pci_.template enable_pin<RX_>();
		}

		/**
		 * Stop reception.
		 * Once called, it is possible to re-enable reception again by
		 * calling `begin()`.
		 * @param buffer_handling how to handle output buffer before ending
		 * transmissions
		 */
		void end(BufferHandling buffer_handling = BufferHandling::KEEP)
		{
			pci_.template disable_pin<RX_>();
			if (buffer_handling == BufferHandling::CLEAR)
				in_().queue().clear();
		}

	private:
		void on_pin_change()
		{
			this->pin_change<RX>(parity_, errors());
			// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
			pci_.clear_();
		}

		Parity parity_;
		gpio::FAST_PIN<RX> rx_ = gpio::PinMode::INPUT;
		PCI_TYPE& pci_;
		friend struct isr_handler;
	};

	/** @sa UART_PCI */
	template<board::InterruptPin RX_, board::DigitalPin TX_>
	class UART<board::InterruptPin, RX_, TX_> : public AbstractUARX, public AbstractUATX, public UARTErrors
	{
	public:
		/** The `board::DigitalPin` to which transmitted signal is sent */
		static constexpr const board::DigitalPin TX = TX_;
		/**
		 * The `board::DigitalPin` which shall receive serial signal.
		 */
		static constexpr const board::DigitalPin RX = board::PCI_PIN<RX_>();

		/**
		 * The interrupt::PCISignal type for `RX_` pin, if it is a
		 * PinChangeInterrupt pin. This type is used in `begin()` call.
		 * @sa begin()
		 */
		using PCI_TYPE = interrupt::PCI_SIGNAL<RX_>;

		/**
		 * Construct a new software serial receiver/transceiver and provide it 
		 * with 2 buffers, one for interrupt-based reception, one for transmission.
		 * 
		 * @param input an array of characters used by this receiver to
		 * store content received through serial line, buffered until read through
		 * `in()`.
		 * @param output an array of characters used by this transmitter to
		 * buffer output during transmission.
		 * @param enabler the `interrupt::PCISignal` for the RX pin; it is used to
		 * enable interrupts on that pin.
		 */
		template<uint8_t SIZE_RX, uint8_t SIZE_TX>
		explicit UART(char (&input)[SIZE_RX], char (&output)[SIZE_TX], PCI_TYPE& enabler)
		:	AbstractUARX{input}, AbstractUATX{output}, pci_{enabler}
		{
			interrupt::register_handler(*this);
		}

		/**
		 * Enable the receiver/transceiver. 
		 * This is needed before any transmission or reception can take place.
		 * Once called, it is possible to send and receive content through serial
		 * connection, by using `in()` for reading and `out()` for writing.
		 * 
		 * @param rate the transmission rate in bits per second (bps)
		 * @param parity the kind of parity check used by transmission
		 * @param stop_bits the number of stop bits used by transmission
		 */
		void begin(uint32_t rate, Parity parity = Parity::NONE, StopBits stop_bits = StopBits::ONE)
		{
			out_().queue().unlock();
			parity_ = parity;
			AbstractUARX::compute_times(rate, parity != Parity::NONE, stop_bits);
			AbstractUATX::compute_times(rate, stop_bits);
			pci_.template enable_pin<RX_>();
		}

		/**
		 * Stop all transmissions and receptions.
		 * Once called, it is possible to re-enable transmission and reception 
		 * again by calling `begin()`.
		 * @param buffer_handling how to handle input buffer before ending
		 * transmissions
		 * @sa BufferHandling
		 */
		void end(BufferHandling buffer_handling = BufferHandling::KEEP)
		{
			pci_.template disable_pin<RX_>();
			if (buffer_handling == BufferHandling::CLEAR)
				in_().queue().clear();
			out_().queue().lock();
		}

	private:
		bool on_put(streams::ostreambuf& obuf)
		{
			if (&obuf != &out_()) return false;
			check_overflow(errors());
			char value;
			while (out_().queue().pull(value)) write<TX>(parity_, uint8_t(value));
			return true;
		}

		void on_pin_change()
		{
			this->pin_change<RX>(parity_, errors());
			// Clear PCI interrupt to remove pending PCI occurred during this method and to detect next start bit
			pci_.clear_();
		}

		Parity parity_;
		gpio::FAST_PIN<TX> tx_ = gpio::FAST_PIN<TX>{gpio::PinMode::OUTPUT, true};
		gpio::FAST_PIN<RX> rx_ = gpio::PinMode::INPUT;
		PCI_TYPE& pci_;

		friend struct isr_handler;
		DECL_OSTREAMBUF_LISTENERS_FRIEND
	};

	// Useful type aliases
	//=====================

	/**
	 * Software-emulated serial receiver API.
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_UART_INT_ISR()`.
	 * 
	 * @tparam RX_ the `board::ExternalInterruptPin` which shall receive serial signal
	 * 
	 * @sa REGISTER_UART_INT_ISR()
	 * @sa UARX
	 * @sa UATX
	 * @sa UART
	 */
	template<board::ExternalInterruptPin RX_> using UARX_EXT = UARX<board::ExternalInterruptPin, RX_>;

	/**
	 * Software-emulated serial receiver API.
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_UART_PCI_ISR()`.
	 * 
	 * @tparam RX_ the `board::InterruptPin` which shall receive serial signal
	 * 
	 * @sa REGISTER_UART_PCI_ISR()
	 * @sa UARX
	 * @sa UATX
	 * @sa UART
	 */
	template<board::InterruptPin RX_> using UARX_PCI = UARX<board::InterruptPin, RX_>;

	/**
	 * Software-emulated serial receiver/transceiver API.
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_UART_INT_ISR()`.
	 * You must also register this class as a `streams::ostreambuf` callback 
	 * listener through `REGISTER_OSTREAMBUF_LISTENERS()`.
	 * 
	 * @tparam RX_ the `board::ExternalInterruptPin` which shall receive serial signal
	 * @tparam TX_ the `board::DigitalPin` to which transmitted signal is sent
	 * 
	 * @sa REGISTER_UART_INT_ISR()
	 * @sa REGISTER_OSTREAMBUF_LISTENERS()
	 * @sa UART
	 * @sa UATX
	 * @sa UARX
	 */
	template<board::ExternalInterruptPin RX_, board::DigitalPin TX_>
	using UART_EXT = UART<board::ExternalInterruptPin, RX_, TX_>;

	/**
	 * Software-emulated serial receiver/transceiver API.
	 * For this API to be fully functional, you must register the right ISR in your
	 * program, through `REGISTER_UART_PCI_ISR()`.
	 * You must also register this class as a `streams::ostreambuf` callback 
	 * listener through `REGISTER_OSTREAMBUF_LISTENERS()`.
	 * 
	 * @tparam RX_ the `board::InterruptPin` which shall receive serial signal
	 * @tparam TX_ the `board::DigitalPin` to which transmitted signal is sent
	 * 
	 * @sa REGISTER_UART_PCI_ISR()
	 * @sa REGISTER_OSTREAMBUF_LISTENERS()
	 * @sa UART
	 * @sa UATX
	 * @sa UARX
	 */
	template<board::InterruptPin RX_, board::DigitalPin TX_>
	using UART_PCI = UART<board::InterruptPin, RX_, TX_>;

	/// @cond notdocumented
	struct isr_handler
	{
		template<uint8_t PCI_NUM_, board::InterruptPin RX_> static void check_uarx_pci()
		{
			interrupt::isr_handler_pci::check_pci_pins<PCI_NUM_, RX_>();
			using UARX = serial::soft::UARX_PCI<RX_>;
			interrupt::HandlerHolder<UARX>::handler()->on_pin_change();
		}

		template<uint8_t INT_NUM_, board::ExternalInterruptPin RX_> static void check_uarx_int()
		{
			interrupt::isr_handler_int::check_int_pin<INT_NUM_, RX_>();
			using UARX = serial::soft::UARX_EXT<RX_>;
			interrupt::HandlerHolder<UARX>::handler()->on_pin_change();
		}

		template<uint8_t PCI_NUM_, board::InterruptPin RX_, board::DigitalPin TX_> static void check_uart_pci()
		{
			interrupt::isr_handler_pci::check_pci_pins<PCI_NUM_, RX_>();
			using UART = serial::soft::UART_PCI<RX_, TX_>;
			interrupt::HandlerHolder<UART>::handler()->on_pin_change();
		}

		template<uint8_t INT_NUM_, board::ExternalInterruptPin RX_, board::DigitalPin TX_> static void check_uart_int()
		{
			interrupt::isr_handler_int::check_int_pin<INT_NUM_, RX_>();
			using UART = serial::soft::UART_EXT<RX_, TX_>;
			interrupt::HandlerHolder<UART>::handler()->on_pin_change();
		}
	};
	/// @endcond
}

namespace serial
{
	/// @cond notdocumented
	// Specific traits of HW UART classes
	template<board::DigitalPin TX> struct UART_trait<soft::UATX<TX>>
	{
		static constexpr bool IS_UART = true;
		static constexpr bool IS_HW_UART = false;
		static constexpr bool IS_SW_UART = true;
		static constexpr bool HAS_TX = true;
		static constexpr bool HAS_RX = false;
	};
	template<board::InterruptPin RX> struct UART_trait<soft::UARX_PCI<RX>>
	{
		static constexpr bool IS_UART = true;
		static constexpr bool IS_HW_UART = false;
		static constexpr bool IS_SW_UART = true;
		static constexpr bool HAS_TX = false;
		static constexpr bool HAS_RX = true;
	};
	template<board::ExternalInterruptPin RX> struct UART_trait<soft::UARX_EXT<RX>>
	{
		static constexpr bool IS_UART = true;
		static constexpr bool IS_HW_UART = false;
		static constexpr bool IS_SW_UART = true;
		static constexpr bool HAS_TX = false;
		static constexpr bool HAS_RX = true;
	};
	template<board::DigitalPin TX, board::InterruptPin RX> struct UART_trait<soft::UART_PCI<RX, TX>>
	{
		static constexpr bool IS_UART = true;
		static constexpr bool IS_HW_UART = false;
		static constexpr bool IS_SW_UART = true;
		static constexpr bool HAS_TX = true;
		static constexpr bool HAS_RX = true;
	};
	template<board::DigitalPin TX, board::ExternalInterruptPin RX> struct UART_trait<soft::UART_EXT<RX, TX>>
	{
		static constexpr bool IS_UART = true;
		static constexpr bool IS_HW_UART = false;
		static constexpr bool IS_SW_UART = true;
		static constexpr bool HAS_TX = true;
		static constexpr bool HAS_RX = true;
	};
	/// @endcond
}

#endif /* SOFTUART_HH */
/// @endcond
