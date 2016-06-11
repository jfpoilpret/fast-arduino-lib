#ifndef BOARDS_MEGA_HH
#define BOARDS_MEGA_HH

/* This board is based on ATmega1280/2560 */
#if defined(ARDUINO_MEGA2560)
# define BOARD_ATMEGA2560
#else
# define BOARD_ATMEG1280
#endif

/**
 * Cosa MEGA Board pin symbol definitions for the ATmega1280 and
 * ATmega2560 based Arduino boards; Mega 1280/2560. Cosa does not use
 * pin numbers as Arduino/Wiring, instead strong data type is used
 * (enum types) for the specific pin classes; DigitalPin, AnalogPin,
 * etc.
 *
 * The pin numbers for ATmega1280 and ATmega2560 are only symbolically
 * mapped, i.e. a pin number/digit will not work, symbols must be
 * used, e.g., Board::D42. Avoid iterations assuming that the symbols
 * are in order.
 *
 * The static inline functions, SFR, BIT and UART, rely on compiler
 * optimizations to be reduced.
 */
namespace Board
{
	static constexpr uint8_t PIN(uint8_t pin)
	{
		return _SFR_IO_ADDR(pin < 8  ? PINE :
							pin < 16 ? PINH :
							pin < 24 ? PINB :
							pin < 32 ? PINA :
							pin < 40 ? PINC :
							pin < 48 ? PIND :
							pin < 56 ? PINL :
							pin < 64 ? PINF :
							pin < 72 ? PINK :
							pin < 80 ? PINJ :
							PING);
	}

	static constexpr uint8_t BIT(uint8_t pin)
	{
		return (pin & 0x7);
		return (pin < 8  ? pin : pin < 14 ? pin - 8 : pin - 14);
	}

	/**
	 * Digital pin symbols
	 */
	enum DigitalPin
	{
		D0 = 0,			// PE0
		D1 = 1,			// PE1
		D2 = 4,			// PE4
		D3 = 5,			// PE5
		D4 = 85,		// PG5
		D5 = 3,			// PE3
		D6 = 11,		// PH3
		D7 = 12,		// PH4
		D8 = 13,		// PH5
		D9 = 14,		// PH6
		D10 = 20,		// PB4
		D11 = 21,		// PB5
		D12 = 22,		// PB6
		D13 = 23,		// PB7
		D14 = 73,		// PJ1
		D15 = 72,		// PJ0
		D16 = 9,		// PH1
		D17 = 8,		// PH0
		D18 = 43,		// PD3
		D19 = 42,		// PD2
		D20 = 41,		// PD1
		D21 = 40,		// PD0
		D22 = 24,		// PA0
		D23 = 25,		// PA1
		D24 = 26,		// PA2
		D25 = 27,		// PA3
		D26 = 28,		// PA4
		D27 = 29,		// PA5
		D28 = 30,		// PA6
		D29 = 31,		// PA7
		D30 = 39,		// PC7
		D31 = 38,		// PC6
		D32 = 37,		// PC5
		D33 = 36,		// PC4
		D34 = 35,		// PC3
		D35 = 34,		// PC2
		D36 = 33,		// PC1
		D37 = 32,		// PC0
		D38 = 47,		// PD7
		D39 = 82,		// PG2
		D40 = 81,		// PG1
		D41 = 80,		// PG0
		D42 = 55,		// PL7
		D43 = 54,		// PL6
		D44 = 53,		// PL5
		D45 = 52,		// PL4
		D46 = 51,		// PL3
		D47 = 50,		// PL2
		D48 = 49,		// PL1
		D49 = 48,		// PL0
		D50 = 19,		// PB3/MISO
		D51 = 18,		// PB2/MOSI
		D52 = 17,		// PB1/SCK
		D53 = 16,		// PB0/SS
		D54 = 56,		// PF0/A0
		D55 = 57,		// PF1/A1
		D56 = 58,		// PF2/A2
		D57 = 59,		// PF3/A3
		D58 = 60,		// PF4/A4
		D59 = 61,		// PF5/A5
		D60 = 62,		// PF6/A6
		D61 = 63,		// PF7/A7
		D62 = 64,		// PK0/A8
		D63 = 65,		// PK1/A9
		D64 = 66,		// PK2/A10
		D65 = 67,		// PK3/A11
		D66 = 68,		// PK4/A12
		D67 = 69,		// PK5/A13
		D68 = 70,		// PK6/A14
		D69 = 71,		// PK7/A15
		LED = D13
	} __attribute__((packed));

