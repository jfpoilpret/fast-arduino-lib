#ifndef NRF24L01_IMPL_HH
#define NRF24L01_IMPL_HH

#include <stddef.h>
#include <util/delay.h>
#include "utilities.hh"
#include "SPI.hh"

class NRF24L01Impl: public SPI::SPIDevice
{
public:
	/** Broadcast device address. */
	static const uint8_t BROADCAST = 0x00;

	/**
	 * Maximum size of payload on device.
	 */
	static const size_t DEVICE_PAYLOAD_MAX = 32;

	/**
	 * Maximum size of payload. The device allows 32 bytes payload.
	 * The source address one byte and port one byte as header.
	 */
	static const size_t PAYLOAD_MAX = DEVICE_PAYLOAD_MAX - 2;
	
protected:
	// Forward-declare types
	enum class Command: uint8_t;
	enum class Register: uint8_t;
	union observe_tx_t;
	union fifo_status_t;
	union status_t;
	
	NRF24L01Impl(	uint16_t net, uint8_t dev,
					Board::DigitalPin csn, Board::DigitalPin ce);

	static const uint8_t DEFAULT_CHANNEL = 64;
	
	uint8_t read(uint8_t cmd);
	void read(uint8_t cmd, void* buf, size_t size);

	void write(uint8_t cmd);
	void write(uint8_t cmd, uint8_t data);
	void write(uint8_t cmd, const void* buf, size_t size);

