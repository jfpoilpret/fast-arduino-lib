#ifndef BOARDS_ATMEGA328P_HH
#define BOARDS_ATMEGA328P_HH

#include <avr/io.h>

/* This board is based on ATmega328P */
#define BOARD_ATMEGA328P

namespace Board
{
	volatile uint8_t* const PORT_B = &PINB;
	volatile uint8_t* const PORT_C = &PINC;
	volatile uint8_t* const PORT_D = &PIND;

	static volatile uint8_t* PIN(uint8_t pin)
	{
		return pin < 8  ? PORT_D : pin < 14 ? PORT_B : PORT_C;
	}

	static constexpr uint8_t BIT(uint8_t pin)
	{
		return (pin < 8  ? pin : pin < 14 ? pin - 8 : pin - 14);
	}
	
	/**
	 * Digital pin symbols
	 */
	enum DigitalPin
	{
		D0 = 0,			// PD0
		D1,				// PD1
		D2,				// PD2
		D3,				// PD3
		D4,				// PD4
		D5,				// PD5
		D6,				// PD6
		D7,				// PD7
		D8,				// PB0
		D9,				// PB1
		D10,			// PB2
		D11,			// PB3
		D12,			// PB4
		D13,			// PB5
		D14,			// PC0
		D15,			// PC1
		D16,			// PC2
		D17,			// PC3
		D18,			// PC4
		D19,			// PC5
		LED = D13
	} __attribute__((packed));

	/**
	 * External interrupt pin symbols; sub-set of digital pins
	 * to allow compile time checking.
	 */
	enum ExternalInterruptPin
	{
		EXT0 = D2,			// PD2
		EXT1 = D3			// PD3
	} __attribute__((packed));

	/**
	 * Pin change interrupt (PCI) pins.
	 */
	enum InterruptPin
	{
		PCI0 = D0,			// PD0
		PCI1 = D1,			// PD1
		PCI2 = D2,			// PD2
		PCI3 = D3,			// PD3
		PCI4 = D4,			// PD4
		PCI5 = D5,			// PD5
		PCI6 = D6,			// PD6
		PCI7 = D7,			// PD7
		PCI8 = D8,			// PB0
		PCI9 = D9,			// PB1
		PCI10 = D10,		// PB2
		PCI11 = D11,		// PB3
		PCI12 = D12,		// PB4
		PCI13 = D13,		// PB5
		PCI14 = D14,		// PC0
		PCI15 = D15,		// PC1
		PCI16 = D16,		// PC2
		PCI17 = D17,		// PC3
		PCI18 = D18,		// PC4
		PCI19 = D19			// PC5
	} __attribute__((packed));

	/**
	 * Size of pin maps.
	 */
	enum
	{
		ANALOG_PIN_MAX = 8,
		DIGITAL_PIN_MAX = 20,
		EXT_PIN_MAX = 2,
		PCI_PIN_MAX = 20,
		PWM_PIN_MAX = 6
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
	void PCINT0_vect(void) __attribute__ ((signal));
	void PCINT1_vect(void) __attribute__ ((signal));
	void PCINT2_vect(void) __attribute__ ((signal));
	void SPI_STC_vect(void) __attribute__ ((signal));
	void TIMER0_COMPA_vect(void) __attribute__ ((signal));
	void TIMER0_COMPB_vect(void) __attribute__ ((signal));
	void TIMER0_OVF_vect(void) __attribute__ ((signal));
	void TIMER1_CAPT_vect(void)  __attribute__ ((signal));
	void TIMER1_COMPA_vect(void) __attribute__ ((signal));
	void TIMER1_COMPB_vect(void) __attribute__ ((signal));
	void TIMER1_OVF_vect(void) __attribute__ ((signal));
	void TIMER2_COMPA_vect(void) __attribute__ ((signal));
	void TIMER2_COMPB_vect(void) __attribute__ ((signal));
	void TIMER2_OVF_vect(void) __attribute__ ((signal));
	void TWI_vect(void) __attribute__ ((signal));
	void WDT_vect(void) __attribute__ ((signal));
	void USART_RX_vect(void) __attribute__ ((signal));
	void USART_TX_vect(void) __attribute__ ((signal));
	void USART_UDRE_vect(void) __attribute__ ((signal));
}
#endif /* BOARDS_ATMEGA328P_HH */