	/**
	 * External interrupt pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	enum ExternalInterruptPin
	{
		EXT0 = D21,			// PD0
		EXT1 = D20,			// PD1
		EXT2 = D19,			// PD2
		EXT3 = D18,			// PD3
		EXT4 = D2,			// PE4
		EXT5 = D3			// PE5
	} __attribute__((packed));

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	enum InterruptPin
	{
		PCI0 = D10,			// PB4
		PCI1 = D11,			// PB5
		PCI2 = D12,			// PB6
		PCI3 = D13,			// PB7
		PCI4 = D50,			// PB3
		PCI5 = D51,			// PB2
		PCI6 = D52,			// PB1
		PCI7 = D53,			// PB0
		PCI8 = D62,			// PK0/A8
		PCI9 = D63,			// PK1/A9
		PCI10 = D64,		// PK2/A10
		PCI11 = D65,		// PK3/A11
		PCI12 = D66,		// PK4/A12
		PCI13 = D67,		// PK5/A13
		PCI14 = D68,		// PK6/A14
		PCI15 = D69			// PK7/A15
	} __attribute__((packed));

	/**
	 * Size of pin maps.
	 */
	enum
	{
		ANALOG_PIN_MAX = 16,
		DIGITAL_PIN_MAX = 70,
		EXT_PIN_MAX = 6,
		PCI_PIN_MAX = 24,
		PWM_PIN_MAX = 13
	};
};

/**
 * Forward declare interrupt service routines to allow them as friends.
 */
extern "C" {
	void ADC_vect(void) __attribute__ ((signal));
	void ANALOG_COMP_vect(void) __attribute__ ((signal));
	void INT0_vect(void) __attribute__ ((signal));
	void INT1_vect(void) __attribute__ ((signal));
	void INT2_vect(void) __attribute__ ((signal));
	void INT3_vect(void) __attribute__ ((signal));
	void INT4_vect(void) __attribute__ ((signal));
	void INT5_vect(void) __attribute__ ((signal));
	void INT6_vect(void) __attribute__ ((signal));
	void INT7_vect(void) __attribute__ ((signal));
	void PCINT0_vect(void) __attribute__ ((signal));
	void PCINT1_vect(void) __attribute__ ((signal));
	void PCINT2_vect(void) __attribute__ ((signal));
	void SPI_STC_vect(void) __attribute__ ((signal));
	void TIMER0_COMPA_vect(void) __attribute__ ((signal));
	void TIMER0_COMPB_vect(void) __attribute__ ((signal));
	void TIMER0_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_COMPA_vect(void) __attribute__ ((signal));
	void TIMER1_COMPB_vect(void) __attribute__ ((signal));
	void TIMER1_COMPC_vect(void) __attribute__ ((signal));
	void TIMER1_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_CAPT_vect(void)  __attribute__ ((signal));
	void TIMER2_COMPA_vect(void) __attribute__ ((signal));
	void TIMER2_COMPB_vect(void) __attribute__ ((signal));
	void TIMER2_OVF_vect(void) __attribute__ ((signal));
	void TWI_vect(void) __attribute__ ((signal));
	void WDT_vect(void) __attribute__ ((signal));
	void USART_UDRE_vect(void) __attribute__ ((signal));
	void USART_RX_vect(void) __attribute__ ((signal));
	void USART_TX_vect(void) __attribute__ ((signal));
	void USART1_UDRE_vect(void) __attribute__ ((signal));
	void USART1_RX_vect(void) __attribute__ ((signal));
	void USART1_TX_vect(void) __attribute__ ((signal));
	void USART2_UDRE_vect(void) __attribute__ ((signal));
	void USART2_RX_vect(void) __attribute__ ((signal));
	void USART2_TX_vect(void) __attribute__ ((signal));
	void USART3_UDRE_vect(void) __attribute__ ((signal));
	void USART3_RX_vect(void) __attribute__ ((signal));
	void USART3_TX_vect(void) __attribute__ ((signal));
}
#endif /* BOARDS_MEGA_HH */
