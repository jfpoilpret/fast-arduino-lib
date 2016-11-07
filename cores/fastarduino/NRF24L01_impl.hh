#ifndef NRF24L01_IMPL_HH
#define NRF24L01_IMPL_HH

#include <stddef.h>
#include <util/delay.h>
#include "NRF24L01_internals.hh"
#include "utilities.hh"
#include "SPI.hh"

class NRF24L01Impl: public SPI::SPIDevice, public NRF24L01Internals
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
	NRF24L01Impl(	uint16_t net, uint8_t dev,
					Board::DigitalPin csn, Board::DigitalPin ce);

	static const uint8_t DEFAULT_CHANNEL = 64;
	
	uint8_t read(uint8_t cmd);
	void read(uint8_t cmd, void* buf, size_t size);

	void write(uint8_t cmd);
	void write(uint8_t cmd, uint8_t data);
	void write(uint8_t cmd, const void* buf, size_t size);

	inline uint8_t read(Command cmd)
	{
		return read(uint8_t(cmd));
	}
	inline void read(Command cmd, void* buf, size_t size)
	{
		read(uint8_t(cmd), buf, size);
	}

	inline void write(Command cmd)
	{
		write(uint8_t(cmd));
	}
	inline void write(Command cmd, uint8_t data)
	{
		write(uint8_t(cmd), data);
	}
	inline void write(Command cmd, const void* buf, size_t size)
	{
		write(uint8_t(cmd), buf, size);
	}

	/**
	 * Read register value. Issue R_REGISTER command with given
	 * register and read value.
	 * @param[in] reg register address.
	 * @return register value.
	 */
	uint8_t read(Register reg)
	{
		return read(uint8_t(Command::R_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)));
	}

	/**
	 * Read register values. Issue R_REGISTER command with given
	 * register and write given buffer.
	 * @param[in] reg register address.
	 * @param[in] buf buffer for read data.
	 * @param[in] size number of bytes to read.
	 */
	void read(Register reg, void* buf, size_t size)
	{
		read(uint8_t(Command::R_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), buf, size);
	}

	/**
	 * Write command and value. Issue W_REGISTER command with register
	 * and write data.
	 * @param[in] reg register address.
	 * @param[in] data new setting.
	 * @return status.
	 */
	void write(Register reg, uint8_t data)
	{
		write(uint8_t(Command::W_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), data);
	}

	/**
	 * Write command and values. Issue W_REGISTER command with register
	 * and write data from given buffer.
	 * @param[in] reg register address.
	 * @param[in] buf buffer with data to write.
	 * @param[in] size number of bytes to write.
	 * @return status.
	 */
	void write(Register reg, const void* buf, size_t size)
	{
		write(uint8_t(Command::W_REGISTER) | (uint8_t(Command::REG_MASK) & uint8_t(reg)), buf, size);
	}

	/**
	 * Read status. Issue NOP command to read status.
	 * @return status.
	 */
	status_t read_status();

	/**
	 * Read FIFO status. Issue FIFO_STATUS command to read status.
	 * @return fifo status.
	 */
	fifo_status_t read_fifo_status()
	{
		return read(Register::FIFO_STATUS);
	}

	/**
	 * Read transmission status. Issue OBSERVE_TX command to read
	 * status.
	 * @return observe tx status.
	 */
	observe_tx_t read_observe_tx()
	{
		return read(Register::OBSERVE_TX);
	}

	int read_fifo_payload(uint8_t& src, uint8_t& port, void* buf, size_t count);
	
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
