#ifndef UARTCOMMONS_HH
#define	UARTCOMMONS_HH

#include "Board.hh"

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
	
#ifdef USART0_RX_vect
	constexpr uint8_t AVR_USART_PARITY(Parity parity)
	{
		return (parity == Parity::EVEN ? _BV(UPM00) : parity == Parity::ODD ? _BV(UPM00) | _BV(UPM01) : 0x00);
	}
	
	constexpr uint8_t AVR_USART_STOPBITS(StopBits stopbits)
	{
		return (stopbits == StopBits::ONE ? 0x00 : _BV(USBS0));
	}
#endif
	
	union _UARTErrors
	{
		uint8_t has_errors;
		struct
		{
			bool frame_error	:1;
			bool data_overrun	:1;
			bool queue_overflow	:1;
			bool parity_error	:1;
		} all_errors;
	};
	
	class UARTErrors
	{
	public:
		UARTErrors()
		{
			clear_errors();
		}
		inline void clear_errors()
		{
			_errors.has_errors = 0;
		}
		inline uint8_t has_errors() const
		{
			return _errors.has_errors;
		}
		inline bool frame_error() const
		{
			return _errors.all_errors.frame_error;
		}
		inline bool data_overrun() const
		{
			return _errors.all_errors.data_overrun;
		}
		inline bool queue_overflow() const
		{
			return _errors.all_errors.queue_overflow;
		}
		inline bool parity_error() const
		{
			return _errors.all_errors.parity_error;
		}

	protected:
		_UARTErrors _errors;
	};
};

#endif	/* UARTCOMMONS_HH */