	inline uint8_t read_command(Command cmd)
	{
		return read(uint8_t(cmd));
	}
	inline void read_command(Command cmd, void* buf, size_t size)
	{
		read(uint8_t(cmd), buf, size);
	}
	//TODO check if having one READ Command per register could improve size
	inline uint8_t read_register(Register reg)
	{
		return read(uint8_t(Command::R_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)));
	}
	inline void read_register(Register reg, void* buf, size_t size)
	{
		read(uint8_t(Command::R_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), buf, size);
	}

	inline void write_command(Command cmd)
	{
		write(uint8_t(cmd));
	}
	inline void write_command(Command cmd, uint8_t data)
	{
		write(uint8_t(cmd), data);
	}
	inline void write_command(Command cmd, const void* buf, size_t size)
	{
		write(uint8_t(cmd), buf, size);
	}
	inline void write_register(Register reg, uint8_t data)
	{
		write(uint8_t(Command::W_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), data);
	}
	inline void write_register(Register reg, const void* buf, size_t size)
	{
		write(uint8_t(Command::W_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), buf, size);
	}

	void transmit_mode(uint8_t dest);
	void receive_mode();

	bool available();
	status_t read_status();
	int read_fifo_payload(uint8_t& src, uint8_t& port, void* buf, size_t count);

	inline fifo_status_t read_fifo_status()
	{
		return read_register(Register::FIFO_STATUS);
	}

	inline observe_tx_t read_observe_tx()
	{
		return read_register(Register::OBSERVE_TX);
	}
	
	/**
	 * SPI Commands (See chap. 8.3.1, tab. 20, pp. 51).
	 */
	enum class Command: uint8_t
	{
		R_REGISTER = 0x00, //!< Read command and status register.
		W_REGISTER = 0x20, //!< Write command and status register.
		REG_MASK = 0x1f, //!< Mask register address (5b).
		
		R_RX_PAYLOAD = 0x61, //!< Read RX payload.
		W_TX_PAYLOAD = 0xa0, //!< Write TX payload.
		
		FLUSH_TX = 0xe1, //!< Flush TX FIFO.
		FLUSH_RX = 0xe2, //!< Flush RX FIFO.
		
		REUSE_TX_PL = 0xe3, //!< Reuse last transmitted payload.
		
		R_RX_PL_WID = 0x60, //!< Read RX payload width.
		
		W_ACK_PAYLOAD = 0xa8, //!< Write TX payload with ACK (3 bit addr).
		PIPE_MASK = 0x07, //!< Mask pipe address.
		
		W_TX_PAYLOAD_NO_ACK = 0xb0, //!< Disable AUTOACK on this specific packet.
		NOP = 0xff //!< No operation, return status.
	};

	/**
	 * NRF transceiver registers map (See chap. 9, tab. 28, pp. 57).
	 */
	enum class Register: uint8_t
	{
		CONFIG = 0x00, //!< Configuration register.
		EN_AA = 0x01, //!< Enable auto acknowledgement.
		EN_RXADDR = 0x02, //!< Enable rx addresses.
		SETUP_AW = 0x03, //!< Setup of address width.
		SETUP_RETR = 0x04, //!< Setup of auto retransmission.
		RF_CH = 0x05, //!< RF channel.
		RF_SETUP = 0x06, //!< RF setup register.
		STATUS = 0x07, //!< Status register.
		OBSERVE_TX = 0x08, //!< Transmit observe register.
		RPD = 0x09, //!< Received power detector.
		RX_ADDR_P0 = 0x0a, //!< Receive address data pipe 0.
		RX_ADDR_P1 = 0x0b, //!< - data pipe 1.
		RX_ADDR_P2 = 0x0c, //!< - data pipe 2.
		RX_ADDR_P3 = 0x0d, //!< - data pipe 3.
		RX_ADDR_P4 = 0x0e, //!< - data pipe 4.
		RX_ADDR_P5 = 0x0f, //!< - data pipe 5.
		TX_ADDR = 0x10, //!< Transmit address.
		RX_PW_P0 = 0x11, //!< Number of bytes in RX payload in data pipe 0.
		RX_PW_P1 = 0x12, //!< - data pipe 1.
		RX_PW_P2 = 0x13, //!< - data pipe 2.
		RX_PW_P3 = 0x14, //!< - data pipe 3.
		RX_PW_P4 = 0x15, //!< - data pipe 4.
		RX_PW_P5 = 0x16, //!< - data pipe 5.
		FIFO_STATUS = 0x17, //!< FIFO status register.
		DYNPD = 0x1c, //!< Enable dynamic payload length.
		FEATURE = 0x1d //!< Feature register.
	};

	/**
	 * Register OBSERVE_TX data type, performance statistics.
	 */
	union observe_tx_t
	{
		uint8_t as_byte; //!< Byte representation of performance statistics.

		struct
		{
			uint8_t arc_cnt : 4; //!< Count retransmitted packets.
			uint8_t plos_cnt : 4; //!< Count lost packets.
		};

		/**
		 * Construct transmitter performance statistics from register
		 * reading.
		 * @param[in] value register reading.
		 */
		observe_tx_t(uint8_t value) {
			as_byte = value;
		}
	};

	/**
	 * Register FIFO_STATUS data type, transmission queue status.
	 */
	union fifo_status_t
	{
		uint8_t as_byte; //!< Byte representation of fifo status.

		struct
		{
			uint8_t rx_empty : 1; //!< RX FIFO empty flag.
			uint8_t rx_full : 1; //!< RX FIFO full flag.
			uint8_t reserved1 : 2;
			uint8_t tx_empty : 1; //!< TX FIFO empty flag.
			uint8_t tx_full : 1; //!< TX FIFO full flag.
			uint8_t tx_reuse : 1; //!< Reuse last transmitted data packat.
			uint8_t reserved2 : 1;
		};

		/**
		 * Construct transmitter queue status from register reading.
		 * @param[in] value register reading.
		 */
		fifo_status_t(uint8_t value) {
			as_byte = value;
		}
	};

	/**
	 * Network address together with port.
	 */
	struct addr_t
	{
		uint8_t device; //!< Device address (LSB).
		uint16_t network; //!< Network address.

		addr_t(uint16_t net, uint8_t dev) INLINE
		{
			network = net;
			device = dev;
		}
	};

	/**
	 * NRF transceiver states (See chap. 6.1.1, fig. 4, pp. 22).
	 */
	enum class State: uint8_t
	{
		POWER_DOWN_STATE = 0,
		STANDBY_STATE,
		RX_STATE,
		TX_STATE
	};
	
	/**
	 * Register STATUS data type.
	 */
	union status_t
	{
		uint8_t as_byte; //!< Byte representation of status.
		struct
		{
			uint8_t tx_full		:1; //!< TX FIFO full.
			uint8_t rx_p_no		:3; //!< Data pipe number for available payload.
			uint8_t max_rt		:1; //!< Max number of TX retransmit interrupt.
			uint8_t tx_ds		:1; //!< Data send TX FIFO interrupt.
			uint8_t rx_dr		:1; //!< Data ready RX FIFO interrupt.
			uint8_t reserved	:1;
		};

		/**
		 * Construct status from register reading.
		 * @param[in] value register reading.
		 */
		status_t(uint8_t value)
		{
			as_byte = value;
		}
	};

	IOPin _ce;

	addr_t _addr; //!< Current network and device address.
	uint8_t _channel; //!< Current channel (device dependent.
	uint8_t _dest; //!< Latest message destination device address.

	status_t _status; //!< Latest status.
	State _state; //!< Transceiver state.

	uint16_t _trans; //!< Send count.
	uint16_t _retrans; //!< Retransmittion count.
	uint16_t _drops; //!< Dropped messages.
};

#endif /* NRF24L01_IMPL_HH */
